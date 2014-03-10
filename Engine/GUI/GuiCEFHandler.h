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

		// CefContextMenuHandler methods
		virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefContextMenuParams> params,
			CefRefPtr<CefMenuModel> model) OVERRIDE {
				// Customize the context menu...
		}
		virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefContextMenuParams> params,
			int command_id,
			EventFlags event_flags) OVERRIDE {
				// Handle a context menu command...
		}

		// CefDisplayHandler methods
		virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
			bool isLoading,
			bool canGoBack,
			bool canGoForward) OVERRIDE {
				// Update UI for browser state...
		}
		virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			const CefString& url) OVERRIDE {
				// Update the URL in the address bar...
		}
		virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
			const CefString& title) OVERRIDE {
				// Update the browser window title...
		}
		virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
			const CefString& message,
			const CefString& source,
			int line) OVERRIDE {
				// Log a console message...
		}

		// CefDownloadHandler methods
		virtual void OnBeforeDownload(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefDownloadItem> download_item,
			const CefString& suggested_name,
			CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE {
				// Specify a file path or cancel the download...
		}
		virtual void OnDownloadUpdated(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefDownloadItem> download_item,
			CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE {
				// Update the download status...
		}

		// CefDragHandler methods
		virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefDragData> dragData,
			DragOperationsMask mask) OVERRIDE {
				// Allow or deny drag events...
		}

		// CefGeolocationHandler methods
		virtual void OnRequestGeolocationPermission(
			CefRefPtr<CefBrowser> browser,
			const CefString& requesting_url,
			int request_id,
			CefRefPtr<CefGeolocationCallback> callback) OVERRIDE {
				// Allow or deny geolocation API access...
		}

		// CefKeyboardHandler methods
		virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
			const CefKeyEvent& event,
			CefEventHandle os_event,
			bool* is_keyboard_shortcut) OVERRIDE {
				// Perform custom handling of key events...
		}

		// CefLifeSpanHandler methods
		virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			const CefString& target_url,
			const CefString& target_frame_name,
			const CefPopupFeatures& popupFeatures,
			CefWindowInfo& windowInfo,
			CefRefPtr<CefClient>& client,
			CefBrowserSettings& settings,
			bool* no_javascript_access) OVERRIDE {
				// Allow or block popup windows, customize popup window creation...
		}
		virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE {
			// Browser window created successfully...
		}
		virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE {
			// Allow or block browser window close...
		}
		virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE {
			// Browser window is closed, perform cleanup...
		}

		// CefLoadHandler methods
		virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame) OVERRIDE {
				// A frame has started loading content...
		}
		virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			int httpStatusCode) OVERRIDE {
				// A frame has finished loading content...
		}
		virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			ErrorCode errorCode,
			const CefString& errorText,
			const CefString& failedUrl) OVERRIDE {
				// A frame has failed to load content...
		}
		virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
			TerminationStatus status) OVERRIDE {
				// A render process has crashed...
		}

		// CefRequestHandler methods
		virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefRequest> request) OVERRIDE {
				// Optionally intercept resource requests...
		}
		virtual bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
			const CefString& origin_url,
			int64 new_size,
			CefRefPtr<CefQuotaCallback> callback) OVERRIDE {
				// Allow or block quota requests...
		}
		virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
			const CefString& url,
			bool& allow_os_execution) OVERRIDE {
				// Handle execution of external protocols...
		}

		virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefGeolocationHandler> GetGeolocationHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
		virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }

		IMPLEMENT_REFCOUNTING(CefHandler);

	private:

	};

}}
#endif