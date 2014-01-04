#ifndef LEVIATHAN_ENGINE
#define LEVIATHAN_ENGINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application/AppDefine.h"
#include "Rendering/Graphics.h"
#include "Events/EventHandler.h"
#include "Handlers/ObjectLoader.h"
#include "Statistics/RenderingStatistics.h"
#include "Leap/LeapManager.h"
#include "Script/Console.h"
#include "FileSystem.h"
#include "Sound/SoundDevice.h"
#include "Common/DataStoring/DataStore.h"
#include "Handlers/OutOfMemoryHandler.h"
#include "Utility/Random.h"
#include "Common/GraphicalInputEntity.h"
#include "Newton/NewtonManager.h"
#include "Newton/PhysicalMaterialManager.h"
#include "Networking/NetworkHandler.h"
#include "Threading/ThreadingManager.h"


#define TICKSPEED 60



namespace Leviathan{

	// for storing pointer to owner //
	class LeviathanApplication;

	class Engine : public Object, public ThreadSafe{
		// friend so that window can update size //
		friend Window;
		friend GraphicalInputEntity;
		friend Gui::GuiManager;
	public:
		DLLEXPORT Engine(LeviathanApplication* owner);

		DLLEXPORT bool Init(AppDef* definition, NETWORKED_TYPE ntype);
		DLLEXPORT void Release();

		DLLEXPORT void Tick();
		DLLEXPORT void RenderFrame();
		DLLEXPORT void PhysicsUpdate();
		DLLEXPORT void ResetPhysicsTime();
		DLLEXPORT void PreFirstTick();

		DLLEXPORT int GetWindowOpenCount();

		// ------------------------------------ //
		// Passes the commands and preprocesses them, but also interprets commands like --nogui //
		DLLEXPORT void PassCommandLine(const wstring &commands);
		// Runs the normal commands passed by the PassCommandLine function //
		DLLEXPORT void ExecuteCommandLine();


		DLLEXPORT shared_ptr<GameWorld> CreateWorld();

		DLLEXPORT void SaveScreenShot();

		DLLEXPORT Graphics* GetGraphics(){ return Graph; };
		DLLEXPORT EventHandler* GetEventHandler(){ return MainEvents; };
		DLLEXPORT ObjectLoader* GetObjectLoader(){return Loader;};
		DLLEXPORT RenderingStatistics* GetRenderingStatistics(){ return RenderTimer;};
		DLLEXPORT LeapManager* GetLeapManager(){ return LeapData;};
		DLLEXPORT ScriptConsole* GetScriptConsole(){ return MainConsole;};
		DLLEXPORT FileSystem* GetFileSystem(){ return MainFileHandler; };
		DLLEXPORT AppDef* GetDefinition(){ return Define;};
		DLLEXPORT GraphicalInputEntity* GetWindowEntity(){ return GraphicalEntity1; };
		DLLEXPORT NewtonManager* GetNewtonManager(){ return _NewtonManager; };
		DLLEXPORT LeviathanApplication* GetOwningApplication(){ return Owner; };
		DLLEXPORT PhysicsMaterialManager* GetPhysicalMaterialManager(){ return PhysMaterials; };
		DLLEXPORT NetworkHandler* GetNetworkHandler(){ return _NetworkHandler; };
		DLLEXPORT ThreadingManager* GetThreadingManager(){ return _ThreadingManager; };
		DLLEXPORT bool GetNoGui(){ return NoGui; };
		// static access //
		DLLEXPORT static Engine* GetEngine();
		DLLEXPORT static Engine* Get();

		// For NoGui mode //
#ifdef _WIN32
		DLLEXPORT static void WinAllocateConsole();
#endif

	private:
		// after load function //
		void PostLoad();

		// Function called by first instance of Window class after creating a window to not error when registering threads to work with Ogre //
		void _NotifyThreadsRegisterOgre();

		// ------------------------------------ //
		AppDef* Define;

		RenderingStatistics* RenderTimer;
		Graphics* Graph;
		GraphicalInputEntity* GraphicalEntity1;

		SoundDevice* Sound;
		DataStore* Mainstore;
		EventHandler* MainEvents;
		ScriptInterface* MainScript;
		ObjectLoader* Loader;
		LeapManager* LeapData;
		ScriptConsole* MainConsole;
		FileSystem* MainFileHandler;
		Random* MainRandom;
		OutOfMemoryHandler* OutOMemory;
		NewtonManager* _NewtonManager;
		PhysicsMaterialManager* PhysMaterials;
		NetworkHandler* _NetworkHandler;
		ThreadingManager* _ThreadingManager;

		IDFactory* IDDefaultInstance;
		LeviathanApplication* Owner;
		// world data //
		std::vector<shared_ptr<GameWorld>> GameWorlds;

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
		bool NoGui;

		// Stores the command line before running it //
		std::vector<unique_ptr<wstring>> PassedCommands;

		// NoGui input handler //
		boost::thread CinThread;

		static Engine* instance;
	};

}
#endif
