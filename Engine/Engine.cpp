// ------------------------------------ //
#include "Engine.h"

#include "Addons/GameModuleLoader.h"
#include "Application/AppDefine.h"
#include "Application/Application.h"
#include "Application/ConsoleInput.h"
#include "Common/DataStoring/DataStore.h"
#include "Common/StringOperations.h"
#include "Common/Types.h"
#include "Editor/Editor.h"
#include "Editor/Importer.h"
#include "Entities/GameWorld.h"
#include "Entities/GameWorldFactory.h"
#include "Events/EventHandler.h"
#include "FileSystem.h"
#include "GUI/GuiManager.h"
#include "GlobalCEFHandler.h"
#include "Handlers/IDFactory.h"
#include "Handlers/OutOfMemoryHandler.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "Iterators/StringIterator.h"
#include "Networking/NetworkHandler.h"
#include "Networking/RemoteConsole.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Rendering/Graphics.h"
#include "Script/Console.h"
#include "Sound/SoundDevice.h"
#include "Statistics/RenderingStatistics.h"
#include "Statistics/TimingMonitor.h"
#include "Threading/QueuedTask.h"
#include "Threading/ThreadingManager.h"
#include "TimeIncludes.h"
#include "Utility/Random.h"
#include "Window.h"

#include "utf8.h"

#ifdef LEVIATHAN_USES_LEAP
#include "Leap/LeapManager.h"
#endif

#ifdef LEVIATHAN_USING_SDL2
#include <SDL.h>
#endif

#include "boost/program_options.hpp"

#include <chrono>
#include <future>

using namespace Leviathan;
using namespace std;
// ------------------------------------ //
//! Used to detect when accessed from main thread
static thread_local int MainThreadMagic = 0;
constexpr auto THREAD_MAGIC = 42;

//! Used for automatic unique window names
static std::atomic<int> WindowNameCounter = {1};

DLLEXPORT Engine::Engine(LeviathanApplication* owner) : Owner(owner)
{
    // This makes sure that uninitialized engine will have at least some last frame time //
    LastTickTime = Time::GetTimeMs64();

    instance = this;
}

DLLEXPORT Engine::~Engine()
{
    // Reset the instance ptr //
    instance = nullptr;

    _ConsoleInput.reset();
}

DLLEXPORT Engine* Engine::instance = nullptr;

Engine* Engine::GetEngine()
{
    return instance;
}

DLLEXPORT Engine* Engine::Get()
{
    return instance;
}

