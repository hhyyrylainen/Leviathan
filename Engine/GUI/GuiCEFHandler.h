#ifndef LEVIATHAN_GUICEFHANDLER
#define LEVIATHAN_GUICEFHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "include/cef_client.h"

namespace Leviathan{ namespace Gui{

	class CefHandler : public CefClient, public CefContextMenuHandler, public CefDisplayHandler, public CefDownloadHandler,	public CefDragHandler,
		public CefGeolocationHandler, public CefKeyboardHandler, public CefLifeSpanHandler,	public CefLoadHandler, public CefRequestHandler
	{
	public:
		CefHandler();
		DLLEXPORT ~CefHandler();

		// CefDisplayHandler methods
		virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
			bool isLoading,
			bool canGoBack,
			bool canGoForward) OVERRIDE;
		virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			const CefString& url) OVERRIDE;
		virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
			const CefString& title) OVERRIDE;
		virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
			const CefString& message,
			const CefString& source,
			int line) OVERRIDE;

		// CefDownloadHandler methods
		virtual void OnBeforeDownload(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefDownloadItem> download_item,
			const CefString& suggested_name,
			CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;
		virtual void OnDownloadUpdated(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefDownloadItem> download_item,
			CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE;

		// CefGeolocationHandler methods
		virtual void OnRequestGeolocationPermission(
			CefRefPtr<CefBrowser> browser,
			const CefString& requesting_url,
			int request_id,
			CefRefPtr<CefGeolocationCallback> callback) OVERRIDE;
		// CefLifeSpanHandler methods
		virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			const CefString& target_url,
			const CefString& target_frame_name,
			const CefPopupFeatures& popupFeatures,
			CefWindowInfo& windowInfo,
			CefRefPtr<CefClient>& client,
			CefBrowserSettings& settings,
			bool* no_javascript_access) OVERRIDE;

		DLLEXPORT void SetCurrentInputHandlingWindow(Window* wind);

		virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event) OVERRIDE;

		virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
		virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

		// CefLoadHandler methods
		virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame) OVERRIDE;
		virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			int httpStatusCode) OVERRIDE;
		virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			ErrorCode errorCode,
			const CefString& errorText,
			const CefString& failedUrl) OVERRIDE;
		virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
			TerminationStatus status) OVERRIDE;

		virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
			const CefString& url,
			bool& allow_os_execution) OVERRIDE;

		virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE;
		virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE;
		virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() OVERRIDE;
		virtual CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE;
		virtual CefRefPtr<CefGeolocationHandler> GetGeolocationHandler() OVERRIDE;
		virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE;
		virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE;
		virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;
		virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE;

		IMPLEMENT_REFCOUNTING(CefHandler);

	protected:


		Window* InputHandlingWindow;

	};

}}
#endif