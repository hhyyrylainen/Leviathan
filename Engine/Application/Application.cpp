// ------------------------------------ //
#include "Application.h"

#include "FileSystem.h"
#include "Iterators/StringIterator.h"

#include <iostream>

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT LeviathanApplication::LeviathanApplication() : _Engine(new Engine(this))
{
    Curapp = this;
}

//! \brief Version for tests with incomplete engine instance
DLLEXPORT LeviathanApplication::LeviathanApplication(Engine* engine) :
    ExternalEngineInstance(true), _Engine(engine)
{
    LEVIATHAN_ASSERT(_Engine, "no engine pointer given");
    Curapp = this;
}

DLLEXPORT LeviathanApplication::~LeviathanApplication()
{
    // Release should have been called when exiting the main loop
    if(!ExternalEngineInstance)
        SAFE_DELETE(_Engine);
    _Engine = nullptr;
    Curapp = nullptr;
}

DLLEXPORT LeviathanApplication* LeviathanApplication::Get()
{
    return Curapp;
}

LeviathanApplication* LeviathanApplication::Curapp = NULL;
// ------------------------------------ //
DLLEXPORT bool LeviathanApplication::Initialize(AppDef* configuration)
{
    GUARD_LOCK();

    // Store configuration //
    ApplicationConfiguration = configuration;

    // Init engine //
    if(!_Engine->Init(
           ApplicationConfiguration, GetProgramNetType(), _GetApplicationPacketHandler()))
        return false;

    _InternalInit();
    return true;
}

DLLEXPORT void LeviathanApplication::Release()
{
    {
        GUARD_LOCK();
        // set as quitting //
        Quit = true;

        // Nothing else to do if no engine //
        if(!_Engine)
            return;

        // Shutdown the packet handler
        // PreRelease should have been done at this point and the NetworkHandler
        // should have been released so this can no longer be in use
        _ShutdownApplicationPacketHandler();
    }

    // This avoids deadlocking //
    _Engine->Release();

    {
        GUARD_LOCK();
        // Delete the already released engine //
        delete _Engine;
        _Engine = NULL;
    }
}

DLLEXPORT void LeviathanApplication::StartRelease()
{
    GUARD_LOCK();
    ShouldQuit = true;

    // Tell Engine to expect a Release soon //
    _Engine->PreRelease();
}
// ------------------------------------ //
DLLEXPORT void LeviathanApplication::ForceRelease()
{
    GUARD_LOCK();
    ShouldQuit = true;
    Quit = true;

    if(_Engine) {
        // The prelease does some which requires a tick //
        _Engine->PreRelease();
        _Engine->Tick();
        _Engine->Release(true);
    }

    SAFE_DELETE(_Engine);
}
// ------------------------------------ //
DLLEXPORT bool LeviathanApplication::PassCommandLine(int argcount, char* args[])
{
    return _Engine->PassCommandLine(argcount, args);
}

DLLEXPORT void LeviathanApplication::_InternalInit() {}
// ------------------------------------ //
DLLEXPORT void LeviathanApplication::Render()
{
    _Engine->RenderFrame();
}

DLLEXPORT void LeviathanApplication::PreFirstTick() {}
// ------------------------------------ //
DLLEXPORT int LeviathanApplication::RunMessageLoop()
{
    // This is almost at tick so call this outside the loop for performance //
    _Engine->PreFirstTick();
    PreFirstTick();

    // For reporting wait failures //
    int FailCount = 0;

    while(!_Engine->HasPreReleaseBeenDone()) {
        // Store this //
        bool canprocess = _Engine->GetWindowOpenCount() != 0;

        _Engine->MessagePump();

        // Set as quitting //
        if((!canprocess || QuitSometime) && !ShouldQuit) {
            Logger::Get()->Info("Application: starting real close");
            StartRelease();
        }

        // engine tick //
        _Engine->Tick();

        if(ShouldQuit || Quit) {
            // We need to have done a proper run after calling StartRelease //
            continue;
        }

        Render();

        // We could potentially wait here //
        //! TODO: make this wait happen only if tick wasn't actually and no frame was
        //! rendered
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } catch(...) {
            FailCount++;
        }
    }

    // Report problems //
    if(FailCount)
        std::cout << "Application main loop sleep fails: " << FailCount << std::endl;

    // always release before quitting to avoid tons of memory leaks //
    Release();

    return 0;
}
// ------------------------------------ //
DLLEXPORT void LeviathanApplication::ClearTimers()
{

    _Engine->ClearTimers();
}
// ------------------ Default callbacks that do nothing ------------------ //
DLLEXPORT bool LeviathanApplication::InitLoadCustomScriptTypes(asIScriptEngine* engine)
{

    return true;
}

