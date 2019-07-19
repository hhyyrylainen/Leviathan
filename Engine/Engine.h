// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Entities/WorldNetworkSettings.h"
#include "Networking/CommonNetwork.h"

#include <functional>
#include <inttypes.h>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>


namespace Leviathan {

namespace Editor {
class Editor;
class Importer;
} // namespace Editor

class GameModuleLoader;

//! \brief The main class of the Leviathan Game Engine
//!
//! Allocates a lot of classes and performs almost all startup operations.
//! \note Only some operations are thread safe
//! \todo Reduce the amount of logging and mention thread safety on methods that ARE thread
//! safe and add note to assume that everything else isn't
class Engine : public ThreadSafe {

    friend Window;
    friend GUI::GuiManager;
    friend GameWorld;
    friend LeviathanApplication;

public:
    DLLEXPORT Engine(LeviathanApplication* owner);
    DLLEXPORT ~Engine();

    DLLEXPORT bool Init(
        AppDef* definition, NETWORKED_TYPE ntype, NetworkInterface* packethandler);

    //! \todo Add a thread that monitors if the thing gets stuck on a task
    DLLEXPORT void Release(bool forced = false);

    //! \brief Sets objects ready to be released
    //! \note The Tick function must be called after this but before Release
    DLLEXPORT void PreRelease();

    //! \brief Checks if PreRelease is done and Release can be called
    //! \pre PreRelease is called
    DLLEXPORT inline bool HasPreReleaseBeenDone() const
    {
        return PreReleaseDone;
    }

    //! \brief Calculates how long has elapsed since the last tick
    //! \return The time in milliseconds
    DLLEXPORT int64_t GetTimeSinceLastTick() const;

    //! \brief Returns the number of tick that was last simulated
    DLLEXPORT int GetCurrentTick() const;

    //! \brief Processes queued messages from Ogre, SDL and input
    DLLEXPORT void MessagePump();


    DLLEXPORT void Tick();
    DLLEXPORT void RenderFrame();
    DLLEXPORT void PreFirstTick();

    DLLEXPORT int GetWindowOpenCount();

    //! \brief Clears physical timers
    DLLEXPORT void ClearTimers();

    //! \brief Marks the owning application to quit
    DLLEXPORT void MarkQuit();

    // ------------------------------------ //
    // Threading support

    //! \brief Runs function on the main thread before the next tick
    //!
    //! This is provided to be able to
    //! \note This maybe called after preshutdown has started before final tick.
    //! This means that some objects may no longer be valid so check the result of
    //! any Get functions called in the invoke
    //! \todo Write a wrapper file that can be used for invoking without having to include this
    //! huge file
    DLLEXPORT void Invoke(const std::function<void()>& function);

    //! \brief Runs the function now if on the main thread otherwise calls Invoke
    DLLEXPORT void RunOnMainThread(const std::function<void()>& function);

    //! \brief Returns true if called on the main thread
    DLLEXPORT bool IsOnMainThread() const;

    //! \brief Asserts if not called on the main thread
    DLLEXPORT inline void AssertIfNotMainThread() const
    {
        LEVIATHAN_ASSERT(
            IsOnMainThread(), "Function not called on main thread (AssertIfNotMainThread)");
    }


    // ------------------------------------ //
    //! Passes the commands and preprocesses them
    //!
    //! Also interprets commands like --nogui
    //! \returns False if invalid command line and the game should quit instantly
    DLLEXPORT bool PassCommandLine(int argcount, char* args[]);

    //! \brief Creates a GameWorld for placing entities into
    //! \note To actually move the world camera you need to use
    //! Leviathan::ObjectLoader::LoadCamera to create a camera entity
    //! \param worldtype Is the type of the world to be created. This is passed to the game's
    //! registered world factory. Types over 1024 are reserved for inbuilt types and are needed
    //! for example things like the editor
    //! \param physicsMaterials The physical materials used for the world. This must be
    //! non-null if physics is wanted for this world. This is here in order to allow inbuilt
    //! worlds to have custom materials
    //! \param overrideid Set to >= 0 if this is a clientside world and must have the specific
    //! id
    DLLEXPORT std::shared_ptr<GameWorld> CreateWorld(Window* owningwindow, int worldtype,
        const std::shared_ptr<PhysicsMaterialManager>& physicsMaterials,
        const WorldNetworkSettings& networking, int overrideid = -1);

