// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/BaseNotifiable.h"
#include "Common/ReferenceCounted.h"
#include "Events/CallableObject.h"
#include "GuiInputSettings.h"
#include "JSProxyable.h"

#include "OgreMaterial.h"
#include "OgreTexture.h"

#include "include/cef_client.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_render_handler.h"
#include "wrapper/cef_message_router.h"

#include <atomic>

namespace Leviathan { namespace GUI {

class LeviathanJavaScriptAsync;

//! Controls what functions can be called from the page
enum VIEW_SECURITYLEVEL {
    //! The page is not allowed to access anything
    //! Except the notification about input element being focused
    VIEW_SECURITYLEVEL_BLOCKED = 0,

    //! The page can view minimal information only and access some objects
    //! \note This is recommended for external "unsafe" pages, like a web page displayed during
    //! connecting to a server
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
class View : public CefClient,
             public CefContextMenuHandler,
             public CefDisplayHandler,
             public CefDownloadHandler,
             public CefDragHandler,
             public CefKeyboardHandler,
             public CefLifeSpanHandler,
             public CefLoadHandler,
             public CefRequestHandler,
             public CefRenderHandler,
             public ThreadSafe,
             public CallableObject {
    friend class LeviathanJavaScriptAsync;

public:
    DLLEXPORT View(GuiManager* owner, Window* window,
        VIEW_SECURITYLEVEL security = VIEW_SECURITYLEVEL_ACCESS_ALL);
    DLLEXPORT ~View();

    //! \brief Sets the order Views are drawn, higher value is draw under other Views
    //! \param zcoord The z-coordinate, should be between -1 and 1, higher lower values mean
    //! that it will be drawn earlier \note Actually it most likely won't be drawn earlier, but
    //! it will overwrite everything below it (if it isn't transparent)
    //! \todo This is unimplemented
    DLLEXPORT void SetZVal(float zcoord);

    //! \brief Must be called before using, initializes required Ogre resources
    //! \return True when succeeds
    DLLEXPORT bool Init(const std::string& filetoload, const NamedVars& headervars);

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

    DLLEXPORT inline INPUT_MODE GetInputMode() const
    {
        // Automatically reject input if we have no browser host
        if(!OurBrowser)
            return INPUT_MODE::None;
        return InputMode;
    }

    //! \brief Sets the input mode. This should be regularly called from game code to update
    //! how the key presses should be sent to this View (or not sent)
    DLLEXPORT inline void SetInputMode(INPUT_MODE newmode)
    {
        InputMode = newmode;
    }

    //! \returns True if this has a focused input element
    DLLEXPORT inline bool HasFocusedInputElement() const
    {
        return InputFocused;
    }

    //! \returns True if there is a scrollable element under the mouse
    DLLEXPORT inline bool HasScrollableElementUnderCursor() const
    {
        return ScrollableElement;
    }

    //! \brief Passes events to the render process
    DLLEXPORT int OnEvent(Event* event) override;
    //! \brief Passes generic events to the render process
    DLLEXPORT int OnGenericEvent(GenericEvent* event) override;

    //! \brief Uses jQuery toggle method on target DOM element
    DLLEXPORT void ToggleElement(const std::string& name);

    DLLEXPORT CefRefPtr<CefBrowserHost> GetBrowserHost();

    // CEF callbacks //

    virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY,
        int& screenX, int& screenY) override;

    virtual bool GetScreenInfo(
        CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;

    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;

    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
        const RectList& dirtyRects, const void* buffer, int width, int height) override;

    virtual void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor,
        CursorType type, const CefCursorInfo& custom_cursor_info) override;

    virtual void OnScrollOffsetChanged(
        CefRefPtr<CefBrowser> browser, double x, double y) override;


