// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/BaseNotifiable.h"
#include "Events/CallableObject.h"
#include "GuiLayer.h"
#include "JSProxyable.h"
#include "Rendering/RotatingBufferHelper.h"

#include "bsfCore/BsCorePrerequisites.h"

#include "include/cef_client.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_render_handler.h"
#include "include/cef_request_handler.h"
#include "wrapper/cef_message_router.h"

namespace Leviathan { namespace GUI {

class LeviathanJavaScriptAsync;

//! The default CEF scroll speed is ridiculously slow so we multiply it with this
constexpr auto CEF_MOUSE_SCROLL_MULTIPLIER = 35.f;

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
//! \todo Split the process messages away from this into the places that actually sent them
//! "JSNativeCoreAPI.cpp" to be clearer
//! \todo If this is to be kept around rename this to CEFLayer
class View : public Layer,
             public CefClient,
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
             public CallableObject,
             RotatingBufferHelper<bs::SPtr<bs::PixelData>, 4> {
    friend class LeviathanJavaScriptAsync;

public:
    DLLEXPORT View(GuiManager* owner, Window* window, int renderorder,
        VIEW_SECURITYLEVEL security = VIEW_SECURITYLEVEL_ACCESS_ALL);
    DLLEXPORT ~View();

    //! \brief Must be called before using, initializes teh required rendering resources
    //! \return True on success
    DLLEXPORT bool Init(const std::string& filetoload, const NamedVars& headervars);

    DLLEXPORT inline INPUT_MODE GetInputMode() const override
    {
        // Automatically reject input if we have no browser host
        if(!OurBrowser)
            return INPUT_MODE::None;
        return InputMode;
    }

    //! \brief Passes events to the render process
    DLLEXPORT int OnEvent(Event* event) override;
    //! \brief Passes generic events to the render process
    DLLEXPORT int OnGenericEvent(GenericEvent* event) override;

    //! \brief Uses jQuery toggle method on target DOM element
    DLLEXPORT void ToggleElement(const std::string& name);

    DLLEXPORT CefRefPtr<CefBrowserHost> GetBrowserHost();


    //! This converts keypresses to CEF events and injects them
    DLLEXPORT bool OnReceiveKeyInput(const SDL_Event& sdlevent, bool down, bool textinput,
        int mousex, int mousey, int specialkeymodifiers, int cefspecialkeymodifiers) override;

    DLLEXPORT void OnMouseMove(const SDL_Event& event, bool outsidewindow) override;

    DLLEXPORT void OnMouseWheel(const SDL_Event& event) override;

    DLLEXPORT void OnMouseButton(const SDL_Event& event, bool down) override;

    // CEF callbacks //

    virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

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

    // Request handler methods //
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect) override;

    virtual bool OnQuotaRequest(CefRefPtr<CefBrowser> browser, const CefString& origin_url,
        int64 new_size, CefRefPtr<CefRequestCallback> callback) override;

    virtual bool OnCertificateError(CefRefPtr<CefBrowser> browser, cef_errorcode_t cert_error,
        const CefString& request_url, CefRefPtr<CefSSLInfo> ssl_info,
        CefRefPtr<CefRequestCallback> callback) override;

    virtual void OnPluginCrashed(
        CefRefPtr<CefBrowser> browser, const CefString& plugin_path) override;

    virtual void OnRenderProcessTerminated(
        CefRefPtr<CefBrowser> browser, TerminationStatus status) override;


    //! \todo check access level properly
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame, CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override;

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

    // Cef refcount overrides
    void AddRef() const OVERRIDE
    {
        const_cast<View*>(this)->ReferenceCounted::AddRef();
    }
    bool Release() const OVERRIDE
    {
        const_cast<View*>(this)->ReferenceCounted::Release();
        // We don't know if us were deleted. Hopefully nothing in CEF needs this
        return false;
    }
    bool HasOneRef() const OVERRIDE
    {
        return GetRefCount() == 1;
    }

    bool HasAtLeastOneRef() const OVERRIDE
    {
        return GetRefCount() >= 1;
    }

protected:
    DLLEXPORT void _DoReleaseResources() override;
    DLLEXPORT void _OnWindowResized() override;
    DLLEXPORT void _OnFocusChanged() override;
    bs::SPtr<bs::PixelData> _OnNewBufferNeeded() override;

    bool _PMCheckIsEvent(const CefString& name, CefRefPtr<CefProcessMessage>& message);

    //! \todo It would be quite good if this and HandleProcessMessage (the AudioSource part)
    //! could be moved to one file so that all the implementation for the proxy would be in the
    //! same place
    void _HandleAudioSourceMessage(const CefRefPtr<CefProcessMessage>& message);

    void _HandlePlayCutsceneMessage(const CefRefPtr<CefProcessMessage>& message);

    void _HandleDestroyProxyMsg(int id);

protected:
    //! This View's security level
    //! \see VIEW_SECURITYLEVEL
    VIEW_SECURITYLEVEL ViewSecurity;

    //! This makes sure that opened iframes cannot access dangerous things by default
    //! \todo Allow configuring per frame
    VIEW_SECURITYLEVEL AdditionalFrameSecurity = VIEW_SECURITYLEVEL_BLOCKED;

    CefRefPtr<CefBrowser> OurBrowser;

    // Message passing //
    CefRefPtr<CefMessageRouterBrowserSide> OurBrowserSide = nullptr;
    LeviathanJavaScriptAsync* OurAPIHandler;

    //! Keeps track of events that are registered
    std::map<EVENT_TYPE, int> RegisteredEvents;

    //! Keeps track of generic events
    std::map<std::string, int> RegisteredGenerics;

    //! Proxied objects. These are sent messages from the render process from the proxy objects
    //! there that are exposed to JavaScript
    std::map<int, JSProxyable::pointer> ProxyedObjects;

    // Rendering resources
    bs::HTexture Texture;

    // Store for needed texture size
    int NeededTextureWidth = -1;
    int NeededTextureHeight = -1;

    std::vector<uint8_t> IntermediateTextureBuffer;
};

}} // namespace Leviathan::GUI
