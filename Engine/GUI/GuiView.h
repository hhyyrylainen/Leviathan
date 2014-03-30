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
#include "include/wrapper/cef_message_router.h"
#include "Common/BaseNotifiable.h"
#include "Events/CallableObject.h"


namespace Leviathan{ namespace Gui{

	//! Controls what functions can be called from the page
	enum VIEW_SECURITYLEVEL {
		//! The page is not allowed to access anything
		VIEW_SECURITYLEVEL_BLOCKED = 0,

		//! The page can view minimal view only information and access some objects
		//! \note This is recommended for external "unsafe" pages, like a web page displayed during connecting to a server
		VIEW_SECURITYLEVEL_MINIMAL,

		//! The page can view most settings and set some "safe" settings
		VIEW_SECURITYLEVEL_NORMAL,

		//! The page can access all internal functions
		//! \note This is the recommended level for games' internal GUI page
		VIEW_SECURITYLEVEL_ACCESS_ALL
	};


	//! \brief A class that represents a single GUI layer that has it's own chromium browser
	//!
	//! GUI can be layered by setting the z coordinate of Views different
	class View : public CefClient, public CefContextMenuHandler, public CefDisplayHandler, public CefDownloadHandler,	public CefDragHandler,
		public CefGeolocationHandler, public CefKeyboardHandler, public CefLifeSpanHandler,	public CefLoadHandler, public CefRequestHandler,
		public CefRenderHandler, public ThreadSafe, public CallableObject
	{
		friend LeviathanJavaScriptAsync;

		class RenderDataHolder{
		public:
			RenderDataHolder() : BufferSize(0), Buffer(NULL), Width(0), Height(0), Updated(false){

			}
			~RenderDataHolder(){
				SAFE_DELETE(Buffer);
			}

			PaintElementType Type;
			size_t BufferSize;
			void* Buffer;
			int Width;
			int Height;
			bool Updated;
		};
	public:
		DLLEXPORT View(GuiManager* owner, Window* window, VIEW_SECURITYLEVEL security = VIEW_SECURITYLEVEL_ACCESS_ALL);
		DLLEXPORT ~View();

		//! \brief Sets the order Views are drawn, higher value is draw under other Views
		//! \param zcoord The z-coordinate, should be between -1 and 1, higher lower values mean that it will be drawn earlier
		//! \note Actually it most likely won't be drawn earlier, but it will overwrite everything below it (if it isn't transparent)
		DLLEXPORT void SetZVal(float zcoord);

		//! \brief Must be called before using, initializes required Ogre resources
		//! \return True when succeeds
		DLLEXPORT bool Init(const wstring &filetoload, const NamedVars &headervars);

		//! \brief Must be called before destroying to release allocated Ogre resources
		DLLEXPORT void ReleaseResources();


		//! \brief Updates the texture
		//!
		//! Should be called before rendering on the main thread
		DLLEXPORT void CheckRender();

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

		virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
		virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;



		DLLEXPORT void SetCurrentInputHandlingWindow(Window* wind);

		DLLEXPORT CefRefPtr<CefBrowserHost> GetBrowserHost();

		//! \brief Sets the CanPaint variable allowing or preventing this object from updating the texture
		//! \note When setting to true the whole browser is invalidated and will be redrawn
		//! \warning Doesn't actually do anything
		DLLEXPORT void SetAllowPaintStatus(bool canpaintnow);

		virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event) OVERRIDE;

		// CefLoadHandler methods
		virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame) OVERRIDE;
		//! \todo Make that future multi Gui::View windows don't all get focus back
		virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			int httpStatusCode) OVERRIDE;
		virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			ErrorCode errorCode,
			const CefString& errorText,
			const CefString& failedUrl) OVERRIDE;

		virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
			const CefString& url,
			bool& allow_os_execution) OVERRIDE;

		// Request handler methods //
		virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefRequest> request,
			bool is_redirect) OVERRIDE;

		virtual bool OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefRequest> request) OVERRIDE;

		virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefRequest> request) OVERRIDE;

		virtual void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			const CefString& old_url,
			CefString& new_url) OVERRIDE;

		virtual bool GetAuthCredentials(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			bool isProxy,
			const CefString& host,
			int port,
			const CefString& realm,
			const CefString& scheme,
			CefRefPtr<CefAuthCallback> callback) OVERRIDE;

		virtual bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
			const CefString& origin_url,
			int64 new_size,
			CefRefPtr<CefQuotaCallback> callback) OVERRIDE;

		virtual bool OnCertificateError(
			cef_errorcode_t cert_error,
			const CefString& request_url,
			CefRefPtr<CefAllowCertificateErrorCallback> callback) OVERRIDE;

		virtual bool OnBeforePluginLoad(CefRefPtr<CefBrowser> browser,
			const CefString& url,
			const CefString& policy_url,
			CefRefPtr<CefWebPluginInfo> info) OVERRIDE;

		virtual void OnPluginCrashed(CefRefPtr<CefBrowser> browser,
			const CefString& plugin_path) OVERRIDE;

		virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
			TerminationStatus status) OVERRIDE;


		virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message)
			OVERRIDE;

		//! \brief Passes events to the render process
		DLLEXPORT virtual int OnEvent(Event** pEvent);
		//! \brief Passes generic events to the render process
		DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent);

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


		//! \brief Allows scripts to toggle gui elements
		DLLEXPORT void ToggleElement(const string &name);

		IMPLEMENT_REFCOUNTING(CefApplication);

	public:
		
		void AddRefProxy(){
			AddRef();
		}
		void ReleaseProxy(){
			Release();
		}

	protected:


		bool _PMCheckIsEvent(CefRefPtr<CefProcessMessage> &message);

		// ------------------------------------ //

		//! Unique ID
		int ID;

		//! This View's security level
		//! \see VIEW_SECURITYLEVEL
		VIEW_SECURITYLEVEL ViewSecurity;

		//! Current focus state, set with NotifyFocusUpdate
		bool OurFocus;

		//! Stored access to matching window
		Window* Wind;
		//! Owning GuiManager
		GuiManager* Owner;

		//! Name of the Ogre material
		string MaterialName;
		//! Name of the Ogre texture
		string TextureName;


		//! Prevents crashing from painting the window too soon
		bool CanPaint;

		//! The quad to which the browser is rendered
		Ogre::ManualObject* CEFOverlayQuad;

		//! Node that has the overlay, can be used to hide it //
		Ogre::SceneNode* CEFSNode;

		//! The direct texture pointer
		Ogre::TexturePtr Texture;

		//! The direct material pointer
		Ogre::MaterialPtr Material;

		CefRefPtr<CefBrowser> OurBrowser;


		// Message passing //
		CefRefPtr<CefMessageRouterBrowserSide> OurBrowserSide;
		LeviathanJavaScriptAsync* OurAPIHandler;


		//! Holds the buffer before it is transferred into a texture
		shared_ptr<RenderDataHolder> RenderHolderForMain;
		shared_ptr<RenderDataHolder> RenderHolderForPopUp;

		//! Keeps track of events that are registered
		std::map<EVENT_TYPE, int> RegisteredEvents;

		//! Keeps track of generic events
		std::map<wstring, int> RegisteredGenerics;

	};

}}
#endif