    //! \brief Releases a GameWorld
    //! \param world The world to destroy. This is taken as a const reference to allow
    //! shared_ptrs of derived types to be passed here. Make sure to call world.reset()
    //! afterwards to release the reference.
    //! \post The World will have been released and removed from Engine's internal list and
    //! when all other holders of the pointer release it will be deleted
    DLLEXPORT void DestroyWorld(const std::shared_ptr<GameWorld>& world);

    //! \brief Opens a new window
    //! \note The window may become broken if the main window is closed
    //! \todo Allow changing the parameters
    DLLEXPORT Window* OpenNewWindow();

    //! \brief Closes a window
    //! \note Due to Ogre related issues the window isn't actually closed before shutting down
    //! completely
    //! \returns True if closed. False if window was invalid
    //!
    //! This is a wrapper for ReportClosedWindow
    DLLEXPORT bool CloseWindow(Window* window);

    //! \brief Returns the main window
    DLLEXPORT Window* GetWindowEntity()
    {
        return GraphicalEntity1;
    }

    //! \brief Removes an closed window from the engine
    DLLEXPORT void ReportClosedWindow(Window* windowentity);

    //! \returns True if window is a valid open Window
    DLLEXPORT bool IsValidWindow(Window* window) const;

    DLLEXPORT void SaveScreenShot();

    // ------------------------------------ //
    //! \brief Opens an Editor::Editor window
    //! \todo Exit fullscreen mode if main window is in fullscreen mode
    DLLEXPORT void OpenEditorWindow(Window* useexistingwindow = nullptr);

    //! \brief Focuses the first editor or opens an editor if none are open
    //! \todo Closed editors need to report that they are closed in order to open one again
    DLLEXPORT void FocusOrOpenEditor();

    // ------------------------------------ //
    inline Graphics* GetGraphics()
    {
        return Graph;
    }

    inline bool IsInGraphicalMode() const
    {
        return !NoGui;
    }

    inline EventHandler* GetEventHandler()
    {
        return MainEvents;
    }
    inline RenderingStatistics* GetRenderingStatistics()
    {
        return RenderTimer;
    }
    inline ScriptConsole* GetScriptConsole()
    {
        return MainConsole;
    }
    inline FileSystem* GetFileSystem()
    {
        return MainFileHandler;
    }
    inline AppDef* GetDefinition()
    {
        return Define;
    }
    inline LeviathanApplication* GetOwningApplication()
    {
        return Owner;
    }
    inline NetworkHandler* GetNetworkHandler()
    {
        return _NetworkHandler;
    }
    inline ThreadingManager* GetThreadingManager()
    {
        return _ThreadingManager;
    }
    inline SoundDevice* GetSoundDevice()
    {
        return Sound;
    }
    inline ResourceRefreshHandler* GetResourceRefreshHandler()
    {
        return _ResourceRefreshHandler;
    }
    inline RemoteConsole* GetRemoteConsole()
    {
        return _RemoteConsole;
    }
    inline Random* GetRandom()
    {
        return MainRandom;
    }
    inline GameModuleLoader* GetGameModuleLoader()
    {
        return _GameModuleLoader.get();
    }
    inline ScriptExecutor* GetScriptExecutor()
    {
        return MainScript;
    }

    DLLEXPORT Window* GetWindowFromSDLID(uint32_t sdlid);

#ifdef LEVIATHAN_USES_LEAP
    inline LeapManager* GetLeapManager()
    {
        return LeapData;
    }
#endif

    inline bool GetNoGui()
    {
        return NoGui;
    }

    // Command line settings can only be set before initializing //
    inline void SetNoGUI()
    {

        NoGui = true;
    }

    // Static access //
    DLLEXPORT static Engine* GetEngine();
    DLLEXPORT static Engine* Get();

protected:
    // after load function //
    void PostLoad();

