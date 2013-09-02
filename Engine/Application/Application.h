#ifndef LEVIATHAN_APPLICATION
#define LEVIATHAN_APPLICATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Engine.h"
#include "Application\AppDefine.h"

namespace Leviathan{

	class LeviathanApplication : public Object{
	public:
		// creation and initialization //
		DLLEXPORT LeviathanApplication();
		DLLEXPORT virtual ~LeviathanApplication();

		DLLEXPORT virtual bool Initialize(AppDef* configuration);
		DLLEXPORT virtual void Release();
		// sets should quit to true so that the main thread will stop everything and close //
		DLLEXPORT virtual void StartRelease();
		
		// functions called when certain events happen //
		DLLEXPORT virtual void OnLoseFocus();
		DLLEXPORT virtual void OnGainFocus();
		DLLEXPORT virtual void OnResize(const int &width, const int &height);


		// perform actions //
		DLLEXPORT virtual void DoResizeWindow(const int &newwidth, const int &newheight);
		DLLEXPORT virtual int RunMessageLoop();
		DLLEXPORT virtual void Render();
		DLLEXPORT void PassCommandLine(const wstring &params);
		
		// getting data from the class //
		DLLEXPORT bool Quitting(){ return Quit; };
		DLLEXPORT Engine* GetEngine(){ return engine;};
		DLLEXPORT AppDef* GetDefinition(){ return ApplicationConfiguration;};

		// static access method for getting instance of this class //
		DLLEXPORT static inline LeviathanApplication* GetApp(){
			return Curapp;
		}
	protected:
		// called just before returning from initialization, and can be used setting start time etc. //
		DLLEXPORT virtual void _InternalInit();
		// ------------------------------------ //
		bool Quit;
		bool ShouldQuit;

		Engine* engine;
		AppDef* ApplicationConfiguration;

		// static part //
		static LeviathanApplication* Curapp;
	};


}
#endif