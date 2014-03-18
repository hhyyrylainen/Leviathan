#ifndef LEVIATHAN_GUIVIEW
#define LEVIATHAN_GUIVIEW
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "include/cef_render_handler.h"
#include "Common/ReferenceCounted.h"
#include "OgreTexture.h"
#include "OgreMaterial.h"
#include "include/cef_client.h"

namespace Leviathan{ namespace Gui{

	//! \brief A class that represents a single GUI layer that has it's own chromium browser
	//!
	//! GUI can be layered by setting the z coordinate of Views different
	class View : public CefClient, public CefContextMenuHandler, public CefDisplayHandler, public CefDownloadHandler,	public CefDragHandler,
		public CefGeolocationHandler, public CefKeyboardHandler, public CefLifeSpanHandler,	public CefLoadHandler, public CefRequestHandler,
		public CefRenderHandler
	{
	public:
		DLLEXPORT View(GuiManager* owner, Window* window);
		DLLEXPORT ~View();

		//! \brief Sets the order Views are drawn, higher value is draw under other Views
		//! \param zcoord The z-coordinate, should be between -1 and 1, higher lower values mean that it will be drawn earlier
		//! \note Actually it most likely won't be drawn earlier, but it will overwrite everything below it (if it isn't transparent)
		DLLEXPORT void SetZVal(float zcoord);

		//! \brief Must be called before using, initializes required Ogre resources
		//! \return True when succeeds
		DLLEXPORT bool Init();

		//! \brief Must be called before destroying to release allocated Ogre resources
		DLLEXPORT void ReleaseResources();

		//! \brief Notifies the internal browser that the window has resized
		//!
		//! Called by GuiManager
		DLLEXPORT void NotifyWindowResized();

		//! \brief Notifies the internal browser that focus has been updated
		//!
		//! Called by GuiManager
		DLLEXPORT void NotifyFocusUpdate(bool focused);


		// CEF callbacks //

		virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect);

		virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect);

		virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY);

		virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info);

		virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show);

		virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect);

		virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height);

		virtual void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor);

		virtual void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser);


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

		DLLEXPORT CefRefPtr<CefBrowserHost> GetBrowserHost();

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
		virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE;

		IMPLEMENT_REFCOUNTING(CefApplication);


	protected:

		//! Unique ID
		int ID;

		//! Stored access to matching window
		Window* Wind;
		//! Owning GuiManager
		GuiManager* Owner;

		//! Name of the Ogre material
		string MaterialName;
		//! Name of the Ogre texture
		string TextureName;

		//! The quad to which the browser is rendered
		Ogre::ManualObject* CEFOverlayQuad;

		//! The direct texture pointer
		Ogre::TexturePtr Texture;

		//! The direct material pointer
		Ogre::MaterialPtr Material;

		CefRefPtr<CefBrowser> OurBrowser;

	};

}}
#endif