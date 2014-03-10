#ifndef LEVIATHAN_GUICEFAPPLICATION
#define LEVIATHAN_GUICEFAPPLICATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "include/cef_app.h"
#include "include/client_app.h"

namespace Leviathan{ namespace Gui{

	class CefApplication : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler{
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
		virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,C efRefPtr<CefV8Context> context) OVERRIDE;


		virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }

		IMPLEMENT_REFCOUNTING(CefApplication);

	private:

	};

}}
#endif

