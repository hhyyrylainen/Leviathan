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
#include "GUI/LeviathanJavaScriptAsync.h"

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

		//! \brief Gets the corresponding Gui::CefApplication
		DLLEXPORT CefRefPtr<Gui::CefApplication> GetCEFApp() const;

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

		//! \brief Registers a new JavaScript query handler
		//! \param newdptr Pass a newed object or NULL
		//! \todo Add support for removing existing ones
		DLLEXPORT static void RegisterCustomJavaScriptQueryHandler(Gui::JSAsyncCustom* newdptr);
		//! \brief Unregisters a registered query handler
		//! \see RegisterCustomJavaScriptQueryHandler
		DLLEXPORT static void UnRegisterCustomJavaScriptQueryHandler(Gui::JSAsyncCustom* toremove);

		//! \brief Mainly allows LeviathanJavaScriptAsync to access the list of handlers
		DLLEXPORT static const std::vector<shared_ptr<Gui::JSAsyncCustom>>& GetRegisteredCustomHandlers();

		//! \brief Registers a LeviathanJavaScriptAsync to receive notifications about JSAsyncCustom changes
		DLLEXPORT static void RegisterJSAsync(Gui::LeviathanJavaScriptAsync* ptr);
		//! \brief Unregisters a registered LeviathanJavaScriptAsync
		//! \see RegisterJSAsync
		DLLEXPORT static void UnRegisterJSAsync(Gui::LeviathanJavaScriptAsync* ptr);


	private:
		GlobalCEFHandler();
		~GlobalCEFHandler();
	protected:

		//! A flag for making sure that functions are only ran if CEF is actually used
		static bool CEFInitialized;
		static CEFSandboxInfoKeeper* AccessToThese;

		//! Stores all the custom handlers
		static std::vector<shared_ptr<Gui::JSAsyncCustom>> CustomJSHandlers;

		//! Stored to be able to notify all LeviathanJavaScriptAsync objects
		static std::vector<Gui::LeviathanJavaScriptAsync*> JSAsynToNotify;

		static boost::recursive_mutex JSCustomMutex;
	};

}
#endif