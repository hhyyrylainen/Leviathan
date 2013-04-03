#ifndef LEVIATHAN_ENGINE
#define LEVIATHAN_ENGINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


#define TICKSPEED 60

#include "Random.h"

#include "DataStore.h"
#include "AppDefine.h"
#include "ObjectManager.h"
#include "ObjectLoader.h"
#include "GeometryAdvancedLoader.h"
#include "ResolutionScaling.h"

#include "GuiManager.h"
#include "EventHandler.h"
#include "KeyPressManager.h"

#include "ScriptInterface.h"

#include "Window.h"
#include ".\Rendering\Graphics.h"

#include "Timer.h"
#include "Input.h"
#include "SoundDevice.h"
#include "CameraPos.h"

#include "BasePositionable.h"
#include "ObjectFileProcessor.h"

// monitoring //
//#include "CpuMonitor.h"
//#include "FpsCounter.h"
#include "RenderingStatistics.h"

namespace Leviathan{
	class Engine : public Object{
	public:
		DLLEXPORT Engine::Engine();
		DLLEXPORT bool InitEngine(Window* wind, bool windowed, AppDef* def);
		DLLEXPORT bool ShutDownEngine();

		//DLLEXPORT bool ConfigureRenderingSetup();

		DLLEXPORT void Tick(bool Force);

		DLLEXPORT void UpdateFrameScene();
		DLLEXPORT void RenderFrame();
		//DLLEXPORT HRESULT StartFrame();

		DLLEXPORT bool HandleWindowCallBack(UINT message, WPARAM wParam,LPARAM lParam);
		DLLEXPORT bool DoWindowResize(int width, int height);
		DLLEXPORT void OnResize(int width, int height);

		DLLEXPORT static Engine* GetEngine();

		DLLEXPORT void CaptureMouse(bool toset);

		DLLEXPORT void SetGuiActive(bool toset);

		DLLEXPORT void LoseFocus();
		DLLEXPORT void GainFocus();



		// Object handling //
		DLLEXPORT void AddObject(BaseObject* obj);
		DLLEXPORT const shared_ptr<BaseObject>& GetObjectByID(int id);
		DLLEXPORT int GetIndex(int id);
		DLLEXPORT const shared_ptr<BaseObject>& GetObjectByIndex(int index);

		DLLEXPORT void RemoveObject(int id);
		DLLEXPORT bool RemoveObjectByIndex(int index);

		DLLEXPORT int GetObjectCount();

		// ----------------- //

		DLLEXPORT void ExecuteCommandLine(wstring commands);
		DLLEXPORT void RunScrCommand(wstring command, wstring params);


		DLLEXPORT RenderingLight* GetLightAtObject(BasePositionable* obj);

		Timer* MTimer;

		DLLEXPORT Window* GetWindow(){ return Wind; };
		DLLEXPORT GuiManager* GetGui(){ return Gui; };
		DLLEXPORT Graphics* GetGraphics(){ return Graph; };
		DLLEXPORT EventHandler* GetEventHandler(){ return MainEvents; };
		DLLEXPORT ObjectManager* GetObjectManager(){ return GObjects; };
		DLLEXPORT ObjectLoader* GetObjectLoader(){return Loader;};
		DLLEXPORT RenderingStatistics* GetRenderingStatistics(){ return RenderTimer;};
		DLLEXPORT GeometryAdvancedLoader* GetAdvancedGeometryHandler(){ return AdvancedGeometryFiles; };

	private:
		void PostLoad();

		static Engine* instance;

		Logger* Mainlog;
		bool Inited;
		Window* Wind;
		Graphics* Graph;
		AppDef* Define;
		DxRendConf dxconf;

		SoundDevice* Sound;

		Input* Inputs;

		__int64 LastFrame;
		int TimePassed;
		//__int64 RenderStart;
		//int SinceRender;

		int FrameLimit;

		int TickCount;
		int TickTime;
		//int FrameTime;

		int FrameCount;

		//FpsCounter* FpsMonitor;
		//CpuMonitor* CpuUsage;

		DataStore* Mainstore;
		EventHandler* MainEvents;
		ScriptInterface* MainScript;
		ObjectManager* GObjects;
		ObjectLoader* Loader;
		Random* MainRandom;
		RenderingStatistics* RenderTimer;
		GeometryAdvancedLoader* AdvancedGeometryFiles;

		bool MouseCaptured;
		bool WantsToCapture;
		bool Focused;
		bool GuiActive;


		ViewerCameraPos* MainCamera;
		KeyPressManager* KeyListener;

		GuiManager* Gui;
	};












}
#endif