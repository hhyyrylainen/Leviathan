#ifndef LEVIATHAN_APPLICATION
#define LEVIATHAN_APPLICATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#ifndef LEVIATHAN_ENGINE
#include "Engine.h"
#endif
#ifndef LEVIATHAN_WINDOW
#include "Window.h"
#endif
#include "AppDefine.h"
namespace Leviathan{

	class LeviathanApplication : public Object{
	public:
		DLLEXPORT LeviathanApplication();

		DLLEXPORT virtual bool Initialize(HINSTANCE hinstance);
		//DLLEXPORT virtual bool Initialize(HINSTANCE hinstance, HWND hwnd, /*int width, int height,*/ bool windowed);
		DLLEXPORT virtual bool Initialize(HINSTANCE hinstance,  WNDPROC proc, wstring tittle, /*int width, int height,*/ HICON hIcon, bool windowed);

		DLLEXPORT virtual void Close();
		DLLEXPORT virtual void Render();
		DLLEXPORT virtual void OnResize(UINT width, UINT height);
		DLLEXPORT virtual void ResizeWindow(int newwidth, int newheight);
		DLLEXPORT virtual int RunMessageLoop();
		//static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,LPARAM lParam);

		DLLEXPORT Engine* GetEngine(){ return engine;};

		DLLEXPORT void PostQuit(){ Quit = true; };
		DLLEXPORT bool Quitting(){ return Quit; };
		DLLEXPORT bool HasQuit(){ return Quitted;};

		DLLEXPORT void PassCommandLine(wstring params);

		DLLEXPORT HINSTANCE GetHinstance(){ return hInstance;};

		DLLEXPORT static LeviathanApplication* GetApp();
		DLLEXPORT static AppDef* GetAppDef();

		DLLEXPORT virtual void LoseFocus();
		DLLEXPORT virtual void GainFocus();

	protected:
		void ConstructDefinition();
		void InternalInit();
		static LeviathanApplication* Curapp;
		bool Windowed;
		bool Quitted;
		bool Quit;

		Engine* engine;
		Window* m_wind;
		HINSTANCE hInstance;
		AppDef* Defvals;

		// static part //
	};


}
#endif