DLLEXPORT bool Engine::IsOnMainThread() const
{
    return MainThreadMagic == THREAD_MAGIC;
}
// ------------------------------------ //
DLLEXPORT bool Engine::Init(
    AppDef* definition, NETWORKED_TYPE ntype, NetworkInterface* packethandler)
{
    GUARD_LOCK();

    MainThreadMagic = THREAD_MAGIC;

    // Get the  time, for monitoring how long loading takes //
    auto InitStartTime = Time::GetTimeMs64();

    // Store parameters //
    Define = definition;

    IsClient = ntype == NETWORKED_TYPE::Client;

    // Create all the things //

    OutOMemory = new OutOfMemoryHandler();

    IDDefaultInstance = new IDFactory();

    // Create threading facilities //
    _ThreadingManager = new ThreadingManager();
    if(!_ThreadingManager->Init()) {

        Logger::Get()->Error("Engine: Init: cannot start threading");
        return false;
    }

    // Create the randomizer //
    MainRandom = new Random((int)InitStartTime);
    MainRandom->SetAsMain();

    // Console might be the first thing we want //
    if(!NoSTDInput) {

        _ConsoleInput = std::make_unique<ConsoleInput>();

        if(!_ConsoleInput->Init(
               std::bind(&Engine::_ReceiveConsoleInput, this, std::placeholders::_1),
               NoGui ? true : false)) {
            Logger::Get()->Error("Engine: Init: failed to read stdin, perhaps pass --nocin");
            return false;
        }
    }

    if(NoGui) {

        // Tell window title //
        Logger::Get()->Write(
            "// ----------- " + Define->GetWindowDetails().Title + " ----------- //");
    }


    // We could immediately receive a remote console request so this should be
    // ready when networking is started
    _RemoteConsole = new RemoteConsole();

    // We want to send a request to the master server as soon as possible //
    {
        Lock lock(NetworkHandlerLock);

        _NetworkHandler = new NetworkHandler(ntype, packethandler);

        _NetworkHandler->Init(Define->GetMasterServerInfo());
    }

    // These should be fine to be threaded //

    // File change listener //
    _ResourceRefreshHandler = new ResourceRefreshHandler();
    if(!_ResourceRefreshHandler->Init()) {

        Logger::Get()->Error("Engine: Init: cannot start resource monitor");
        return false;
    }

    // Data storage //
    Mainstore = new DataStore(true);
    if(!Mainstore) {

        Logger::Get()->Error("Engine: Init: failed to create main data store");
        return false;
    }

    // Search data folder for files //
    MainFileHandler = new FileSystem();
    if(!MainFileHandler) {

        Logger::Get()->Error("Engine: Init: failed to create FileSystem");
        return false;
    }

    if(!MainFileHandler->Init(Logger::Get())) {

        Logger::Get()->Error("Engine: Init: failed to init FileSystem");
        return false;
    }

    // File parsing //
    ObjectFileProcessor::Initialize();

    // Main program wide event dispatcher //
    MainEvents = new EventHandler();
    if(!MainEvents) {

        Logger::Get()->Error("Engine: Init: failed to create MainEvents");
        return false;
    }

    if(!MainEvents->Init()) {

        Logger::Get()->Error("Engine: Init: failed to init MainEvents");
        return false;
    }

    // Check is threading properly started //
    if(!_ThreadingManager->CheckInit()) {

        Logger::Get()->Error("Engine: Init: threading start failed");
        return false;
    }

    // create script interface before renderer //
    std::promise<bool> ScriptInterfaceResult;

    // Ref is OK to use since this task finishes before this function //
    _ThreadingManager->QueueTask(std::make_shared<QueuedTask>(std::bind<void>(
        [](std::promise<bool>& returnvalue, Engine* engine) -> void {
            try {
                engine->MainScript = new ScriptExecutor();
            } catch(const Exception&) {

                Logger::Get()->Error("Engine: Init: failed to create ScriptInterface");
                returnvalue.set_value(false);
                return;
            }

            // create console after script engine //
            engine->MainConsole = new ScriptConsole();
            if(!engine->MainConsole) {

                Logger::Get()->Error("Engine: Init: failed to create ScriptConsole");
                returnvalue.set_value(false);
                return;
            }

            if(!engine->MainConsole->Init(engine->MainScript)) {

                Logger::Get()->Error("Engine: Init: failed to initialize Console, "
                                     "continuing anyway");
            }

            engine->_GameModuleLoader = std::make_unique<GameModuleLoader>();
            engine->_GameModuleLoader->Init();

            returnvalue.set_value(true);
        },
        std::ref(ScriptInterfaceResult), this)));

    // Check if we don't want a window //
    if(NoGui) {

        Logger::Get()->Info("Engine: Init: starting in console mode "
                            "(won't allocate graphical objects) ");

        if(!_ConsoleInput->IsAttachedToConsole()) {

            Logger::Get()->Error(
                "Engine: Init: in nogui mode and no input terminal connected, "
                "quitting");
            return false;
        }

    } else {

        ObjectFileProcessor::LoadValueFromNamedVars<int>(
            Define->GetValues(), "MaxFPS", FrameLimit, 120, Logger::Get(), "Graphics: Init:");

        Graph = new Graphics();
    }

    // We need to wait for all current tasks to finish //
    _ThreadingManager->WaitForAllTasksToFinish();

    // Check return values //
    if(!ScriptInterfaceResult.get_future().get()) {

        Logger::Get()->Error("Engine: Init: one or more queued tasks failed");
        return false;
    }

    // We can queue some more tasks //
    // create leap controller //
#ifdef LEVIATHAN_USES_LEAP

    // Disable leap if in non-gui mode //
    if(NoGui)
        NoLeap = true;

    std::thread leapinitthread;
    if(!NoLeap) {

        Logger::Get()->Info("Engine: will try to create Leap motion connection");

        // Seems that std::threads are joinable when constructed with default constructor
        leapinitthread = std::thread(std::bind<void>(
            [](Engine* engine) -> void {
                engine->LeapData = new LeapManager(engine);
                if(!engine->LeapData) {
                    Logger::Get()->Error("Engine: Init: failed to create LeapManager");
                    return;
                }
                // try here just in case //
                try {
                    if(!engine->LeapData->Init()) {

                        Logger::Get()->Info(
                            "Engine: Init: No Leap controller found, not using one");
                    }
                } catch(...) {
                    // threw something //
                    Logger::Get()->Error(
                        "Engine: Init: Leap threw something, even without leap "
                        "this shouldn't happen; continuing anyway");
                }
            },
            this));
    }
#endif


    // sound device //
    std::promise<bool> SoundDeviceResult;
    // Ref is OK to use since this task finishes before this function //
    _ThreadingManager->QueueTask(std::make_shared<QueuedTask>(std::bind<void>(
        [](std::promise<bool>& returnvalue, Engine* engine) -> void {
            if(!engine->NoGui) {
                engine->Sound = new SoundDevice();

                if(!engine->Sound) {
                    Logger::Get()->Error("Engine: Init: failed to create Sound");
                    returnvalue.set_value(false);
                    return;
                }

                if(!engine->Sound->Init()) {

                    Logger::Get()->Error(
                        "Engine: Init: failed to init SoundDevice. Continuing anyway");
                }
            }

            if(!engine->NoGui) {
                // measuring //
                engine->RenderTimer = new RenderingStatistics();
                if(!engine->RenderTimer) {
                    Logger::Get()->Error("Engine: Init: failed to create RenderingStatistics");

                    returnvalue.set_value(false);
                    return;
                }
            }

            returnvalue.set_value(true);
        },
        std::ref(SoundDeviceResult), this)));

    if(!NoGui) {
        if(!Graph) {

            Logger::Get()->Error("Engine: Init: failed to create instance of Graphics");
            return false;
        }

        // call init //
        if(!Graph->Init(definition)) {
            Logger::Get()->Error("Failed to init Engine, Init graphics failed! Aborting");
            return false;
        }

        // Create window //
        GraphicalEntity1 = new Window(Graph, definition);
    }

    if(!SoundDeviceResult.get_future().get()) {

        Logger::Get()->Error("Engine: Init: sound device queued tasks failed");
        return false;
    }

#ifdef LEVIATHAN_USES_LEAP
    // We can probably assume here that leap creation has stalled if the thread is running //
    if(!NoLeap) {

        auto start = WantedClockType::now();

        while(leapinitthread.joinable()) {

            auto elapsed = WantedClockType::now() - start;

            if(elapsed > std::chrono::milliseconds(150)) {

                Logger::Get()->Warning("LeapController creation would have stalled the game!");
                Logger::Get()->Write("TODO: allow increasing wait period");
                leapinitthread.detach();
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
#endif

    const auto timeNow = Time::GetTimeMs64();

    LOG_INFO("Engine init took " + Convert::ToString(timeNow - InitStartTime) + " ms");

    PostLoad();

    const auto timeAfterPostLoad = Time::GetTimeMs64();

    if(timeAfterPostLoad - timeNow > 5)
        LOG_INFO("PostLoad took " + Convert::ToString(timeAfterPostLoad - timeNow) + " ms");


    return true;
}

void Engine::PostLoad()
{
    // increase start count //
    int startcounts = 0;

    if(Mainstore->GetValueAndConvertTo<int>("StartCount", startcounts)) {
        // increase //
        Mainstore->SetValue("StartCount", new VariableBlock(new IntBlock(startcounts + 1)));
    } else {

        Mainstore->AddVar(
            std::make_shared<NamedVariableList>("StartCount", new VariableBlock(1)));

        // set as persistent //
        Mainstore->SetPersistance("StartCount", true);
    }

    // Check if we are attached to a terminal //

    ClearTimers();

    // get time //
    LastTickTime = Time::GetTimeMs64();

    ExecuteCommandLine();

    // Run startup command line //
    _RunQueuedConsoleCommands();

    // Run queued imports
    if(!QueuedImports.empty()) {
        for(const auto& importer : QueuedImports) {
            try {
                if(!importer->Run()) {

                    LOG_ERROR("An import operation failed");
                }
            } catch(const Exception& e) {
                LOG_ERROR("An exception happened in importer:");
                e.PrintToLog();
            }
        }

        QueuedImports.clear();
        LOG_INFO("Marking as closing after processing imports");
        MarkQuit();
    }
}
// ------------------------------------ //
DLLEXPORT void Engine::PreRelease()
{
    GUARD_LOCK();

    if(PreReleaseWaiting || PreReleaseCompleted)
        return;

    PreReleaseWaiting = true;
    // This will stay true until the end of times //
    PreReleaseCompleted = true;

    // Stop command handling first //
    if(_ConsoleInput) {

        _ConsoleInput->Release(false);
        Logger::Get()->Info("Successfully stopped command handling");
    }

    // Close all editors
    OpenedEditors.clear();

    // Automatically destroy input sources //
    _NetworkHandler->ReleaseInputHandler();

    // Then kill the network //
    {
        Lock lock(NetworkHandlerLock);

        _NetworkHandler->GetInterface()->CloseDown();
    }

    // Let the game release it's resources //
    Owner->EnginePreShutdown();

    // Close remote console //
    SAFE_DELETE(_RemoteConsole);

    // Close all connections //
    {
        Lock lock(NetworkHandlerLock);

        SAFE_RELEASEDEL(_NetworkHandler);
    }

    SAFE_RELEASEDEL(_ResourceRefreshHandler);

    // Set worlds to empty //
    {
        Lock lock(GameWorldsLock);

        for(auto iter = GameWorlds.begin(); iter != GameWorlds.end(); ++iter) {
            // Set all objects to release //
            (*iter)->MarkForClear();
        }
    }

    // Set tasks to a proper state //
    _ThreadingManager->SetDiscardConditionalTasks(true);
    _ThreadingManager->SetDisallowRepeatingTasks(true);

    Logger::Get()->Info("Engine: prerelease done, waiting for a tick");
}

void Engine::Release(bool forced)
{
    GUARD_LOCK();

    if(!forced)
        LEVIATHAN_ASSERT(PreReleaseDone, "PreReleaseDone must be done before actual release!");

    // Force garbase collection //
    if(MainScript)
        MainScript->CollectGarbage();

    // Make windows clear their stored objects //
    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++) {

        AdditionalGraphicalEntities[i]->UnlinkAll();
    }

    // Finally the main window //
    if(GraphicalEntity1) {

        GraphicalEntity1->UnlinkAll();
    }

    // Destroy worlds //
    {
        Lock lock(GameWorldsLock);

        while(GameWorlds.size()) {

            GameWorlds[0]->Release();
            GameWorlds.erase(GameWorlds.begin());
        }
    }

    if(_NetworkHandler)
        _NetworkHandler->ShutdownCache();

    // Wait for tasks to finish //
    if(!forced)
        _ThreadingManager->WaitForAllTasksToFinish();

    // Destroy windows //
    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++) {

        SAFE_DELETE(AdditionalGraphicalEntities[i]);
    }

    AdditionalGraphicalEntities.clear();

    SAFE_DELETE(GraphicalEntity1);

#ifdef LEVIATHAN_USES_LEAP
    SAFE_RELEASEDEL(LeapData);
#endif

    // Console needs to be released before script release //
    SAFE_RELEASEDEL(MainConsole);

    _GameModuleLoader.reset();

    SAFE_DELETE(MainScript);

    // Save at this point (just in case it crashes before exiting) //
    Logger::Get()->Save();


    SAFE_DELETE(RenderTimer);

    SAFE_RELEASEDEL(Sound);
    SAFE_DELETE(Mainstore);

    // If graphics aren't unregistered crashing will occur //
    _ThreadingManager->UnregisterGraphics();

    // Stop threads //
    SAFE_RELEASEDEL(_ThreadingManager);

    SAFE_RELEASEDEL(Graph);

    SAFE_RELEASEDEL(MainEvents);
    // delete randomizer last, for obvious reasons //
    SAFE_DELETE(MainRandom);

    ObjectFileProcessor::Release();
    SAFE_DELETE(MainFileHandler);

    // clears all running timers that might have accidentally been left running //
    TimingMonitor::ClearTimers();

    // safe to delete this here //
    SAFE_DELETE(OutOMemory);

    SAFE_DELETE(IDDefaultInstance);

    Logger::Get()->Write("Goodbye cruel world!");
}
// ------------------------------------ //
DLLEXPORT void Engine::MessagePump()
{
    // CEF events (Also on windows as multi_threaded_message_loop makes rendering harder)
    GlobalCEFHandler::DoCEFMessageLoopWork();

    SDL_Event event;
    while(SDL_PollEvent(&event)) {

        switch(event.type) {
        case SDL_QUIT:
            LOG_INFO("SDL_QUIT received, marked as closing");
            MarkQuit();
            break;

        case SDL_KEYDOWN: {
            Window* win = GetWindowFromSDLID(event.key.windowID);

            if(win) {

                // LOG_WRITE("SDL_KEYDOWN: " + Convert::ToString(event.key.keysym.sym));

                // Core engine functionality keys
                switch(event.key.keysym.sym) {
                case SDLK_F10: {
                    // Editor key pressed
                    LOG_INFO("Editor key pressed");
                    FocusOrOpenEditor();
                    break;
                }
                default: win->InjectKeyDown(event);
                }
            }
            break;
        }
        case SDL_KEYUP: {
            Window* win = GetWindowFromSDLID(event.key.windowID);

            if(win) {

                // LOG_WRITE("SDL_KEYUP: " + Convert::ToString(event.key.keysym.sym));
                win->InjectKeyUp(event);
            }

            break;
        }
        case SDL_TEXTINPUT: {
            Window* win = GetWindowFromSDLID(event.text.windowID);

            if(win) {

                const auto text = std::string(event.text.text);

                // LOG_WRITE("TextInput: " + text);

                std::vector<uint32_t> codepoints;

                utf8::utf8to32(
                    std::begin(text), std::end(text), std::back_inserter(codepoints));

                // LOG_WRITE("Codepoints(" + Convert::ToString(codepoints.size()) + "): ");
                // for(auto codepoint : codepoints)
                //     LOG_WRITE(" " + Convert::ToString(codepoint));
                for(auto codepoint : codepoints) {

                    win->InjectCodePoint(event);
                }
            }

            break;
        }
        // TODO: implement this
        // case SDL_TEXTEDITING: (https://wiki.libsdl.org/Tutorials/TextInput)
        case SDL_MOUSEBUTTONDOWN: {
            Window* win = GetWindowFromSDLID(event.button.windowID);

            if(win)
                win->InjectMouseButtonDown(event);

            break;
        }

        case SDL_MOUSEBUTTONUP: {
            Window* win = GetWindowFromSDLID(event.button.windowID);

            if(win)
                win->InjectMouseButtonUp(event);

            break;
        }

        case SDL_MOUSEMOTION: {
            Window* win = GetWindowFromSDLID(event.motion.windowID);

            if(win)
                win->InjectMouseMove(event);

            break;
        }

        case SDL_MOUSEWHEEL: {
            Window* win = GetWindowFromSDLID(event.motion.windowID);

            if(win)
                win->InjectMouseWheel(event);

            break;
        }

        case SDL_WINDOWEVENT: {
            switch(event.window.event) {

            case SDL_WINDOWEVENT_RESIZED: {
                Window* win = GetWindowFromSDLID(event.window.windowID);

                if(win) {

                    int32_t width, height;
                    win->GetSize(width, height);

                    LOG_INFO("SDL window resize: " + Convert::ToString(width) + "x" +
                             Convert::ToString(height));

                    win->OnResize(width, height);
                }

                break;
            }
            case SDL_WINDOWEVENT_CLOSE: {
                LOG_INFO("SDL window close");

                Window* win = GetWindowFromSDLID(event.window.windowID);

                // Detect closed windows //
                if(win == GraphicalEntity1) {
                    // Window closed //
                    ReportClosedWindow(GraphicalEntity1);
                }

                for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++) {
                    if(AdditionalGraphicalEntities[i] == win) {

                        ReportClosedWindow(AdditionalGraphicalEntities[i]);
                        break;
                    }
                }

                break;
            }
            case SDL_WINDOWEVENT_FOCUS_GAINED: {
                Window* win = GetWindowFromSDLID(event.window.windowID);

                if(win)
                    win->OnFocusChange(true);

                break;
            }
            case SDL_WINDOWEVENT_FOCUS_LOST: {
                Window* win = GetWindowFromSDLID(event.window.windowID);

                if(win)
                    win->OnFocusChange(false);

                break;
            }
            }
        }
        }
    }

    // CEF needs to be let handle the keyboard events now to make sure that they can be
    // dispatched to further on listeners
    GlobalCEFHandler::DoCEFMessageLoopWork();

    // Reset input states //
    if(GraphicalEntity1) {

        // TODO: fix initial mouse position being incorrect
        GraphicalEntity1->InputEnd();
    }

    for(auto iter = AdditionalGraphicalEntities.begin();
        iter != AdditionalGraphicalEntities.end(); ++iter) {
        (*iter)->InputEnd();
    }
}

