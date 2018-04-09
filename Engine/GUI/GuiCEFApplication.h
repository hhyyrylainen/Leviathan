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
    struct CustomExtensionFileData {
        CustomExtensionFileData(const std::string& file, const std::string& filecontents) :
            FileName(file), Contents(filecontents)
        {
        }

        const std::string FileName;
        const std::string Contents;
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

    //! \brief Registers a custom file for all processes to load as V8 extension
    //! \note Only one should ever be registered, for performance reasons
    DLLEXPORT void RegisterCustomExtensionFile(const std::string& file);


    IMPLEMENT_REFCOUNTING(CefApplication);

private:
    bool _PMCheckIsEvent(CefRefPtr<CefProcessMessage>& message);

    // ------------------------------------ //
    CefRefPtr<CefMessageRouterRendererSide> RendererRouter;

    CefRefPtr<JSNativeCoreAPI> NativeCoreLeviathanAPI;

    //! Store pointer to our browser
    CefRefPtr<CefBrowser> OurBrowser;

    //! Custom extension storage
    std::vector<std::string> CustomExtensionFiles;

    std::vector<std::unique_ptr<CustomExtensionFileData>> ExtensionContents;
};

}} // namespace Leviathan::GUI
