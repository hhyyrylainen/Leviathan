#ifndef LEVIATHAN_APPLICATIONDEFINE
#define LEVIATHAN_APPLICATIONDEFINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/DataStoring/NamedVars.h"
#include "Common/Window.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Networking\NetworkClient.h"

namespace Leviathan{

	struct WindowDataDetails{
		WindowDataDetails();
#ifdef _WIN32
		WindowDataDetails(const wstring &title, const int &width, const int &height, const bool &windowed, const bool &windowborder, HICON icon,
			LeviathanApplication* appvirtualptr);

		void ApplyIconToHandle(HWND hwnd) const;


		HICON Icon;
#else
		WindowDataDetails(const wstring &title, const int &width, const int &height, const bool &windowed, const bool &windowborder,
			LeviathanApplication* appvirtualptr);

#endif
		wstring Title;
		int Height;
		int Width;
		bool Windowed;
	};


	class AppDef{
		friend Engine;
	public:
		DLLEXPORT AppDef(const bool &isdef = false);
		DLLEXPORT ~AppDef();


		DLLEXPORT NamedVars* GetValues();

		// named constructor functions //
#ifdef _WIN32
		DLLEXPORT AppDef& SetHInstance(HINSTANCE instance){

			HInstance = instance;
			return *this;
		}
#else
		// TODO: linux equivalent
#endif
		DLLEXPORT AppDef& SetWindowDetails(const WindowDataDetails &det){

			WDetails = det;
			return *this;
		}

		DLLEXPORT AppDef& SetMasterServerParameters(const MasterServerInformation &info){

			MasterServerInfo = info;
			return *this;
		}


		DLLEXPORT WindowDataDetails& GetWindowDetails(){

			return WDetails;
		}
		DLLEXPORT MasterServerInformation& GetMasterServerInfo(){

			return MasterServerInfo;
		}
		DLLEXPORT inline static AppDef* GetDefault(){
			return Defaultconf;
		}
		DLLEXPORT inline bool GetVSync(){

			bool vsync;

			ObjectFileProcessor::LoadValueFromNamedVars<bool>(*ConfigurationValues, L"Vsync", vsync, false, false);
			return vsync;
		}



		DLLEXPORT static AppDef* GenerateAppdefine();
#ifdef _WIN32
		DLLEXPORT void StoreWindowDetails(const wstring &title, const bool &windowborder, HICON icon, LeviathanApplication* appvirtualptr);
#else
		DLLEXPORT void StoreWindowDetails(const wstring &title, const bool &windowborder, LeviathanApplication* appvirtualptr);
#endif

	protected:

		unique_ptr<NamedVars> ConfigurationValues;
#ifdef _WIN32
		HINSTANCE HInstance;
#else
		int HInstance;
#endif
		MasterServerInformation MasterServerInfo;

		// details used to create a window //
		WindowDataDetails WDetails;

		// ------------------------------------ //
		static AppDef* Defaultconf;
	};
}

#endif
