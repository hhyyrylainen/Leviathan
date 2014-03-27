#ifndef LEVIATHAN_GUICEFAPPLICATION
#define LEVIATHAN_GUICEFAPPLICATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "include/cef_app.h"
#include "include/wrapper/cef_message_router.h"
#include "JSEventInterface.h"


namespace Leviathan{ namespace Gui{

	//! \brief Handler for new render processes
	class CefApplication : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler{
		struct CustomExtensionFileData{
			CustomExtensionFileData(const string &file, const string &filecontents) : FileName(file), Contents(filecontents){
			}

			const string FileName;
			const string Contents;
		};
	public:
		CefApplication();
		DLLEXPORT ~CefApplication();


		virtual void OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line);

		//! \todo Register custom schemes
		virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE;


		//! \note This is called after the browser UI thread is initialized
		virtual void OnContextInitialized() OVERRIDE;
		virtual void OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;

		// CefRenderProcessHandler methods.
		virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;
		virtual void OnWebKitInitialized() OVERRIDE;
		virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
		virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;
		virtual bool OnBeforeNavigation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request,
			NavigationType navigation_type,	bool is_redirect) OVERRIDE;

		virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,	CefRefPtr<CefV8Context> context) OVERRIDE;
		virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;


		virtual bool OnProcessMessageReceived(
			CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message) OVERRIDE;


		virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }


		void StartListeningForEvent(JSNativeCoreAPI::JSListener* eventsinfo);
		void StopListeningForEvents();

		//! \brief Registers a custom file for all processes to load as V8 extension
		//! \note Only one should ever be registered, for performance reasons
		DLLEXPORT void RegisterCustomExtensionFile(const string &file);


		IMPLEMENT_REFCOUNTING(CefApplication);

	private:


		bool _PMCheckIsEvent(CefRefPtr<CefProcessMessage> &message);

		// ------------------------------------ //
		CefRefPtr<CefMessageRouterRendererSide> RendererRouter;

		CefRefPtr<JSNativeCoreAPI> NativeCoreLeviathanAPI;

		//! Store pointer to our browser
		CefRefPtr<CefBrowser> OurBrowser;

		//! Custom extension storage
		std::vector<std::string> CustomExtensionFiles;

		std::vector<unique_ptr<CustomExtensionFileData>> ExtensionContents;
	};

}}
#endif

