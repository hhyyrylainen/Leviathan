#ifndef LEVIATHAN_APPLICATIONDEFINE
#define LEVIATHAN_APPLICATIONDEFINE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/DataStoring/NamedVars.h"
#include "Window.h"
#include "Networking/NetworkHandler.h"

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

		DLLEXPORT HINSTANCE GetHInstance(){

			return HInstance;
		}
#else
		// \todo linux equivalent
#endif
		DLLEXPORT AppDef& SetWindowDetails(const WindowDataDetails &det){

			WDetails = det;
			return *this;
		}
		DLLEXPORT AppDef& SetPacketHandler(NetworkInterface* networkhandler){

			_NetworkInterface = networkhandler;
			return *this;
		}

		DLLEXPORT AppDef& SetMasterServerParameters(const MasterServerInformation &info){

			MasterServerInfo = info;
			return *this;
		}
		// Sets the version information of the application, leviathan version is set automatically //
		DLLEXPORT AppDef& SetApplicationIdentification(const wstring &userreadable, const wstring &gamename, const wstring &gameversion);

		DLLEXPORT WindowDataDetails& GetWindowDetails(){

			return WDetails;
		}
		DLLEXPORT MasterServerInformation& GetMasterServerInfo(){

			return MasterServerInfo;
		}
		DLLEXPORT inline static AppDef* GetDefault(){
			return Defaultconf;
		}
		DLLEXPORT bool GetVSync();

		DLLEXPORT const wstring& GetLogFile(){
			return LogFile;
		}


		DLLEXPORT NetworkInterface* GetPacketHandler(){
			return _NetworkInterface;
		}

		DLLEXPORT static AppDef* GenerateAppdefine(const wstring &logfile, const wstring &engineconfigfile, const wstring &gameconfig, const wstring &keyconfig, 
			boost::function<void (GameConfiguration* configobj)> configchecker, boost::function<void (KeyConfiguration* keysobject)> keychecker);
#ifdef _WIN32
		DLLEXPORT void StoreWindowDetails(const wstring &title, const bool &windowborder, HICON icon, LeviathanApplication* appvirtualptr);
#else
		DLLEXPORT void StoreWindowDetails(const wstring &title, const bool &windowborder, LeviathanApplication* appvirtualptr);
#endif


		DLLEXPORT void GetGameIdentificationData(wstring &userreadable, wstring &gamename, wstring &gameversion);

	protected:

		unique_ptr<NamedVars> ConfigurationValues;
#ifdef _WIN32
		HINSTANCE HInstance;
#else
		int HInstance;
#endif
		MasterServerInformation MasterServerInfo;

		NetworkInterface* _NetworkInterface;

		// details used to create a window //
		WindowDataDetails WDetails;

		// Game variables //
		GameConfiguration* _GameConfiguration;
		KeyConfiguration* _KeyConfiguration;

		wstring LogFile;
		Logger* Mainlog;


		wstring LeviathanVersion;
		wstring GameVersion;
		wstring Game;
		wstring UserReadableGame;

		// ------------------------------------ //
		static AppDef* Defaultconf;
	};
}

#endif
