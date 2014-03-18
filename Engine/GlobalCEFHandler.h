#ifndef LEVIATHAN_GLOBALCEFHANDLER
#define LEVIATHAN_GLOBALCEFHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "include/internal/cef_ptr.h"
#include "include/cef_task.h"

// Forward declare some things //
class CefScopedSandboxInfo;

#define REQUIRE_UI_THREAD()   assert(CefCurrentlyOn(TID_UI));
#define REQUIRE_IO_THREAD()   assert(CefCurrentlyOn(TID_IO));
#define REQUIRE_FILE_THREAD() assert(CefCurrentlyOn(TID_FILE));

namespace Leviathan{




	//! \brief Keeps certain CEF objects allocated for long enough
	class CEFSandboxInfoKeeper{
		friend GlobalCEFHandler;
	public:
		CEFSandboxInfoKeeper();
		DLLEXPORT ~CEFSandboxInfoKeeper();

		void* GetPtr();

	protected:
		shared_ptr<CefScopedSandboxInfo> ScopedInfo;
		CefRefPtr<Gui::CefApplication> CEFApp;
		void* SandBoxAccess;
	};

	//! \brief Singleton class for handling CEF initialization that needs to be done right away
	class GlobalCEFHandler{
	public:

		//! \brief This is the first function called in the Engine to handle CEF child processes
		//!
		//! This function will check command line arguments and possibly just run a subprocess or continue with the main application
		//! Passing command line argument of "--nogui" will skip this step and CEF initialization
		DLLEXPORT static bool CEFFirstCheckChildProcess(const wstring &commandline, int &returnvalue, shared_ptr<CEFSandboxInfoKeeper> &keeper,
#ifdef _WIN32
				HINSTANCE hInstance
#endif // _WIN32
			);

		DLLEXPORT static void CEFLastThingInProgram();
		DLLEXPORT static CEFSandboxInfoKeeper* GetCEFObjects();

		DLLEXPORT static void DoCEFMessageLoopWork();

	private:
		GlobalCEFHandler();
		~GlobalCEFHandler();
	protected:

		//! A flag for making sure that functions are only ran if CEF is actually used
		static bool CEFInitialized;
		static CEFSandboxInfoKeeper* AccessToThese;
	};

}
#endif