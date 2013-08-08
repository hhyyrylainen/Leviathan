#ifndef LEVIATHAN_APPLICATIONDEFINE
#define LEVIATHAN_APPLICATIONDEFINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NamedVars.h"
#include "Window.h"

namespace Leviathan{

	class Engine;
	class LeviathanApplication;

	class AppDef{
		friend Engine;
	public:
		DLLEXPORT AppDef(const bool &isdef = false);
		DLLEXPORT AppDef::~AppDef();


		DLLEXPORT NamedVars* GetValues();

		// named constructor functions //
		DLLEXPORT AppDef& SetRenderingWindow(Window* wind){

			RWindow = wind;
			return *this;
		}

		DLLEXPORT AppDef& SetHInstance(HINSTANCE instance){

			HInstance = instance;
			return *this;
		}



		DLLEXPORT inline static AppDef* GetDefault(){
			return Defaultconf;
		}

		DLLEXPORT static AppDef* GenerateAppdefine();
		DLLEXPORT Window* CreateRenderingWindow(const wstring &title, const bool &windowborder, HICON icon, WNDPROC wndproc, 
			LeviathanApplication* appvirtualptr);

	protected:
		
		unique_ptr<NamedVars> ConfigurationValues;
		HINSTANCE HInstance;
		Window* RWindow;


		// ------------------------------------ //
		static AppDef* Defaultconf;
	};
















}
#endif