    //! Runs the normal commands passed by the PassCommandLine function //
    //! Ran automatically after Init
    DLLEXPORT void ExecuteCommandLine();

    //! Function called by first instance of Window class after creating a window to not error
    //! when registering threads to work with Ogre
    void _NotifyThreadsRegisterOgre();

    //! \brief Sets the tick clock to a certain value
    //! \note Should only be used to match the server's clock
    //! \param amount The amount of time in milliseconds to set or change
    //! \param absolute When true sets the time until a tick to amount otherwise
    //! changes the remaining
    //! time by amount
    void _AdjustTickClock(int amount, bool absolute = true);

    //! \brief Sets the tick number to a specified value
    //! \note Should only be called on the client as this may break some simulations
    void _AdjustTickNumber(int tickamount, bool absolute);

    //! \brief Handles InvokeQueue
    DLLEXPORT void ProcessInvokes();

    //! Console input comes through this
    bool _ReceiveConsoleInput(const std::string& command);

    //! Runs all commands in QueuedConsoleCommands
    void _RunQueuedConsoleCommands();

    // ------------------------------------ //
    AppDef* Define = nullptr;

    RenderingStatistics* RenderTimer = nullptr;
    Graphics* Graph = nullptr;

    Window* GraphicalEntity1 = nullptr;
    std::vector<Window*> AdditionalGraphicalEntities;

    SoundDevice* Sound = nullptr;
    DataStore* Mainstore = nullptr;
    EventHandler* MainEvents = nullptr;
    ScriptExecutor* MainScript = nullptr;
    ScriptConsole* MainConsole = nullptr;
    FileSystem* MainFileHandler = nullptr;
    Random* MainRandom = nullptr;
    OutOfMemoryHandler* OutOMemory = nullptr;
    NetworkHandler* _NetworkHandler = nullptr;
    ThreadingManager* _ThreadingManager = nullptr;
    RemoteConsole* _RemoteConsole = nullptr;
    ResourceRefreshHandler* _ResourceRefreshHandler = nullptr;

    std::unique_ptr<ConsoleInput> _ConsoleInput;
    std::unique_ptr<GameModuleLoader> _GameModuleLoader;
    std::vector<std::unique_ptr<Editor::Editor>> OpenedEditors;

#ifdef LEVIATHAN_USES_LEAP
    LeapManager* LeapData = nullptr;
#endif


    IDFactory* IDDefaultInstance = nullptr;
    LeviathanApplication* Owner = nullptr;

    //! List of current worlds
    std::vector<std::shared_ptr<GameWorld>> GameWorlds;

    //! Mutex that is locked when changing the worlds
    std::mutex GameWorldsLock;

    //! Mutex that is locked while NetworkHandler is used
    std::mutex NetworkHandlerLock;

    // data //
    int64_t LastTickTime;

    int TimePassed = 0;
    int FrameLimit = 0;
    int TickCount = 0;
    int TickTime = 0;
    int FrameCount = 0;

    //! Set when PreRelease is called and Tick has happened
    bool PreReleaseDone = false;

    //! Set when PreRelease called and waiting for Tick
    //! see PreReleaseDone
    bool PreReleaseWaiting = false;

    // Engine settings //
    bool NoGui = false;
    bool NoLeap = false;
    bool NoSTDInput = false;

    //! \brief Set to true when initialized as a client
    //!
    //! Used to call client specific events
    bool IsClient = false;

    // Marks that the Engine has already done prerelease //
    bool PreReleaseCompleted = false;


    // Invoke store //
    RecursiveMutex InvokeLock;
    std::list<std::function<void()>> InvokeQueue;

    // Stores the command line before running it //
    //! \todo Remove this doesn't work now and needs redoing
    std::vector<std::unique_ptr<std::string>> PassedCommands;

    //! Stores console commands that came from the command line
    std::vector<std::unique_ptr<std::string>> QueuedConsoleCommands;

    //! Queued importers from command line parsing
    std::vector<std::unique_ptr<Editor::Importer>> QueuedImports;

    DLLEXPORT static Engine* instance;
};

} // namespace Leviathan