DLLEXPORT Window* Engine::GetWindowFromSDLID(uint32_t sdlid)
{
    if(GraphicalEntity1 && GraphicalEntity1->GetSDLID() == sdlid) {
        return GraphicalEntity1;
    }

    for(auto iter = AdditionalGraphicalEntities.begin();
        iter != AdditionalGraphicalEntities.end(); ++iter) {
        if((*iter)->GetSDLID() == sdlid) {

            return *iter;
        }
    }

    return nullptr;
}
// ------------------------------------ //
void Engine::Tick()
{
    // Always try to update networking //
    {
        Lock lock(NetworkHandlerLock);

        if(_NetworkHandler)
            _NetworkHandler->UpdateAllConnections();
    }

    // And handle invokes //
    ProcessInvokes();

    GUARD_LOCK();

    if(PreReleaseWaiting) {

        PreReleaseWaiting = false;
        PreReleaseDone = true;

        Logger::Get()->Info("Engine: performing final release tick");



        // Call last tick event //

        return;
    }

    // Get the passed time since the last update //
    auto CurTime = Time::GetTimeMs64();
    TimePassed = (int)(CurTime - LastTickTime);


    if((TimePassed < TICKSPEED)) {
        // It's not tick time yet //
        return;
    }


    LastTickTime += TICKSPEED;
    TickCount++;

    // Update input //
#ifdef LEVIATHAN_USES_LEAP
    if(LeapData)
        LeapData->OnTick(TimePassed);
#endif

    if(!NoGui) {
        // sound tick //
        if(Sound)
            Sound->Tick(TimePassed);

        // update windows //
        if(GraphicalEntity1)
            GraphicalEntity1->Tick(TimePassed);

        for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++) {

            AdditionalGraphicalEntities[i]->Tick(TimePassed);
        }
    }


    // Update worlds //
    {
        Lock lock(GameWorldsLock);

        // This will also update physics //
        auto end = GameWorlds.end();
        for(auto iter = GameWorlds.begin(); iter != end; ++iter) {

            (*iter)->Tick(TickCount);
        }
    }


    // Some dark magic here //
    if(TickCount % 25 == 0) {
        // update values
        Mainstore->SetTickCount(TickCount);
        Mainstore->SetTickTime(TickTime);
        Mainstore->SetTicksBehind((TimePassed - TICKSPEED) / TICKSPEED);
        // TODO: having the max tick time of the past second, would also be nice

        if(!NoGui) {
            // send updated rendering statistics //
            RenderTimer->ReportStats(Mainstore);
        }
    }

    // Update file listeners //
    if(_ResourceRefreshHandler)
        _ResourceRefreshHandler->CheckFileStatus();

    // Send the tick event //
    if(MainEvents)
        MainEvents->CallEvent(new Event(EVENT_TYPE_TICK, new IntegerEventData(TickCount)));

    // Call the default app tick //
    Owner->Tick(TimePassed);

    TickTime = (int)(Time::GetTimeMs64() - CurTime);
}

