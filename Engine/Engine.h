#ifndef LEVIATHAN_ENGINE
#define LEVIATHAN_ENGINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "boost/thread/mutex.hpp"
#include "Common/ThreadSafe.h"
#include "Networking/NetworkInterface.h"
#include "boost/thread.hpp"




namespace Leviathan{

	//! \brief The main class of the Leviathan Game Engine
	//!
	//! Allocates a lot of classes and performs almost all startup operations.
	//! \note Should be thread safe, but might not actually be
	class Engine : public Object, public ThreadSafe{
		
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
		DLLEXPORT bool HasPreReleaseBeenDone() const;

        //! \brief Calculates how long has elapsed since the last tick
        //! \return The time in milliseconds
        DLLEXPORT int GetTimeSinceLastTick() const;

		//! \brief Causes VLD to dump current memory leaks
		DLLEXPORT static void DumpMemoryLeaks();

		DLLEXPORT void Tick();
		DLLEXPORT void RenderFrame();
		DLLEXPORT void PreFirstTick();

		DLLEXPORT int GetWindowOpenCount();

        //! \brief Clears physical timers
        DLLEXPORT void ClearTimers();

        //! \brief Simulates all worlds that are not frozen
        DLLEXPORT void SimulatePhysics();

		// ------------------------------------ //
		// Passes the commands and preprocesses them, but also interprets commands like --nogui //
		DLLEXPORT void PassCommandLine(const wstring &commands);
		// Runs the normal commands passed by the PassCommandLine function //
		DLLEXPORT void ExecuteCommandLine();


        //! \brief Creates a GameWorld for placing entities into
		DLLEXPORT shared_ptr<GameWorld> CreateWorld(GraphicalInputEntity* owningwindow, shared_ptr<ViewerCameraPos>
            worldscamera);

        //! \brief Releases a GameWorld
        //! \param world The world to destroy.
        //! \post The World will have been released and removed from Engine's internal list and the world pointer will
        //! be NULL
        DLLEXPORT void DestroyWorld(shared_ptr<GameWorld> &world);

        //! \brief Opens a new window
        //! \note The window may become broken if the main window is closed
        //! \todo Allow changing the parameters
        DLLEXPORT GraphicalInputEntity* OpenNewWindow();

        //! \brief Returns the main window
		DLLEXPORT GraphicalInputEntity* GetWindowEntity(){ return GraphicalEntity1; };

        //! \brief Removes an closed window from the engine
        DLLEXPORT void ReportClosedWindow(GraphicalInputEntity* windowentity);
        
		DLLEXPORT void SaveScreenShot();

		DLLEXPORT Graphics* GetGraphics(){ return Graph; };
		DLLEXPORT EventHandler* GetEventHandler(){ return MainEvents; };
		DLLEXPORT ObjectLoader* GetObjectLoader(){return Loader;};
		DLLEXPORT RenderingStatistics* GetRenderingStatistics(){ return RenderTimer;};
		DLLEXPORT ScriptConsole* GetScriptConsole(){ return MainConsole;};
		DLLEXPORT FileSystem* GetFileSystem(){ return MainFileHandler; };
		DLLEXPORT AppDef* GetDefinition(){ return Define;};
		DLLEXPORT NewtonManager* GetNewtonManager(){ return _NewtonManager; };
		DLLEXPORT LeviathanApplication* GetOwningApplication(){ return Owner; };
		DLLEXPORT PhysicsMaterialManager* GetPhysicalMaterialManager(){ return PhysMaterials; };
		DLLEXPORT NetworkHandler* GetNetworkHandler(){ return _NetworkHandler; };
		DLLEXPORT ThreadingManager* GetThreadingManager(){ return _ThreadingManager; };
		DLLEXPORT ResourceRefreshHandler* GetResourceRefreshHandler(){ return _ResourceRefreshHandler; };
        DLLEXPORT EntitySerializerManager* GetEntitySerializerManager(){ return _EntitySerializerManager; };
        DLLEXPORT ConstraintSerializerManager* GetConstraintSerializerManager(){ return _ConstraintSerializerManager; };
        DLLEXPORT AINetworkCache* GetAINetworkCache(){ return _AINetworkCache; };

#ifdef LEVIATHAN_USES_LEAP
		DLLEXPORT LeapManager* GetLeapManager(){ return LeapData; };
#endif

		DLLEXPORT bool GetNoGui(){ return NoGui; };

		// Static access //
		DLLEXPORT static Engine* GetEngine();
		DLLEXPORT static Engine* Get();

		// For NoGui mode //
#ifdef _WIN32
		DLLEXPORT static void WinAllocateConsole();
#endif

	private:
		// after load function //
		void PostLoad();

		//! Function called by first instance of Window class after creating a window to not error
        //! when registering threads to work with Ogre
		void _NotifyThreadsRegisterOgre();

        //! \brief Sets the tick clock to a certain value
        //! \note Should only be used to match the server's clock
        //! \param amount The amount of time in milliseconds to set or change
        //! \param absolute When true sets the time until a tick to amount otherwise changes the remaining
        //! time by amount
        void _AdjustTickClock(int amount, bool absolute = true);

		// ------------------------------------ //
		AppDef* Define;

		RenderingStatistics* RenderTimer;
		Graphics* Graph;
        
		GraphicalInputEntity* GraphicalEntity1;
        std::vector<GraphicalInputEntity*> AdditionalGraphicalEntities;

		SoundDevice* Sound;
		DataStore* Mainstore;
		EventHandler* MainEvents;
		ScriptInterface* MainScript;
		ObjectLoader* Loader;
		ScriptConsole* MainConsole;
		FileSystem* MainFileHandler;
		Random* MainRandom;
		OutOfMemoryHandler* OutOMemory;
		NewtonManager* _NewtonManager;
		PhysicsMaterialManager* PhysMaterials;
		NetworkHandler* _NetworkHandler;
		ThreadingManager* _ThreadingManager;
		RemoteConsole* _RemoteConsole;
		ResourceRefreshHandler* _ResourceRefreshHandler;
        EntitySerializerManager* _EntitySerializerManager;
        ConstraintSerializerManager* _ConstraintSerializerManager;
        AINetworkCache* _AINetworkCache;

#ifdef LEVIATHAN_USES_LEAP
		LeapManager* LeapData;
#endif


		IDFactory* IDDefaultInstance;
		LeviathanApplication* Owner;
        
		//! List of current worlds
		std::vector<shared_ptr<GameWorld>> GameWorlds;

        //! Mutex that is locked when changing the worlds
        boost::mutex GameWorldsLock;

        //! Mutex that is locked while NetworkHandler is used
        boost::mutex NetworkHandlerLock;

		// data //
		__int64 LastFrame;
		int TimePassed;
		int FrameLimit;
		int TickCount;
		int TickTime;
		int FrameCount;

		// flags //
		bool MouseCaptured : 1;
		bool WantsToCapture : 1;
		bool Focused : 1;
		bool GuiActive : 1;
		bool Inited : 1;
		//! Set when PreRelease is called and Tick has happened
		bool PreReleaseDone : 1;
		//! Set when PreRelease called and waiting for Tick
		//! see PreReleaseDone
		bool PreReleaseWaiting : 1;
		bool NoGui;
		bool NoLeap;

		// Marks that the Engine has already done prerelease //
		bool PreReleaseCompleted;


		// Stores the command line before running it //
		std::vector<unique_ptr<wstring>> PassedCommands;

		// NoGui input handler //
		boost::thread CinThread;

		static Engine* instance;
	};

}
#endif
