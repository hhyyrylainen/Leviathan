// ------------------------------------ //
#include "Engine.h"

#include "Statistics/TimingMonitor.h"
#include "Application/AppDefine.h"
#include "Application/Application.h"
#include "Common/DataStoring/DataStore.h"
#include "Common/StringOperations.h"
#include "Common/Types.h"
#include "Entities/GameWorld.h"
#include "Entities/Serializers/EntitySerializer.h"
#include "Events/EventHandler.h"
#include "Handlers/IDFactory.h"
#include "Handlers/OutOfMemoryHandler.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "Networking/NetworkCache.h"
#include "Networking/NetworkHandler.h"
#include "Networking/NetworkedInputHandler.h"
#include "Networking/RemoteConsole.h"
#include "Newton/NewtonManager.h"
#include "Newton/PhysicsMaterialManager.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Rendering/GraphicalInputEntity.h"
#include "Rendering/Graphics.h"
#include "Script/Console.h"
#include "Sound/SoundDevice.h"
#include "Statistics/RenderingStatistics.h"
#include "Threading/ThreadingManager.h"
#include "Threading/QueuedTask.h"
#include "Utility/Random.h"
#include "TimeIncludes.h"
#include "FileSystem.h"
#include "GUI/GuiManager.h"
#include "Iterators/StringIterator.h"
#include "Application/ConsoleInput.h"

#ifdef LEVIATHAN_USES_LEAP
#include "Leap/LeapManager.h"
#endif

#include <chrono>
#include <future>

using namespace Leviathan;
using namespace std;
// ------------------------------------ //

DLLEXPORT Engine::Engine(LeviathanApplication* owner) :
    Owner(owner)
{
    // This makes sure that uninitialized engine will have at least some last frame time //
	LastTickTime = Time::GetTimeMs64();

#ifdef LEVIATHAN_USES_LEAP
	LeapData = NULL;
#endif

    instance = this;
}

DLLEXPORT Engine::~Engine(){
	// Reset the instance ptr //
	instance = nullptr;

    _ConsoleInput.reset();
}

Engine* Engine::instance = nullptr;

Engine* Engine::GetEngine(){
	return instance;
}