DLLEXPORT void Engine::PreFirstTick()
{
    GUARD_LOCK();

    if(_ThreadingManager)
        _ThreadingManager->NotifyQueuerThread();

    ClearTimers();

    Logger::Get()->Info("Engine: PreFirstTick: everything fine to start running");
}
// ------------------------------------ //
void Engine::RenderFrame()
{
    // We want to totally ignore this if we are in text mode //
    if(NoGui)
        return;

    int SinceLastFrame = -1;
    GUARD_LOCK();

    // limit check //
    if(!RenderTimer->CanRenderNow(FrameLimit, SinceLastFrame)) {

        // fps would go too high //
        return;
    }

    // since last frame is in microseconds 10^-6 convert to milliseconds //
    // SinceLastTickTime is always more than 1000 (always 1 ms or more) //
    SinceLastFrame /= 1000;
    FrameCount++;

    // advanced statistic start monitoring //
    RenderTimer->RenderingStart();

    MainEvents->CallEvent(
        new Event(EVENT_TYPE_FRAME_BEGIN, new IntegerEventData(SinceLastFrame)));

    // Calculate parameters for GameWorld frame rendering systems //
    int64_t timeintick = Time::GetTimeMs64() - LastTickTime;
    int moreticks = 0;

    while(timeintick > TICKSPEED) {

        timeintick -= TICKSPEED;
        moreticks++;
    }

    bool shouldrender = false;

    // Render //
    if(GraphicalEntity1 && GraphicalEntity1->Render(SinceLastFrame, TickCount + moreticks,
                               static_cast<int>(timeintick)))
        shouldrender = true;

    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++) {

        if(AdditionalGraphicalEntities[i]->Render(
               SinceLastFrame, TickCount + moreticks, static_cast<int>(timeintick)))
            shouldrender = true;
    }

    guard.unlock();
    if(shouldrender)
        Graph->Frame();

    guard.lock();
    MainEvents->CallEvent(new Event(EVENT_TYPE_FRAME_END, new IntegerEventData(FrameCount)));

    // advanced statistics frame has ended //
    RenderTimer->RenderingEnd();
}
// ------------------------------------ //
DLLEXPORT void Engine::SaveScreenShot()
{
    LEVIATHAN_ASSERT(!NoGui, "really shouldn't try to screenshot in text-only mode");
    GUARD_LOCK();

    const string fileprefix = MainFileHandler->GetDataFolder() + "Screenshots/Captured_frame_";

    GraphicalEntity1->SaveScreenShot(fileprefix);
}