DLLEXPORT void LeviathanApplication::Tick(int mspassed) {}

DLLEXPORT void LeviathanApplication::CustomizeEnginePostLoad() {}

DLLEXPORT void LeviathanApplication::EnginePreShutdown() {}

DLLEXPORT std::shared_ptr<GameWorld> LeviathanApplication::GetGameWorld(int id)
{

    return nullptr;
}


DLLEXPORT void LeviathanApplication::DummyGameConfigurationVariables(
    GameConfiguration* configobj)
{}

DLLEXPORT void LeviathanApplication::DummyGameKeyConfigVariables(
    KeyConfiguration* keyconfigobj)
{}

DLLEXPORT void LeviathanApplication::MarkAsClosing()
{
    QuitSometime = true;
}
// ------------------------------------ //
DLLEXPORT void LeviathanApplication::StartServerProcess(
    const std::string& processname, const std::string& commandline)
{

#ifdef _WIN32
    // Create needed info //
    STARTUPINFOA processstart;
    PROCESS_INFORMATION startedinfo;

    ZeroMemory(&processstart, sizeof(STARTUPINFOA));
    ZeroMemory(&startedinfo, sizeof(PROCESS_INFORMATION));

    processstart.cb = sizeof(STARTUPINFOA);
    // processstart.dwFlags = STARTF_FORCEOFFFEEDBACK;
    // processstart.wShowWindow = SW_SHOWMINIMIZED;

    std::string finalstart = "\"" + processname + "\" " + commandline;

    // Use windows process creation //
    if(!CreateProcessA(NULL, const_cast<char*>(finalstart.c_str()), NULL, NULL, FALSE, 0, NULL,
           NULL, &processstart, &startedinfo)) {
        // Failed to start the process
        Logger::Get()->Error("Failed to start the server process, error code: " +
                             Convert::ToString(GetLastError()));
        return;
    }

    // Close our handles //
    CloseHandle(startedinfo.hThread);
    DEBUG_BREAK;
    // ServerProcessHandle = startedinfo.hProcess;


#else
    // Popen should work //

    // Actually fork might be simpler //
    if(fork() == 0) {
        // We are now in the child process //

        execl(processname.c_str(), commandline.c_str(), (char*)NULL);
    }


#endif // _WIN32
}
// ------------------------------------ //
DLLEXPORT std::vector<std::string> LeviathanApplication::CommandLineStringSplitter(
    const char* str, std::vector<char*>& argcharstrings, bool addprogramname /*= true*/)
{
    StringIterator itr(std::make_unique<UTF8PointerDataIterator>(str, str + std::strlen(str)));

    std::vector<std::string> args;

    if(addprogramname) {
        // TODO: actually detect this somehow
        args.push_back("LeviathanApplication");
    }

    while(!itr.IsOutOfBounds()) {
        auto current =
            itr.GetNextCharacterSequence<std::string>(UNNORMALCHARACTER_TYPE_WHITESPACE);

        if(current && current->size() > 0) {
            if(current->at(0) == '\'' || current->at(0) == '\"') {
                current = StringIterator(current.get())
                              .GetStringInQuotes<std::string>(QUOTETYPE_BOTH);
            }

            args.push_back(*current);
        }
    }

    argcharstrings.resize(args.size());
    for(size_t i = 0; i < args.size(); ++i) {
        argcharstrings[i] = const_cast<char*>(args[i].c_str());
    }
    argcharstrings.push_back(nullptr);

    return args;
}
