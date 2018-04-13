// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "JSEventInterface.h"

#include "include/cef_app.h"
#include "include/wrapper/cef_message_router.h"


namespace Leviathan { namespace GUI {

//! \brief Handler for new render processes
class CefApplication : public CefApp,
                       public CefBrowserProcessHandler,
                       public CefRenderProcessHandler {
public:
    using HandlerFactory = CefRefPtr<CefV8Handler> (*)(CefApplication* app);

    struct CustomExtension {
        inline CustomExtension(
            const std::string& extname, const std::string& contents, HandlerFactory handler) :
            ExtName(extname),
            Contents(contents), Handler(handler)
        {
        }

        //! Name of this extension. For example "Leviathan/MyCustomThing"
        const std::string ExtName;

        //! The whole javascript text that is the extension
        const std::string Contents;

        //! A pointer to a global factory function that creates a handler for this.
        //! May be null if no handler is needed. The factory method may also return null
        HandlerFactory Handler;
    };

public:
    CefApplication();
    DLLEXPORT ~CefApplication();


    virtual void OnBeforeCommandLineProcessing(
        const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

    //! \todo Register custom schemes
    virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;

    //! \note This is called after the browser UI thread is initialized
    virtual void OnContextInitialized() override;
    virtual void OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info) override;

    // CefRenderProcessHandler methods.
    //! \todo Should the custom extensions require static strings that would also be available
    //! in the render process? To reduce copying and converting between utf8 and utf16
    virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) override;
    virtual void OnWebKitInitialized() override;
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;
    // I guess this now needs to be done by: OnBeforeBrowse

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) override;
    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) override;


    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;


    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
    {
        return this;
    }


    void StartListeningForEvent(JSNativeCoreAPI::JSListener* eventsinfo);
    void StopListeningForEvents();

    //! \brief Registers a custom extension for all render processes to load as a V8 extension
    //! \note Only one should ever be registered, for performance reasons
    DLLEXPORT void RegisterCustomExtension(std::unique_ptr<CustomExtension>&& extension);


    IMPLEMENT_REFCOUNTING(CefApplication);

private:
    bool _PMCheckIsEvent(CefRefPtr<CefProcessMessage>& message);

    // ------------------------------------ //
    CefRefPtr<CefMessageRouterRendererSide> RendererRouter;

    CefRefPtr<JSNativeCoreAPI> NativeCoreLeviathanAPI;

    //! Store pointer to our browser
    CefRefPtr<CefBrowser> OurBrowser;

    //! Custom extension storage
    std::vector<std::unique_ptr<CustomExtension>> CustomExtensions;
};

}} // namespace Leviathan::GUI