DLLEXPORT int Engine::GetWindowOpenCount()
{
    int openwindows = 0;

    // If we are in text only mode always return 1 //
    if(NoGui)
        return 1;

    // TODO: should there be an IsOpen method?
    if(GraphicalEntity1)
        openwindows++;

    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++) {

        if(AdditionalGraphicalEntities[i])
            openwindows++;
    }

    return openwindows;
}
// ------------------------------------ //
DLLEXPORT bool Engine::IsValidWindow(Window* window) const
{
    if(window == GraphicalEntity1) {
        return true;
    }

    for(Window* openWindow : AdditionalGraphicalEntities) {
        if(openWindow == window) {
            return true;
        }
    }

    return false;
}

DLLEXPORT Window* Engine::OpenNewWindow()
{
    AssertIfNotMainThread();

    AppDef winparams;

    winparams.SetWindowDetails(WindowDataDetails(
        "Leviathan Window " + std::to_string(++WindowNameCounter), 1280, 720, "no",
        // Multiple vsyncs cause issues (or they can cause issues, sometimes it is fine)
        /* Define->GetWindowDetails().VSync */ false,
        // Opens on same display as the other window
        // TODO: open on next display
        Define->GetWindowDetails().DisplayNumber, Define->GetWindowDetails().FSAA, true,
#ifdef _WIN32
        Define->GetWindowDetails().Icon,
#endif
        nullptr));

    auto newwindow = std::make_unique<Window>(Graph, &winparams);

    AdditionalGraphicalEntities.push_back(newwindow.get());

    return newwindow.release();
}