    // CefDisplayHandler methods
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading,
        bool canGoBack, bool canGoForward) override;
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        const CefString& url) override;
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level,
        const CefString& message, const CefString& source, int line) override;

    // CefDownloadHandler methods
    virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback) override;
    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        CefRefPtr<CefDownloadItemCallback> callback) override;

    // CefLifeSpanHandler methods
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        const CefString& target_url, const CefString& target_frame_name,
        CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture,
        const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
        CefRefPtr<CefClient>& client, CefBrowserSettings& settings,
        bool* no_javascript_access) override;

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event,
        CefEventHandle os_event) override;

    // CefLoadHandler methods
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        TransitionType transition_type) override;
    //! \todo Make that future multi Gui::View windows don't all get focus back
    virtual void OnLoadEnd(
        CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override;

    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url,
        bool& allow_os_execution) override;

    // Request handler methods //
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request, bool is_redirect) override;

    virtual CefRequestHandler::ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request,
        CefRefPtr<CefRequestCallback> callback) override;

    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request) override;

    virtual void OnResourceRedirect(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response,
        CefString& new_url) override;

    virtual bool GetAuthCredentials(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        bool isProxy, const CefString& host, int port, const CefString& realm,
        const CefString& scheme, CefRefPtr<CefAuthCallback> callback) override;

    virtual bool OnQuotaRequest(CefRefPtr<CefBrowser> browser, const CefString& origin_url,
        int64 new_size, CefRefPtr<CefRequestCallback> callback) override;

    virtual bool OnCertificateError(CefRefPtr<CefBrowser> browser, cef_errorcode_t cert_error,
        const CefString& request_url, CefRefPtr<CefSSLInfo> ssl_info,
        CefRefPtr<CefRequestCallback> callback) override;

    // TODO: fix this
    // virtual bool OnBeforePluginLoad(const CefString& mime_type, const CefString& plugin_url,
    //     bool is_main_frame, const CefString& top_origin_url,
    //     CefRefPtr<CefWebPluginInfo> plugin_info,
    //     CefRequestContextHandler::PluginPolicy* plugin_policy) override;

    virtual void OnPluginCrashed(
        CefRefPtr<CefBrowser> browser, const CefString& plugin_path) override;

    virtual void OnRenderProcessTerminated(
        CefRefPtr<CefBrowser> browser, TerminationStatus status) override;


    //! \todo check access level properly
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefDragHandler> GetDragHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override
    {
        return this;
    }

    IMPLEMENT_REFCOUNTING(View);

protected:
    friend void intrusive_ptr_add_ref(View* obj)
    {
        obj->AddRef();
    }

    friend void intrusive_ptr_release(View* obj)
    {
        obj->Release();
    }

protected:
    bool _PMCheckIsEvent(const CefString& name, CefRefPtr<CefProcessMessage>& message);

    //! \todo It would be quite good if this and HandleProcessMessage (the AudioSource part)
    //! could be moved to one file so that all the implementation for the proxy would be in the
    //! same place
    void _HandleAudioSourceMessage(const CefRefPtr<CefProcessMessage>& message);

    void _HandleDestroyProxyMsg(int id);

protected:
    //! Unique ID
    const int ID;

    //! This View's security level
    //! \see VIEW_SECURITYLEVEL
    VIEW_SECURITYLEVEL ViewSecurity;

    //! This makes sure that opened iframes cannot access dangerous things by default
    //! \todo Allow configuring per frame
    VIEW_SECURITYLEVEL AdditionalFrameSecurity = VIEW_SECURITYLEVEL_BLOCKED;

    //! Current focus state, set with NotifyFocusUpdate
    bool OurFocus = false;

    //! Stored access to matching window
    Window* Wind;
    //! Owning GuiManager
    GuiManager* Owner;

    //! Name of the Ogre texture
    std::string TextureName;

    //! Name of the Ogre material
    std::string MaterialName;

    Ogre::SceneNode* Node = nullptr;
    Ogre::Item* QuadItem;
    Ogre::MeshPtr QuadMesh;

    //! The direct texture pointer
    Ogre::TexturePtr Texture;

    CefRefPtr<CefBrowser> OurBrowser;


    // Message passing //
    CefRefPtr<CefMessageRouterBrowserSide> OurBrowserSide = nullptr;
    LeviathanJavaScriptAsync* OurAPIHandler;

    //! Support for input when text box is focused
    //! \todo This needs to be tracked per frame
    std::atomic<bool> InputFocused = false;

    //! Support for scrolling when the mouse is over a scrollable thing
    //! \todo This needs to be tracked per frame
    std::atomic<bool> ScrollableElement = false;

    //! The mode of input. used by
    std::atomic<INPUT_MODE> InputMode = INPUT_MODE::Menu;

    //! Keeps track of events that are registered
    std::map<EVENT_TYPE, int> RegisteredEvents;

    //! Keeps track of generic events
    std::map<std::string, int> RegisteredGenerics;

    //! Proxied objects. These are sent messages from the render process from the proxy objects
    //! there that are exposed to JavaScript
    std::map<int, JSProxyable::pointer> ProxyedObjects;
};

}} // namespace Leviathan::GUI
