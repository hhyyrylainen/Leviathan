#ifndef LEVIATHAN_ENGINE
#define LEVIATHAN_ENGINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application\AppDefine.h"
#include "Entities\Bases\BaseObject.h"
#include "Statistics\Timer.h"
#include "Common\Window.h"
#include "GUI\GuiManager.h"
#include "Rendering\Graphics.h"
#include "Events\EventHandler.h"
#include "Handlers\ObjectManager.h"
#include "Handlers\ObjectLoader.h"
#include "Statistics\RenderingStatistics.h"
#include "Handlers\AnimationManager.h"
#include "Input\KeyPressManager.h"
#include "Leap\LeapManager.h"
#include "Script\Console.h"
#include "FileSystem.h"
#include "Sound\SoundDevice.h"
#include "Common\DataStoring\DataStore.h"
#include "Entities\ViewerCameraPos.h"
#include "Handlers\OutOfMemoryHandler.h"
#include "Utility\Random.h"



#define TICKSPEED 60



namespace Leviathan{
	class Engine : public Object{
		// friend so that window can update size //
		friend Window;
		friend Gui::GuiManager;
	public:
		DLLEXPORT Engine();

		DLLEXPORT bool Init(AppDef* definition);
		DLLEXPORT void Release();

		DLLEXPORT void Tick();
		DLLEXPORT void RenderFrame();


		DLLEXPORT bool DoWindowResize(int width, int height);


		// Object handling //
		DLLEXPORT void AddEntityObject(BaseObject* obj);
		DLLEXPORT const shared_ptr<BaseObject>& GetObjectByID(const int &id) const;
		DLLEXPORT const shared_ptr<BaseObject>& GetObjectByIndex(const size_t &index) const;
		DLLEXPORT size_t GetIndexFromID(const int &id);
		DLLEXPORT void RemoveObject(const int &id);
		DLLEXPORT bool RemoveObjectByIndex(const size_t &index);
		DLLEXPORT int GetObjectCount();

		// ------------------------------------ //
		DLLEXPORT void ExecuteCommandLine(const wstring &commands);
		DLLEXPORT void RunScrCommand(wstring command, wstring params);


		DLLEXPORT void SaveScreenShot();
		
		DLLEXPORT Gui::GuiManager* GetGui(){ return GManager; };
		DLLEXPORT Graphics* GetGraphics(){ return Graph; };
		DLLEXPORT EventHandler* GetEventHandler(){ return MainEvents; };
		DLLEXPORT ObjectManager* GetObjectManager(){ return GObjects; };
		DLLEXPORT ObjectLoader* GetObjectLoader(){return Loader;};
		DLLEXPORT RenderingStatistics* GetRenderingStatistics(){ return RenderTimer;};
		DLLEXPORT AnimationManager* GetAnimationManager(){ return AnimManager; };
		DLLEXPORT KeyPressManager* GetKeyPresManager(){ return KeyListener; };
		DLLEXPORT LeapManager* GetLeapManager(){ return LeapData;};
		DLLEXPORT ScriptConsole* GetScriptConsole(){ return MainConsole;};
		DLLEXPORT FileSystem* GetFileSystem(){ return MainFileHandler; };
		DLLEXPORT AppDef* GetDefinition(){ return Define;};
		// static access //
		DLLEXPORT static Engine* GetEngine();

	private:
		// after load function //
		void PostLoad();

		// private so that only Window objects can update this //
		void OnResize(int width, int height);
		void LoseFocus();
		void GainFocus();
		// for gui manager to update //
		void SetGuiActive(bool toset);
		// to be called when gui is set non active //
		void CaptureMouse(bool toset);
		// ------------------------------------ //
		Logger* Mainlog;
		Timer* MTimer;
		AppDef* Define;

		Gui::GuiManager* GManager;
		RenderingStatistics* RenderTimer;
		Graphics* Graph;


		SoundDevice* Sound;
		Input* Inputs;
		KeyPressManager* KeyListener;
		DataStore* Mainstore;
		EventHandler* MainEvents;
		ScriptInterface* MainScript;
		ObjectManager* GObjects;
		ObjectLoader* Loader;
		LeapManager* LeapData;
		ScriptConsole* MainConsole;
		FileSystem* MainFileHandler;
		Random* MainRandom;
		AnimationManager* AnimManager;
		OutOfMemoryHandler* OutOMemory;
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

		// this might have to be moved to some other place //
		ViewerCameraPos* MainCamera;


		static Engine* instance;
	};

}
#endif