DLLEXPORT bool Engine::CloseWindow(Window* window)
{
    if(!window)
        return false;

    if(IsValidWindow(window)) {

        ReportClosedWindow(window);
        return true;
    } else {
        return false;
    }
}

DLLEXPORT void Engine::ReportClosedWindow(Window* windowentity)
{
    windowentity->UnlinkAll();

    if(GraphicalEntity1 == windowentity) {

        SAFE_DELETE(GraphicalEntity1);
        return;
    }

    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++) {

        if(AdditionalGraphicalEntities[i] == windowentity) {

            SAFE_DELETE(AdditionalGraphicalEntities[i]);
            AdditionalGraphicalEntities.erase(AdditionalGraphicalEntities.begin() + i);

            return;
        }
    }

    // Didn't find the target //
    Logger::Get()->Error("Engine: couldn't find closing Window");
}
// ------------------------------------ //
DLLEXPORT void Engine::OpenEditorWindow(Window* useexistingwindow /*= nullptr*/)
{
    AssertIfNotMainThread();

    if(useexistingwindow && !IsValidWindow(useexistingwindow)) {

        LOG_WARNING("Engine: OpenEditorWindow: invalid window given, defaulting to opening a "
                    "new window");
        useexistingwindow = nullptr;
    }

    if(!useexistingwindow) {
        useexistingwindow = OpenNewWindow();
    }


    OpenedEditors.emplace_back(std::make_unique<Editor::Editor>(useexistingwindow, this));
}

DLLEXPORT void Engine::FocusOrOpenEditor()
{
    if(OpenedEditors.empty()) {

        OpenEditorWindow();
        return;
    }

    OpenedEditors.front()->BringToFront();
}
// ------------------------------------ //
DLLEXPORT void Engine::MarkQuit()
{
    if(Owner)
        Owner->MarkAsClosing();
}
// ------------------------------------ //
DLLEXPORT void Engine::Invoke(const std::function<void()>& function)
{
    RecursiveLock lock(InvokeLock);
    InvokeQueue.push_back(function);
}

DLLEXPORT void Engine::ProcessInvokes()
{
    RecursiveLock lock(InvokeLock);

    while(!InvokeQueue.empty()) {

        const auto& func = InvokeQueue.front();

        // Recursive mutex allows the invoke to call extra invokes
        func();

        InvokeQueue.pop_front();
    }
}

