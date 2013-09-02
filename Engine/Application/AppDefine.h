#ifndef LEVIATHAN_APPLICATIONDEFINE
#define LEVIATHAN_APPLICATIONDEFINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common\DataStoring\NamedVars.h"
#include "Common\Window.h"
#include "ObjectFiles\ObjectFileProcessor.h"

namespace Leviathan{

	class Engine;
	class LeviathanApplication;

	struct WindowDataDetails{
		WindowDataDetails();
		WindowDataDetails(const wstring &title, const int &width, const int &height, const bool &windowed, const bool &windowborder, HICON icon, 
			LeviathanApplication* appvirtualptr);

		void ApplyIconToHandle(HWND hwnd) const;

		wstring Title;
		HICON Icon;
		int Height;
		int Width;
		bool Windowed;
	};


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
		DLLEXPORT AppDef& SetWindowDetails(const WindowDataDetails &det){

			WDetails = det;
			return *this;
		}


		DLLEXPORT WindowDataDetails& GetWindowDetails(){

			return WDetails;
		}
		DLLEXPORT inline static AppDef* GetDefault(){
			return Defaultconf;
		}
		DLLEXPORT inline bool GetVSync(){
			
			bool vsync;

			ObjectFileProcessor::LoadValueFromNamedVars(*ConfigurationValues, L"Vsync", vsync, false, false);
			return vsync;
		}
		DLLEXPORT Window* GetWindow(){

			return RWindow;
		}

		DLLEXPORT static AppDef* GenerateAppdefine();
		DLLEXPORT void StoreWindowDetails(const wstring &title, const bool &windowborder, HICON icon, LeviathanApplication* appvirtualptr);

	protected:
		
		unique_ptr<NamedVars> ConfigurationValues;
		HINSTANCE HInstance;

		Window* RWindow;

		// details used to create a window //
		WindowDataDetails WDetails;


		


		// ------------------------------------ //
		static AppDef* Defaultconf;
	};
















}
#endif