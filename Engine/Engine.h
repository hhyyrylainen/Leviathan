// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Networking/CommonNetwork.h"

#include <inttypes.h>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>


namespace Leviathan{

	//! \brief The main class of the Leviathan Game Engine
	//!
	//! Allocates a lot of classes and performs almost all startup operations.
	//! \note Should be thread safe, but might not actually be
	class Engine : public ThreadSafe{
		
		friend GraphicalInputEntity;
		friend Gui::GuiManager;
        friend GameWorld;
    public:
		DLLEXPORT Engine(LeviathanApplication* owner);
		DLLEXPORT ~Engine();

		DLLEXPORT bool Init(AppDef* definition, NETWORKED_TYPE ntype);
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

		DLLEXPORT static void DumpMemoryLeaks();

		DLLEXPORT void Tick();
		DLLEXPORT void RenderFrame();
		DLLEXPORT void PreFirstTick();

		DLLEXPORT int GetWindowOpenCount();

        //! \brief Clears physical timers
        DLLEXPORT void ClearTimers();

		// ------------------------------------ //
		// Passes the commands and preprocesses them, but also interprets commands like --nogui //
		DLLEXPORT void PassCommandLine(const std::string &commands);
        
		// Runs the normal commands passed by the PassCommandLine function //
		DLLEXPORT void ExecuteCommandLine();

        //! \brief Creates a GameWorld for placing entities into
		DLLEXPORT std::shared_ptr<GameWorld> CreateWorld(GraphicalInputEntity* owningwindow,
            std::shared_ptr<ViewerCameraPos> worldscamera);

        //! \brief Releases a GameWorld
        //! \param world The world to destroy.
        //! \post The World will have been released and removed from Engine's internal list and
        //! the world pointer will be NULL
        DLLEXPORT void DestroyWorld(std::shared_ptr<GameWorld> &world);

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
		inline ResourceRefreshHandler* GetResourceRefreshHandler(){
            return _ResourceRefreshHandler; };
        inline EntitySerializer* GetEntitySerializer(){ return _EntitySerializer.get(); };
        inline RemoteConsole* GetRemoteConsole() {
            return _RemoteConsole;
        }
        
#ifdef LEVIATHAN_USES_LEAP
		inline LeapManager* GetLeapManager(){ return LeapData; };
#endif

		inline bool GetNoGui(){ return NoGui; };

		// Static access //
		DLLEXPORT static Engine* GetEngine();
		DLLEXPORT static Engine* Get();

	protected:
		// after load function //
		void PostLoad();

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

        //! Console input comes through this
        bool _ReceiveConsoleInput(const std::string &command);
        
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


		// Stores the command line before running it //
		std::vector<std::unique_ptr<std::string>> PassedCommands;

		static Engine* instance;
	};

}