DLLEXPORT void Engine::RunOnMainThread(const std::function<void()>& function)
{
    if(!IsOnMainThread()) {

        Invoke(function);

    } else {

        function();
    }
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<GameWorld> Engine::CreateWorld(Window* owningwindow, int worldtype,
    const std::shared_ptr<PhysicsMaterialManager>& physicsMaterials,
    const WorldNetworkSettings& networking, int overrideid /*= -1*/)
{
    std::shared_ptr<GameWorld> world;
    if(worldtype >= 1024) {
        // Standard world types
        world = InbuiltWorldFactory::CreateNewWorld(
            static_cast<INBUILT_WORLD_TYPE>(worldtype), physicsMaterials, overrideid);
    } else {
        world =
            GameWorldFactory::Get()->CreateNewWorld(worldtype, physicsMaterials, overrideid);
    }

    if(!world) {

        LOG_ERROR("Engine: CreateWorld: factory failed to create a world of type: " +
                  std::to_string(worldtype));
        return nullptr;
    }

    world->Init(networking, NoGui ? nullptr : Graph);

    if(owningwindow)
        owningwindow->LinkObjects(world);

    Lock lock(GameWorldsLock);

    GameWorlds.push_back(world);
    return GameWorlds.back();
}

DLLEXPORT void Engine::DestroyWorld(const shared_ptr<GameWorld>& world)
{
    if(!world)
        return;

    // Release the world first //
    world->Release();

    // Then delete it //
    Lock lock(GameWorldsLock);

    auto end = GameWorlds.end();
    for(auto iter = GameWorlds.begin(); iter != end; ++iter) {

        if((*iter).get() == world.get()) {

            GameWorlds.erase(iter);
            return;
        }
    }
}
// ------------------------------------ //
DLLEXPORT void Engine::ClearTimers()
{
    Lock lock(GameWorldsLock);

    for(auto iter = GameWorlds.begin(); iter != GameWorlds.end(); ++iter) {
    }
}
// ------------------------------------ //
void Engine::_NotifyThreadsRegisterOgre()
{
    if(NoGui)
        return;

    // Register threads to use graphical objects //
    _ThreadingManager->MakeThreadsWorkWithOgre();
}
// ------------------------------------ //
DLLEXPORT int64_t Leviathan::Engine::GetTimeSinceLastTick() const
{

    return Time::GetTimeMs64() - LastTickTime;
}

DLLEXPORT int Engine::GetCurrentTick() const
{

    return TickCount;
}
// ------------------------------------ //
void Engine::_AdjustTickClock(int amount, bool absolute /*= true*/)
{

    GUARD_LOCK();

    if(!absolute) {

        Logger::Get()->Info("Engine: adjusted tick timer by " + Convert::ToString(amount));

        LastTickTime += amount;
        return;
    }

    // Calculate the time in the current last tick //
    int64_t templasttick = LastTickTime;

    int64_t curtime = Time::GetTimeMs64();

    while(curtime - templasttick >= TICKSPEED) {

        templasttick += TICKSPEED;
    }

    // Check how far off we are from the target //
    int64_t intolasttick = curtime - templasttick;

    int changeamount = amount - static_cast<int>(intolasttick);

    Logger::Get()->Info("Engine: changing tick counter by " + Convert::ToString(changeamount));

    LastTickTime += changeamount;
}

void Engine::_AdjustTickNumber(int tickamount, bool absolute)
{

    GUARD_LOCK();

    if(!absolute) {

        TickCount += tickamount;

        Logger::Get()->Info("Engine: adjusted tick by " + Convert::ToString(tickamount) +
                            ", tick is now " + Convert::ToString(TickCount));

        return;
    }

    TickCount = tickamount;

    Logger::Get()->Info("Engine: tick set to " + Convert::ToString(TickCount));
}
// ------------------------------------ //
int TestCrash(int writenum)
{
    volatile int* target = nullptr;
    (*target) = writenum;

    Logger::Get()->Write("It didn't crash...");
    return 42;
}

DLLEXPORT bool Engine::PassCommandLine(int argcount, char* args[])
{
    namespace po = boost::program_options;

    std::vector<std::vector<std::string>> import;
    std::vector<std::string> cmds;
    // TODO: these could probably directly be read into the variables in this class
    bool nogui = false;
    bool noleap = false;
    bool nocin = false;

    bool crash = false;

    po::options_description desc("Engine Options");
    // clang-format off
    desc.add_options()
        ("import", po::value<std::vector<std::string>>()->multitoken(),
            "Import assets from source to destination")
        ("cmd", po::value<std::vector<std::string>>(&cmds)->multitoken(),
            "Run console commands after startup")
        ("nogui", po::bool_switch(&nogui), "Disable graphics")
        ("nocin", po::bool_switch(&nocin), "Disable stdin reading")
        ("noleap", po::bool_switch(&noleap), "Disable Leap Motion")
        ("noleap", po::bool_switch(&crash), "Crash for testing purposes")
        // We see CEF arguments here, we need to allow them
        ("no-sandbox", "CEF option")
        ("single-process", "CEF option")
        ;
    // clang-format on

    // TODO: allow the game to add extra options here

    po::positional_options_description positional;
    positional.add("cmd", -1);

    // This was before added to PassedCommands.push_back(std::move(splitval));
    // but it would be better to make those actual flags

    po::variables_map vm;

    try {
        // The parsing is done in two steps here to add custom handling for the import option
        po::parsed_options parsed_options =
            po::command_line_parser(argcount, args).options(desc).positional(positional).run();


        // Read import option lists
        for(const po::option& option : parsed_options.options) {
            if(option.string_key == "import")
                import.push_back(option.value);
        }

        // Finish parsing
        po::store(parsed_options, vm);
        po::notify(vm);

    } catch(const po::error& e) {
        LOG_INFO("Engine: Command line: ");
        for(int i = 0; i < argcount; ++i) {

            LOG_WRITE("\t> " + (args[i] ? std::string(args[i]) : std::string()));
        }

        LOG_ERROR("Engine: parsing command line failed: " + std::string(e.what()));

        std::stringstream sstream;
        desc.print(sstream);

        LOG_WRITE(sstream.str());

        return false;
    }

    if(nogui) {
        NoGui = true;
        LOG_INFO("Engine starting in non-GUI mode");
    }

    if(nocin) {
        NoSTDInput = true;
        LOG_INFO("Engine not listening for terminal commands");
    }

    if(noleap) {
        NoLeap = true;

#ifdef LEVIATHAN_USES_LEAP
        LOG_INFO("Engine starting with LeapMotion disabled");
#endif
    }

    if(crash) {
        LOG_INFO("Engine testing crash handling");
        // TODO: write a file that disables crash handling
        // Make the log say something useful //
        Logger::Get()->Save();

        // Test crashing //
        TestCrash(12);
    }

    // // Coalesce commands
    // if(vm.count("console-commands")) {
    // }

    for(const std::string& command : cmds) {
        if(StringOperations::IsCharacterQuote(command.at(0))) {

            StringIterator itr(command);

            auto withoutquotes = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);

            if(withoutquotes) {

                QueuedConsoleCommands.push_back(std::move(withoutquotes));

            } else {

                LOG_WARNING("Engine: command in quotes is empty");
            }

        } else {

            QueuedConsoleCommands.push_back(std::make_unique<std::string>(command));
        }
    }

    for(const auto& group : import) {

        if(group.size() != 2) {

            LOG_ERROR("Import option needs two parameters, source and destination, parameter "
                      "count: " +
                      std::to_string(group.size()));
            return false;
        }

        QueuedImports.push_back(std::make_unique<Editor::Importer>(group[0], group[1]));
    }

    return true;
}