DLLEXPORT Engine* Engine::Get(){
	return instance;
}
// ------------------------------------ //
DLLEXPORT bool Engine::Init(AppDef*  definition, NETWORKED_TYPE ntype){
    
	GUARD_LOCK();
    
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
	if(!_ThreadingManager->Init()){

		Logger::Get()->Error("Engine: Init: cannot start threading");
		return false;
	}

	// Create the randomizer //
	MainRandom = new Random((int)InitStartTime);
	MainRandom->SetAsMain();

    // Console might be the first thing we want //
    if(!NoSTDInput){

        _ConsoleInput = std::make_unique<ConsoleInput>();

        if(!_ConsoleInput->Init(std::bind(&Engine::_ReceiveConsoleInput, this,
                    std::placeholders::_1), NoGui ? true : false))
        {
            Logger::Get()->Error("Engine: Init: failed to read stdin, perhaps pass --nocin");
            return false;
        }
    }
    
	if(NoGui){

		// Tell window title //
		Logger::Get()->Write("// ----------- "+Define->GetWindowDetails().Title+
            " ----------- //");
	}
	

	// We could immediately receive a remote console request so this should be
    // ready when networking is started
	_RemoteConsole = new RemoteConsole();

	// We want to send a request to the master server as soon as possible //
    {
        Lock lock(NetworkHandlerLock);
        
        _NetworkHandler = new NetworkHandler(ntype, Define->GetPacketHandler());

        _NetworkHandler->Init(Define->GetMasterServerInfo());
    }

	// These should be fine to be threaded //

	// File change listener //
	_ResourceRefreshHandler = new ResourceRefreshHandler();
	if(!_ResourceRefreshHandler->Init()){

		Logger::Get()->Error("Engine: Init: cannot start resource monitor");
		return false;
	}

	// Data storage //
	Mainstore = new DataStore(true);
	if(!Mainstore){

		Logger::Get()->Error("Engine: Init: failed to create main data store");
		return false;
	}

	// Search data folder for files //
	MainFileHandler = new FileSystem();
	if(!MainFileHandler){

		Logger::Get()->Error("Engine: Init: failed to create FileSystem");
		return false;
	}

	if(!MainFileHandler->Init(Logger::Get())){

		Logger::Get()->Error("Engine: Init: failed to init FileSystem");
		return false;
	}

	// File parsing //
	ObjectFileProcessor::Initialize();

	// Main program wide event dispatcher //
	MainEvents = new EventHandler();
	if(!MainEvents){

		Logger::Get()->Error("Engine: Init: failed to create MainEvents");
		return false;
	}

	if(!MainEvents->Init()){

		Logger::Get()->Error("Engine: Init: failed to init MainEvents");
		return false;
	}

	// Check is threading properly started //
	if(!_ThreadingManager->CheckInit()){

		Logger::Get()->Error("Engine: Init: threading start failed");
		return false;
	}

	// create script interface before renderer //
	std::promise<bool> ScriptInterfaceResult;
    
	// Ref is OK to use since this task finishes before this function //
    _ThreadingManager->QueueTask(std::make_shared<QueuedTask>(std::bind<void>([](
                    std::promise<bool> &returnvalue, Engine* engine) -> void
        {
            try{
                engine->MainScript = new ScriptExecutor();
            } catch(const Exception&){

                Logger::Get()->Error("Engine: Init: failed to create ScriptInterface");
                returnvalue.set_value(false);
                return;                
            }

            // create console after script engine //
            engine->MainConsole = new ScriptConsole();
            if(!engine->MainConsole){

                Logger::Get()->Error("Engine: Init: failed to create ScriptConsole");
                returnvalue.set_value(false);
                return;
            }

            if(!engine->MainConsole->Init(engine->MainScript)){

                Logger::Get()->Error("Engine: Init: failed to initialize Console, "
                    "continuing anyway");
            }

            returnvalue.set_value(true);
        }, std::ref(ScriptInterfaceResult), this)));

	// create newton manager before any newton resources are needed //
	std::promise<bool> NewtonManagerResult;
    
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(std::make_shared<QueuedTask>(std::bind<void>([](
                    std::promise<bool> &returnvalue, Engine* engine) -> void
        {

            engine->_NewtonManager = new NewtonManager();
            if(!engine->_NewtonManager){

                Logger::Get()->Error("Engine: Init: failed to create NewtonManager");
                returnvalue.set_value(false);
                return;
            }

            // next force application to load physical surface materials //
            engine->PhysMaterials = new PhysicsMaterialManager(engine->_NewtonManager);
            if(!engine->PhysMaterials){

                Logger::Get()->Error("Engine: Init: failed to create PhysicsMaterialManager");
                returnvalue.set_value(false);
                return;
            }

            engine->Owner->RegisterApplicationPhysicalMaterials(engine->PhysMaterials);

            returnvalue.set_value(true);
        }, std::ref(NewtonManagerResult), this)));

    // Create the default serializer //
    _EntitySerializer = std::make_unique<EntitySerializer>();
    if(!_EntitySerializer){

        Logger::Get()->Error("Engine: Init: failed to instantiate entity serializer");
        return false;
    }
    
	// Check if we don't want a window //
	if(NoGui){

		Logger::Get()->Info("Engine: Init: starting in console mode "
            "(won't allocate graphical objects) ");

        if(!_ConsoleInput->IsAttachedToConsole()){

            Logger::Get()->Error("Engine: Init: in nogui mode and no input terminal connected, "
                "quitting");
            return false;
        }
        
	} else {

		ObjectFileProcessor::LoadValueFromNamedVars<int>(Define->GetValues(), "MaxFPS",
            FrameLimit, 120, Logger::Get(), "Graphics: Init:");

		Graph = new Graphics();

	}

	// We need to wait for all current tasks to finish //
	_ThreadingManager->WaitForAllTasksToFinish();

	// Check return values //
	if(!ScriptInterfaceResult.get_future().get() || !NewtonManagerResult.get_future().get())
	{

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
	if(!NoLeap){
        
        Logger::Get()->Info("Engine: will try to create Leap motion connection");

        // Seems that std::threads are joinable when constructed with default constructor
		leapinitthread = std::thread(std::bind<void>([](Engine* engine) -> void{

			engine->LeapData = new LeapManager(engine);
			if(!engine->LeapData){
				Logger::Get()->Error("Engine: Init: failed to create LeapManager");
				return;
			}
			// try here just in case //
			try{
				if(!engine->LeapData->Init()){

					Logger::Get()->Info("Engine: Init: No Leap controller found, not using one");
				}
			}
			catch(...){
				// threw something //
				Logger::Get()->Error("Engine: Init: Leap threw something, even without leap "
                    "this shouldn't happen; continuing anyway");
			}

		}, this));
    }
#endif


	// sound device //
	std::promise<bool> SoundDeviceResult;
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(std::make_shared<QueuedTask>(std::bind<void>([](
                    std::promise<bool> &returnvalue, Engine* engine) -> void
        {
                    
            if(!engine->NoGui){
                engine->Sound = new SoundDevice();
                            
                if(!engine->Sound){
                    Logger::Get()->Error("Engine: Init: failed to create Sound");
                    returnvalue.set_value(false);
                    return;
                }

                if(!engine->Sound->Init()){

                    Logger::Get()->Error("Engine: Init: failed to init SoundDevice");
                    returnvalue.set_value(false);
                    return;
                }
            }

            // make angel script make list of registered stuff //
            engine->MainScript->ScanAngelScriptTypes();

            if(!engine->NoGui){
                // measuring //
                engine->RenderTimer = new RenderingStatistics();
                if(!engine->RenderTimer){
                    Logger::Get()->Error(
                        "Engine: Init: failed to create RenderingStatistics");
                            
                    returnvalue.set_value(false);
                    return;
                }
            }

            returnvalue.set_value(true);
        }, std::ref(SoundDeviceResult), this)));

	if(!NoGui){
		if(!Graph){

			Logger::Get()->Error("Engine: Init: failed to create instance of Graphics");
			return false;
		}

		// call init //
		if(!Graph->Init(definition)){
			Logger::Get()->Error("Failed to init Engine, Init graphics failed! Aborting");
			return false;
		}

		// create window //
		GraphicalEntity1 = new GraphicalInputEntity(Graph, definition);
	}

	if(!SoundDeviceResult.get_future().get()){

		Logger::Get()->Error("Engine: Init: sound device queued tasks failed");
		return false;
	}

#ifdef LEVIATHAN_USES_LEAP
	// We can probably assume here that leap creation has stalled if the thread is running //
	if(!NoLeap){

        auto start = WantedClockType::now();
        
        while(leapinitthread.joinable()){

            auto elapsed = WantedClockType::now() - start;

            if(elapsed > std::chrono::milliseconds(150)){

                Logger::Get()->Warning("LeapController creation would have stalled the game!");
                Logger::Get()->Write("TODO: allow increasing wait period");
                leapinitthread.detach();
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
	}
#endif

	PostLoad();

	Logger::Get()->Info("Engine init took " + Convert::ToString(Time::GetTimeMs64()
            - InitStartTime) + " ms");
    
	return true;
}

void Engine::PostLoad(){
	// increase start count //
	int startcounts = 0;

	if(Mainstore->GetValueAndConvertTo<int>("StartCount", startcounts)){
		// increase //
		Mainstore->SetValue("StartCount", new VariableBlock(new IntBlock(startcounts+1)));
	} else {

		Mainstore->AddVar(new NamedVariableList("StartCount", new VariableBlock(1)));
		// set as persistent //
		Mainstore->SetPersistance("StartCount", true);
	}

    // Check if we are attached to a terminal //

    ClearTimers();
    
	// get time //
	LastTickTime = Time::GetTimeMs64();
}

void Engine::Release(bool forced){
	GUARD_LOCK();

	if(!forced)
		LEVIATHAN_ASSERT(PreReleaseDone, "PreReleaseDone must be done before actual release!");

	// Destroy worlds //
    {
        Lock lock(GameWorldsLock);
        
        while(GameWorlds.size()){

            GameWorlds[0]->Release();
            GameWorlds.erase(GameWorlds.begin());
        }

    }

    _NetworkHandler->ShutdownCache();
	
	// Wait for tasks to finish //
	if(!forced)
		_ThreadingManager->WaitForAllTasksToFinish();

    // Make windows clear their stored objects //
    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++){

        AdditionalGraphicalEntities[i]->ReleaseLinked();
        SAFE_DELETE(AdditionalGraphicalEntities[i]);
    }

    AdditionalGraphicalEntities.clear();

    // Finally the main window //
	if(GraphicalEntity1){

		GraphicalEntity1->ReleaseLinked();
	}

	// Destroy windows //
	SAFE_DELETE(GraphicalEntity1);

	// Release newton //
	SAFE_DELETE(PhysMaterials);
	SAFE_DELETE(_NewtonManager);

#ifdef LEVIATHAN_USES_LEAP
	SAFE_RELEASEDEL(LeapData);
#endif

	// Console needs to be released before script release //
	SAFE_RELEASEDEL(MainConsole);

	SAFE_DELETE(MainScript);

	// Save at this point (just in case it crashes before exiting) //
	Logger::Get()->Save();

	SAFE_RELEASEDEL(Graph);
	SAFE_DELETE(RenderTimer);

    _EntitySerializer.reset();

	SAFE_RELEASEDEL(Sound);
	SAFE_DELETE(Mainstore);

	SAFE_RELEASEDEL(MainEvents);
	// delete randomizer last, for obvious reasons //
	SAFE_DELETE(MainRandom);

	Gui::GuiManager::KillGlobalCache();

	ObjectFileProcessor::Release();
	SAFE_DELETE(MainFileHandler);

	// Stop threads //
	if(!forced)
		_ThreadingManager->WaitForAllTasksToFinish();
	SAFE_RELEASEDEL(_ThreadingManager);

	// clears all running timers that might have accidentally been left running //
	TimingMonitor::ClearTimers();

	// safe to delete this here //
	SAFE_DELETE(OutOMemory);

	SAFE_DELETE(IDDefaultInstance);

	Logger::Get()->Write("Goodbye cruel world!");
}
// ------------------------------------ //
void Engine::Tick(){

    // Always try to update networking //
    {
        Lock lock(NetworkHandlerLock);
        
        if(_NetworkHandler)
            _NetworkHandler->UpdateAllConnections();
        
    }
    
    // Update physics //
    SimulatePhysics();

    
	GUARD_LOCK();

	if(PreReleaseWaiting){

		PreReleaseWaiting = false;
		PreReleaseDone = true;

		Logger::Get()->Info("Engine: performing final release tick");

        
        
		// Call last tick event //
        
	}

	// Get the passed time since the last update //
	auto CurTime = Time::GetTimeMs64();
	TimePassed = (int)(CurTime-LastTickTime);


	if((TimePassed < TICKSPEED)){
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

	if(!NoGui){
		// sound tick //
		if(Sound)
		   Sound->Tick(TimePassed);

		// update windows //
        if(GraphicalEntity1)
            GraphicalEntity1->Tick(TimePassed);

        for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++){

            AdditionalGraphicalEntities[i]->Tick(TimePassed);
        }
	}


    // Update worlds //
    {
        Lock lock(GameWorldsLock);

        // This will also update physics //
        auto end = GameWorlds.end();
        for(auto iter = GameWorlds.begin(); iter != end; ++iter){

            (*iter)->Tick(TickCount);
        }
    }
    
    
	// Some dark magic here //
	if(TickCount % 25 == 0){
		// update values
		Mainstore->SetTickCount(TickCount);
		Mainstore->SetTickTime(TickTime);

		if(!NoGui){
			// send updated rendering statistics //
			RenderTimer->ReportStats(Mainstore);
		}
	}

	// Update file listeners //
	if(_ResourceRefreshHandler)
		_ResourceRefreshHandler->CheckFileStatus();

	// Send the tick event //
	MainEvents->CallEvent(new Event(EVENT_TYPE_TICK, new IntegerEventData(TickCount)));

	// Call the default app tick //
	Owner->Tick(TimePassed);

    // Detect closed windows //
    if(GraphicalEntity1 && !GraphicalEntity1->GetWindow()->IsOpen()){

        // Window closed //
        ReportClosedWindow(guard, GraphicalEntity1);
    }

    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++){

        if(!AdditionalGraphicalEntities[i]->GetWindow()->IsOpen()){

            ReportClosedWindow(guard, AdditionalGraphicalEntities[i]);
            
            // The above call might change the vector so stop looping after it //
            break;
        }
    }
    
	TickTime = (int)(Time::GetTimeMs64()-CurTime);
}

DLLEXPORT void Engine::PreFirstTick(){

    // Stop this handling as it is no longer required //
    {
        std::unique_lock<std::mutex> lock(NetworkHandlerLock);

        GUARD_LOCK_OTHER(_NetworkHandler);
        if(_NetworkHandler)
            _NetworkHandler->StopOwnUpdaterThread(guard);
    }
    
    GUARD_LOCK();

    if(_ThreadingManager)
        _ThreadingManager->NotifyQueuerThread();

    ClearTimers();
    
	Logger::Get()->Info("Engine: PreFirstTick: everything fine to start running");
}

DLLEXPORT void Engine::SimulatePhysics(){
    Lock lock(GameWorldsLock);

    auto end = GameWorlds.end();
    for(auto iter = GameWorlds.begin(); iter != end; ++iter){

        (*iter)->SimulatePhysics();
    }
}
// ------------------------------------ //
void Engine::RenderFrame(){
	// We want to totally ignore this if we are in text mode //
	if(NoGui)
		return;

	int SinceLastFrame = -1;
	GUARD_LOCK();

	// limit check //
	if(!RenderTimer->CanRenderNow(FrameLimit, SinceLastFrame)){
        
		// fps would go too high //
		return;
	}

	// since last frame is in microseconds 10^-6 convert to milliseconds //
	// SinceLastTickTime is always more than 1000 (always 1 ms or more) //
	SinceLastFrame /= 1000;
	FrameCount++;

	// advanced statistic start monitoring //
	RenderTimer->RenderingStart();

	MainEvents->CallEvent(new Event(EVENT_TYPE_FRAME_BEGIN,
            new IntegerEventData(SinceLastFrame)));

    // Run rendering systems //
    int64_t timeintick = Time::GetTimeMs64() - LastTickTime;
    int moreticks = 0;

    while(timeintick > TICKSPEED){

        timeintick -= TICKSPEED;
        moreticks++;
    }

    {
        Lock lock(GameWorldsLock);
        
        for(auto iter = GameWorlds.begin(); iter != GameWorlds.end(); ++iter){

            (*iter)->RunFrameRenderSystems(TickCount + moreticks, static_cast<int>(timeintick));
        }
    }
    

	bool shouldrender = false;

	// Render //
	if(GraphicalEntity1 && GraphicalEntity1->Render(SinceLastFrame))
		shouldrender = true;

    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++){

        if(AdditionalGraphicalEntities[i]->Render(SinceLastFrame))
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
DLLEXPORT void Engine::PreRelease(){
	GUARD_LOCK();
	if(PreReleaseWaiting || PreReleaseCompleted)
		return;
	
	PreReleaseWaiting = true;
	// This will stay true until the end of times //
	PreReleaseCompleted = true;

	// Stop command handling first //
    if(_ConsoleInput){

        _ConsoleInput->Release(false);
        Logger::Get()->Info("Successfully stopped command handling");
    }

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
        
        for(auto iter = GameWorlds.begin(); iter != GameWorlds.end(); ++iter){
            // Set all objects to release //
            (*iter)->MarkForClear();
        }
    }

	// Set tasks to a proper state //
	_ThreadingManager->SetDiscardConditionalTasks(true);
	_ThreadingManager->SetDisallowRepeatingTasks(true);

	Logger::Get()->Info("Engine: prerelease done, waiting for a tick");
}
// ------------------------------------ //
DLLEXPORT void Engine::SaveScreenShot(){
	LEVIATHAN_ASSERT(!NoGui, "really shouldn't try to screenshot in text-only mode");
	GUARD_LOCK();

	const string fileprefix = MainFileHandler->GetDataFolder()+"Screenshots/Captured_frame_";

	GraphicalEntity1->SaveScreenShot(fileprefix);
}

DLLEXPORT int Engine::GetWindowOpenCount(){
	int openwindows = 0;

	// If we are in text only mode always return 1 //
	if(NoGui)
		return 1;
    
	GUARD_LOCK();
    

	if(GraphicalEntity1 && GraphicalEntity1->GetWindow()->IsOpen())
		openwindows++;

    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++){

        if(AdditionalGraphicalEntities[i]->GetWindow()->IsOpen())
            openwindows++;
    }

	return openwindows;
}
// ------------------------------------ //
DLLEXPORT GraphicalInputEntity* Engine::OpenNewWindow(){

    AppDef winparams;


    winparams.SetWindowDetails(WindowDataDetails("My Second window", 1280, 720, true, true,
#ifdef _WIN32
            NULL,
#endif            
            NULL));
    
    
    auto newwindow = std::make_unique<GraphicalInputEntity>(Graph, &winparams);

    GUARD_LOCK();
    
    AdditionalGraphicalEntities.push_back(newwindow.get());
    
    return newwindow.release();
}

