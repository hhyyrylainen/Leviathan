// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Networking/CommonNetwork.h"

#include <inttypes.h>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>
#include <functional>
#include <list>


namespace Leviathan{

class LeviathanApplication;

namespace GUI{
class AlphaHitCache;
}

//! \brief The main class of the Leviathan Game Engine
//!
//! Allocates a lot of classes and performs almost all startup operations.
//! \note Should be thread safe, but might not actually be
class Engine : public ThreadSafe{
        
    friend GraphicalInputEntity;
    friend GUI::GuiManager;
    friend GameWorld;
    friend LeviathanApplication;
public:
    DLLEXPORT Engine(LeviathanApplication* owner);
    DLLEXPORT ~Engine();

    DLLEXPORT bool Init(AppDef* definition, NETWORKED_TYPE ntype,
        NetworkInterface* packethandler);
    
    //! \todo Add a thread that monitors if the thing gets stuck on a task
    DLLEXPORT void Release(bool forced = false);

    //! \brief Sets objects ready to be released
    //! \note The Tick function must be called after this but before Release
    DLLEXPORT void PreRelease();

    //! \brief Checks if PreRelease is done and Release can be called
    //! \pre PreRelease is called
    DLLEXPORT inline bool HasPreReleaseBeenDone() const{
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
    DLLEXPORT void Invoke(const std::function<void()> &function);

    //! \brief Runs the function now if on the main thread otherwise calls Invoke
    DLLEXPORT void RunOnMainThread(const std::function<void()> &function);

    //! \brief Returns true if called on the main thread
    DLLEXPORT bool IsOnMainThread() const{LEVIATHAN_ASSERT(false, "TODO");}

    //! \brief Asserts if not called on the main thread
    DLLEXPORT inline void AssertIfNotMainThread() const;
    

    // ------------------------------------ //
    //! Passes the commands and preprocesses them
    //!
    //! Also interprets commands like --nogui
    //! \returns False if invalid command line and the game should quit instantly
    DLLEXPORT bool PassCommandLine(int argcount, char* args[]);
        
    //! \brief Creates a GameWorld for placing entities into
    //! \note To actually move the world camera you need to use
    //! Leviathan::ObjectLoader::LoadCamera to create a camera entity
    DLLEXPORT std::shared_ptr<GameWorld> CreateWorld(GraphicalInputEntity* owningwindow);

    //! \brief Releases a GameWorld
    //! \param world The world to destroy.
    //! \post The World will have been released and removed from Engine's internal list and
    //! when all other holders of the pointer release it will be deleted
    DLLEXPORT void DestroyWorld(const std::shared_ptr<GameWorld> &world);

    //! \brief Opens a new window
    //! \note The window may become broken if the main window is closed
    //! \todo Allow changing the parameters
    DLLEXPORT GraphicalInputEntity* OpenNewWindow();

    //! \brief Returns the main window
    DLLEXPORT GraphicalInputEntity* GetWindowEntity(){ return GraphicalEntity1; };

    //! \brief Removes an closed window from the engine
    DLLEXPORT void ReportClosedWindow(Lock &guard, GraphicalInputEntity* windowentity);

    DLLEXPORT inline void ReportClosedWindow(GraphicalInputEntity* windowentity){

        GUARD_LOCK();
        ReportClosedWindow(guard, windowentity);
    }
        
    DLLEXPORT void SaveScreenShot();

    inline Graphics* GetGraphics(){ return Graph; };
    inline EventHandler* GetEventHandler(){ return MainEvents; };
    inline RenderingStatistics* GetRenderingStatistics(){ return RenderTimer;};
    inline ScriptConsole* GetScriptConsole(){ return MainConsole;};
    inline FileSystem* GetFileSystem(){ return MainFileHandler; };
    inline AppDef* GetDefinition(){ return Define;};
    inline NewtonManager* GetNewtonManager(){ return _NewtonManager; };
    inline LeviathanApplication* GetOwningApplication(){ return Owner; };
    inline PhysicsMaterialManager* GetPhysicalMaterialManager(){ return PhysMaterials; };
    inline NetworkHandler* GetNetworkHandler(){ return _NetworkHandler; };
    inline ThreadingManager* GetThreadingManager(){ return _ThreadingManager; };
    inline SoundDevice* GetSoundDevice(){ return Sound; };
    inline ResourceRefreshHandler* GetResourceRefreshHandler(){
        return _ResourceRefreshHandler; };
    inline EntitySerializer* GetEntitySerializer(){ return _EntitySerializer.get(); };
    inline RemoteConsole* GetRemoteConsole() {
        return _RemoteConsole;
    }
    inline GUI::AlphaHitCache* GetAlphaHitCache(){
        return _AlphaHitCache.get();
    }
    inline Random* GetRandom(){ return MainRandom; }

    DLLEXPORT GraphicalInputEntity* GetWindowFromSDLID(uint32_t sdlid);
    
#ifdef LEVIATHAN_USES_LEAP
    inline LeapManager* GetLeapManager(){ return LeapData; };
#endif

    inline bool GetNoGui(){ return NoGui; };

    // Command line settings can only be set before initializing //
    inline void SetNoGUI(){

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
    bool _ReceiveConsoleInput(const std::string &command);

    //! Runs all commands in QueuedConsoleCommands
    void _RunQueuedConsoleCommands();

    //! Helper for PassCommandLine
    bool ParseSingleCommand(StringIterator &itr, int &argindex, const int argcount,
        char* args[]);
        
    // ------------------------------------ //
    AppDef* Define = nullptr;

    RenderingStatistics* RenderTimer = nullptr;
    Graphics* Graph = nullptr;
        
    GraphicalInputEntity* GraphicalEntity1 = nullptr;
    std::vector<GraphicalInputEntity*> AdditionalGraphicalEntities;

    SoundDevice* Sound = nullptr;
    DataStore* Mainstore = nullptr;
    EventHandler* MainEvents = nullptr;
    ScriptExecutor* MainScript = nullptr;
    ScriptConsole* MainConsole = nullptr;
    FileSystem* MainFileHandler = nullptr;
    Random* MainRandom = nullptr;
    OutOfMemoryHandler* OutOMemory = nullptr;
    NewtonManager* _NewtonManager = nullptr;
    PhysicsMaterialManager* PhysMaterials = nullptr;
    NetworkHandler* _NetworkHandler = nullptr;
    ThreadingManager* _ThreadingManager = nullptr;
    RemoteConsole* _RemoteConsole = nullptr;
    ResourceRefreshHandler* _ResourceRefreshHandler = nullptr;
        
    std::unique_ptr<ConsoleInput> _ConsoleInput;
    std::unique_ptr<EntitySerializer> _EntitySerializer;
    std::unique_ptr<GUI::AlphaHitCache> _AlphaHitCache;

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
    std::vector<std::unique_ptr<std::string>> PassedCommands;

    //! Stores console commands that came from the command line
    std::vector<std::unique_ptr<std::string>> QueuedConsoleCommands;

    DLLEXPORT static Engine* instance;
};

}