DLLEXPORT void Engine::ExecuteCommandLine()
{
    StringIterator itr;

    // TODO: this does nothing
    // Iterate over the commands and process them //
    for(size_t i = 0; i < PassedCommands.size(); i++) {

        itr.ReInit(PassedCommands[i].get());
        // Skip the preceding '-'s //
        itr.SkipCharacters('-');

        // Get the command //
        auto firstpart = itr.GetUntilNextCharacterOrAll<string>(':');

        // Execute the wanted command //
        if(StringOperations::CompareInsensitive<string>(*firstpart, "RemoteConsole")) {

            // Get the next command //
            auto commandpart = itr.GetUntilNextCharacterOrAll<string>(L':');

            if(*commandpart == "CloseIfNone") {
                // Set the command //
                _RemoteConsole->SetCloseIfNoRemoteConsole(true);
                Logger::Get()->Info("Engine will close when no active/waiting remote console "
                                    "sessions");

            } else if(*commandpart == "OpenTo") {
                // Get the to part //
                auto topart = itr.GetStringInQuotes<string>(QUOTETYPE_BOTH);

                int token = 0;

                auto numberpart = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_NONE);

                if(numberpart->size() == 0) {

                    Logger::Get()->Warning("Engine: ExecuteCommandLine: RemoteConsole: "
                                           "no token number provided");
                    continue;
                }
                // Convert to a real number. Maybe we could see if the token is
                // complex enough here, but that isn't necessary
                token = Convert::StringTo<int>(*numberpart);

                if(token == 0) {
                    // Invalid number? //
                    Logger::Get()->Warning("Engine: ExecuteCommandLine: RemoteConsole: "
                                           "couldn't parse token number, " +
                                           *numberpart);
                    continue;
                }

                // Create a connection (or potentially use an existing one) //
                shared_ptr<Connection> tmpconnection =
                    _NetworkHandler->OpenConnectionTo(*topart);

                // Tell remote console to open a command to it //
                if(tmpconnection) {

                    _RemoteConsole->OfferConnectionTo(tmpconnection, "AutoOpen", token);

                } else {
                    // Something funky happened... //
                    Logger::Get()->Warning("Engine: ExecuteCommandLine: RemoteConsole: "
                                           "couldn't open connection to " +
                                           *topart + ", couldn't resolve address");
                }

            } else {
                // Unknown command //
                Logger::Get()->Warning("Engine: ExecuteCommandLine: unknown RemoteConsole "
                                       "command: " +
                                       *commandpart +
                                       ", whole argument: " + *PassedCommands[i]);
            }
        }
    }


    PassedCommands.clear();

    // Now we can set some things that require command line arguments //
    // _RemoteConsole might be NULL //
    if(_RemoteConsole)
        _RemoteConsole->SetAllowClose();
}
// ------------------------------------ //
void Engine::_RunQueuedConsoleCommands()
{
    if(QueuedConsoleCommands.empty())
        return;

    if(!MainConsole) {

        LOG_FATAL("Engine: MainConsole has not been created before running command line "
                  "passed commands");
        return;
    }

    LOG_INFO("Engine: Running PostStartup command line. Commands: " +
             Convert::ToString(QueuedConsoleCommands.size()));

    for(auto& command : QueuedConsoleCommands) {

        LOG_INFO("Engine: Running \"" + *command + "\"");
        MainConsole->RunConsoleCommand(*command);
    }

    QueuedConsoleCommands.clear();
}
// ------------------------------------ //
bool Engine::_ReceiveConsoleInput(const std::string& command)
{
    Invoke([=]() {
        if(MainConsole) {

            MainConsole->RunConsoleCommand(command);

        } else {

            LOG_WARNING("No console handler attached, cannot run command");
        }
    });

    // Listening thread quits if PreReleaseWaiting is true
    return PreReleaseWaiting;
}
// ------------------------------------ //