DLLEXPORT void Engine::ReportClosedWindow(Lock &guard,
    GraphicalInputEntity* windowentity)
{

    windowentity->ReleaseLinked();

    if(GraphicalEntity1 == windowentity){

        SAFE_DELETE(GraphicalEntity1);
        return;
    }

    for(size_t i = 0; i < AdditionalGraphicalEntities.size(); i++){

        if(AdditionalGraphicalEntities[i] == windowentity){
            
            SAFE_DELETE(AdditionalGraphicalEntities[i]);
            AdditionalGraphicalEntities.erase(AdditionalGraphicalEntities.begin()+i);
            
            return;
        }
    }

    // Didn't find the target //
    Logger::Get()->Error("Engine: couldn't find closing GraphicalInputEntity");
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<GameWorld> Engine::CreateWorld(GraphicalInputEntity* owningwindow,
    std::shared_ptr<ViewerCameraPos> worldscamera)
{
    
	auto tmp = make_shared<GameWorld>(_NetworkHandler->GetNetworkType());
    
	tmp->Init(owningwindow, NoGui ? NULL: Graph->GetOgreRoot());
    
	if(owningwindow)
		owningwindow->LinkObjects(worldscamera, tmp);
    
    Lock lock(GameWorldsLock);
    
	GameWorlds.push_back(tmp);
	return GameWorlds.back();
}

DLLEXPORT void Engine::DestroyWorld(shared_ptr<GameWorld> &world){

    if(!world)
        return;
    
    // Release the world first //
    world->Release();

    // Then delete it //
    Lock lock(GameWorldsLock);
    
    auto end = GameWorlds.end();
    for(auto iter = GameWorlds.begin(); iter != end; ++iter){

        if((*iter).get() == world.get()){

            GameWorlds.erase(iter);
            world.reset();
            return;
        }
    }

    // Should be fine destroying worlds that aren't on the list... //
    world.reset();
}
// ------------------------------------ //
DLLEXPORT void Engine::ClearTimers(){
    Lock lock(GameWorldsLock);

    auto end = GameWorlds.end();
    for(auto iter = GameWorlds.begin(); iter != end; ++iter){

        (*iter)->ClearTimers();
    }
}
// ------------------------------------ //
void Engine::_NotifyThreadsRegisterOgre(){
	if(NoGui)
		return;
    
	// Register threads to use graphical objects //
	_ThreadingManager->MakeThreadsWorkWithOgre();
}
// ------------------------------------ //
DLLEXPORT int64_t Leviathan::Engine::GetTimeSinceLastTick() const
{

    return Time::GetTimeMs64()-LastTickTime;
}

DLLEXPORT int Engine::GetCurrentTick() const {

    return TickCount;
}
// ------------------------------------ //
void Engine::_AdjustTickClock(int amount, bool absolute /*= true*/){

    GUARD_LOCK();
    
    if(!absolute){

        Logger::Get()->Info("Engine: adjusted tick timer by "+Convert::ToString(amount));
        
        LastTickTime += amount;
        return;
    }

    // Calculate the time in the current last tick //
    int64_t templasttick = LastTickTime;

    int64_t curtime = Time::GetTimeMs64();

    while(curtime-templasttick >= TICKSPEED){

        templasttick += TICKSPEED;
    }

    // Check how far off we are from the target //
    int64_t intolasttick = curtime - templasttick;

    int changeamount = amount - static_cast<int>(intolasttick);

    Logger::Get()->Info("Engine: changing tick counter by " + Convert::ToString(changeamount));

    LastTickTime += changeamount;
}

void Engine::_AdjustTickNumber(int tickamount, bool absolute){

    GUARD_LOCK();

    if(!absolute){

        TickCount += tickamount;

        Logger::Get()->Info("Engine: adjusted tick by "+Convert::ToString(tickamount)
            +", tick is now "+Convert::ToString(TickCount));
        
        return;
    }

    TickCount = tickamount;

    Logger::Get()->Info("Engine: tick set to "+Convert::ToString(TickCount));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Engine::DumpMemoryLeaks() {

    LOG_INFO("TODO: memory leak detection, or remove this function");
}

int TestCrash(int writenum){

    int* target = nullptr;
    (*target) = writenum;
    
    Logger::Get()->Write("It didn't crash...");
    return 42;
}

DLLEXPORT void Engine::PassCommandLine(const string &commands){

	Logger::Get()->Info("Command line: "+commands);

	GUARD_LOCK();
	// Split all flags and check for some flags that might be set //
	StringIterator itr(commands);
	unique_ptr<string> splitval;

	while((splitval = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_WHITESPACE))
        != NULL)
    {

		if(*splitval == "--nogui"){
			NoGui = true;
			Logger::Get()->Info("Engine starting in non-GUI mode");
			continue;
		}
		if(*splitval == "--noleap"){
			NoLeap = true;

#ifdef LEVIATHAN_USES_LEAP
			Logger::Get()->Info("Engine starting with LeapMotion disabled");
#endif
			continue;
		}
        if(*splitval == "--nocin"){

            NoSTDInput = true;
            continue;
        }
		if(*splitval == "--nonothing"){
			// Shouldn't try to open the console on windows //
			DEBUG_BREAK;
		}
        if(*splitval == "--crash"){
            
            Logger::Get()->Info("Engine testing crash handling");
            // TODO: write a file that disables crash handling
            // Make the log say something useful //
            Logger::Get()->Save();

            // Test crashing //
            TestCrash(12);
            
            continue;
        }
        
		// Add (if not processed already) //
		PassedCommands.push_back(move(splitval));
	}
}

DLLEXPORT void Engine::ExecuteCommandLine(){
	GUARD_LOCK();

	StringIterator itr(NULL, false);

	// Iterate over the commands and process them //
	for(size_t i = 0; i < PassedCommands.size(); i++){
        
		itr.ReInit(PassedCommands[i].get());
		// Skip the preceding '-'s //
		itr.SkipCharacters('-');

		// Get the command //
		auto firstpart = itr.GetUntilNextCharacterOrAll<string>(':');

		// Execute the wanted command //
		if(StringOperations::CompareInsensitive<string>(*firstpart, "RemoteConsole")){
			
			// Get the next command //
			auto commandpart = itr.GetUntilNextCharacterOrAll<string>(L':');

			if(*commandpart == "CloseIfNone"){
				// Set the command //
				_RemoteConsole->SetCloseIfNoRemoteConsole(true);
				Logger::Get()->Info("Engine will close when no active/waiting remote console "
                    "sessions");

			} else if(*commandpart == "OpenTo"){
				// Get the to part //
				auto topart = itr.GetStringInQuotes<string>(QUOTETYPE_BOTH);

				int token = 0;

				auto numberpart = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_NONE);

				if(numberpart->size() == 0){

					Logger::Get()->Warning("Engine: ExecuteCommandLine: RemoteConsole: "
                        "no token number provided");
					continue;
				}
				// Convert to a real number. Maybe we could see if the token is
                // complex enough here, but that isn't necessary
				token = Convert::StringTo<int>(*numberpart);

				if(token == 0){
					// Invalid number? //
					Logger::Get()->Warning("Engine: ExecuteCommandLine: RemoteConsole: "
                        "couldn't parse token number, " + *numberpart);
					continue;
				}

				// Create a connection (or potentially use an existing one) //
				shared_ptr<Connection> tmpconnection =
                    _NetworkHandler->OpenConnectionTo(*topart);

				// Tell remote console to open a command to it //
				if(tmpconnection){

					_RemoteConsole->OfferConnectionTo(tmpconnection, "AutoOpen",
                        token);

				} else {
					// Something funky happened... //
					Logger::Get()->Warning("Engine: ExecuteCommandLine: RemoteConsole: "
                        "couldn't open connection to "+*topart+", couldn't resolve address");
				}

			} else {
				// Unknown command //
				Logger::Get()->Warning("Engine: ExecuteCommandLine: unknown RemoteConsole "
                    "command: "+*commandpart+", whole argument: "+*PassedCommands[i]);
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
bool Engine::_ReceiveConsoleInput(const std::string &command){

    GUARD_LOCK();

    if(MainConsole){

        MainConsole->RunConsoleCommand(command);

    } else {
        
        LOG_WARNING("No console handler attached, cannot run command");
    }
    
    // Listening thread quits if PreReleaseWaiting is true
    return PreReleaseWaiting;
}
