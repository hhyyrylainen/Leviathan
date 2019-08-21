// ------------------------------------ //
#include "GuiCEFApplication.h"

#include "CEFLocalResourceRequest.h"
#include "Common/StringOperations.h"
#include "Events/Event.h"
#include "FileSystem.h"
#include "Generated/LeviathanV8CoreExt.h"

#include "include/base/cef_logging.h"
using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
CefApplication::CefApplication() : RendererRouter(NULL) {}

DLLEXPORT CefApplication::~CefApplication() {}
// ------------------------------------ //
void CefApplication::OnContextInitialized()
{
    CefRegisterSchemeHandlerFactory(
        "http", "leviathan-local", new CefLocalResourceRequestHandlerFactory());
}

void CefApplication::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line)
{
    // Change command line if needed
}
// ------------------------------------ //
// These definitions are from CEF
// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
namespace switches {

// CEF and Chromium support a wide range of command-line switches. This file
// only contains command-line switches specific to the cefclient application.
// View CEF/Chromium documentation or search for *_switches.cc files in the
// Chromium source code to identify other existing command-line switches.
// Below is a partial listing of relevant *_switches.cc files:
//   base/base_switches.cc
//   cef/libcef/common/cef_switches.cc
//   chrome/common/chrome_switches.cc (not all apply)
//   content/public/common/content_switches.cc

const char kMultiThreadedMessageLoop[] = "multi-threaded-message-loop";
const char kExternalMessagePump[] = "external-message-pump";
const char kCachePath[] = "cache-path";
const char kUrl[] = "url";
const char kOffScreenRenderingEnabled[] = "off-screen-rendering-enabled";
const char kOffScreenFrameRate[] = "off-screen-frame-rate";
const char kTransparentPaintingEnabled[] = "transparent-painting-enabled";
const char kShowUpdateRect[] = "show-update-rect";
const char kMouseCursorChangeDisabled[] = "mouse-cursor-change-disabled";
const char kRequestContextPerBrowser[] = "request-context-per-browser";
const char kRequestContextSharedCache[] = "request-context-shared-cache";
const char kRequestContextBlockCookies[] = "request-context-block-cookies";
const char kBackgroundColor[] = "background-color";
const char kEnableGPU[] = "enable-gpu";
const char kFilterURL[] = "filter-url";
const char kUseViews[] = "use-views";
const char kHideFrame[] = "hide-frame";
const char kHideControls[] = "hide-controls";
const char kHideTopMenu[] = "hide-top-menu";
const char kWidevineCdmPath[] = "widevine-cdm-path";
const char kSslClientCertificate[] = "ssl-client-certificate";
const char kCRLSetsPath[] = "crl-sets-path";
const char kLoadExtension[] = "load-extension";

} // namespace switches



void CefApplication::OnBeforeCommandLineProcessing(
    const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
    // Check if we want to change something

    // Pass additional command-line flags to the browser process.
    if(process_type.empty()) {
        // Pass additional command-line flags when off-screen rendering is enabled.
        if(command_line->HasSwitch(switches::kOffScreenRenderingEnabled)) {
            // If the PDF extension is enabled then cc Surfaces must be disabled for
            // PDFs to render correctly.
            // See https://bitbucket.org/chromiumembedded/cef/issues/1689 for details.
            if(!command_line->HasSwitch("disable-extensions") &&
                !command_line->HasSwitch("disable-pdf-extension")) {
                command_line->AppendSwitch("disable-surfaces");
            }

            // Use software rendering and compositing (disable GPU) for increased FPS
            // and decreased CPU usage. This will also disable WebGL so remove these
            // switches if you need that capability.
            // See https://bitbucket.org/chromiumembedded/cef/issues/1257 for details.
            if(!command_line->HasSwitch(switches::kEnableGPU)) {
                command_line->AppendSwitch("disable-gpu");
                command_line->AppendSwitch("disable-gpu-compositing");
            }
        }

        if(command_line->HasSwitch(switches::kUseViews) &&
            !command_line->HasSwitch("top-chrome-md")) {
            // Use non-material mode on all platforms by default. Among other things
            // this causes menu buttons to show hover state. See usage of
            // MaterialDesignController::IsModeMaterial() in Chromium code.
            command_line->AppendSwitchWithValue("top-chrome-md", "non-material");
        }

        if(!command_line->HasSwitch(switches::kCachePath) &&
            !command_line->HasSwitch("disable-gpu-shader-disk-cache")) {
            // Don't create a "GPUCache" directory when cache-path is unspecified.
            command_line->AppendSwitch("disable-gpu-shader-disk-cache");
        }
    }
}

//
// End code from CEF
//

void CefApplication::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) {}

void CefApplication::OnBrowserCreated(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefDictionaryValue> extra_info)
{
    // Browser created in this render process...
    OurBrowser = browser;
}

void CefApplication::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
{
    // Browser destroyed in this render process...
    OurBrowser = NULL;
}
// ------------------------------------ //
void CefApplication::OnWebKitInitialized()
{
    // WebKit has been initialized, register V8 extensions...

    // Register it //
    const std::string levcoreext(LeviathanCoreV8ExtensionsStr);

    // Bind the event handling object //
    NativeCoreLeviathanAPI = new JSNativeCoreAPI(this);

    CefRefPtr<CefV8Handler> tmpptr = NativeCoreLeviathanAPI;

    CefRegisterExtension("Leviathan/API", levcoreext, tmpptr);

    // Load custom extensions //
    for(const auto& ext : CustomExtensions) {
        // Load it //
        CefRegisterExtension(
            ext->ExtName, ext->Contents, ext->Handler ? ext->Handler(this) : nullptr);
    }

    // Clear the memory //
    CustomExtensions.clear();
}

void CefApplication::OnContextCreated(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    // JavaScript context created, add V8 bindings here...

    RendererRouter->OnContextCreated(browser, frame, context);
}

void CefApplication::OnContextReleased(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    // JavaScript context released, release V8 references here...

    RendererRouter->OnContextReleased(browser, frame, context);
    NativeCoreLeviathanAPI->ClearContextValues();
}
// ------------------------------------ //
// void CefApplication::OnUncaughtException(CefRefPtr<CefBrowser> browser,
//     CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context,
//     CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stackTrace)
// {
//     LOG_ERROR("Uncaught exception in CEF: " + std::string(exception->GetMessage()));
// }
// ------------------------------------ //
void CefApplication::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info)
{
    // Send startup information to a new render process...


    // Send custom extensions to it //
    extra_info->SetInt(0, static_cast<int>(CustomExtensions.size()));

    for(int i = 0; i < static_cast<int>(CustomExtensions.size()); i++) {
        if(i != 0) {

            LOG_WARNING("CefApplication: loading multiple extensions, performance "
                        "might be worse than with one large extension");
        }

        // Report this //
        LOG_INFO("CefApplication: sending extension to renderer process: " +
                 CustomExtensions[i]->ExtName);

        const auto baseIndex = 1 + i * 3;

        // Pass data
        // Name
        extra_info->SetString(baseIndex + 0, CustomExtensions[i]->ExtName);

        // Contents
        extra_info->SetString(baseIndex + 1, CustomExtensions[i]->Contents);

        // The factory ptr
        extra_info->SetBinary(baseIndex + 2,
            CefBinaryValue::Create(&CustomExtensions[i]->Handler, sizeof(HandlerFactory)));
    }
}

void CefApplication::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
{
    // The render process main thread has been initialized...
    // Receive startup information in the new render process...

    // Create messaging functionality //
    CefMessageRouterConfig config;
    config.js_query_function = "cefQuery";
    config.js_cancel_function = "cefQueryCancel";

    RendererRouter = CefMessageRouterRendererSide::Create(config);

    // Copy the extensions away //
    int size = extra_info->GetInt(0);

    for(int i = 0; i < size; i++) {

        const auto baseIndex = 1 + i * 3;

        // Copy it to our vector //
        const auto name = Convert::Utf16ToUtf8(extra_info->GetString(baseIndex + 0));
        const auto contents = Convert::Utf16ToUtf8(extra_info->GetString(baseIndex + 1));
        const auto& bin = extra_info->GetBinary(baseIndex + 2);

        HandlerFactory factory;
        static_assert(
            sizeof(factory) == sizeof(HandlerFactory), "Handler ptr size is messed up");
        const auto read = bin->GetData(&factory, sizeof(factory), 0);

        if(read != sizeof(HandlerFactory)) {
            LOG(ERROR) << "Handler ptr read fail";
            factory = nullptr;
        }

        CustomExtensions.push_back(
            std::make_unique<CustomExtension>(name, contents, factory, nullptr));
    }
}
// ------------------------------------ //
void CefApplication::OnScheduleMessagePumpWork(int64 delay) {}
// ------------------------------------ //
bool CefApplication::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame, CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    // Handle IPC messages from the browser process...
    // This is in the renderer process
    if(RendererRouter->OnProcessMessageReceived(browser, frame, source_process, message))
        return true;

    if(_PMCheckIsEvent(message))
        return true;

    if(NativeCoreLeviathanAPI->HandleProcessMessage(browser, frame, source_process, message))
        return true;

    // TODO: custom extension messages in the render process

    // Not handled //
    return false;
}

DLLEXPORT void CefApplication::RegisterCustomExtension(
    std::shared_ptr<CustomExtension> extension)
{
    CustomExtensions.push_back(extension);
}
// ------------------------------------ //
DLLEXPORT void CefApplication::SendCustomExtensionMessage(CefRefPtr<CefProcessMessage> message)
{
    // TODO: add debug verification here for message name being "Custom"
    OurBrowser->GetMainFrame()->SendProcessMessage(PID_BROWSER, message);
}

void CefApplication::SendProcessMessage(CefRefPtr<CefProcessMessage> message)
{
    OurBrowser->GetMainFrame()->SendProcessMessage(PID_BROWSER, message);
}
// ------------------------------------ //
void CefApplication::StartListeningForEvent(JSNativeCoreAPI::JSListener* eventsinfo)
{
    // Tell that we now want this event //
    CefRefPtr<CefProcessMessage> message;

    if(eventsinfo->IsGeneric) {

        message = CefProcessMessage::Create("LGeneric");
        message->GetArgumentList()->SetString(0, eventsinfo->EventName);

    } else {

        message = CefProcessMessage::Create("LEvent");
        message->GetArgumentList()->SetInt(0, eventsinfo->EventsType);
    }

    // Send it //
    OurBrowser->GetMainFrame()->SendProcessMessage(PID_BROWSER, message);
}

void CefApplication::StopListeningForEvents()
{
    // Tell that we no longer want any //
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("LEvents");

    CefRefPtr<CefListValue> args = message->GetArgumentList();
    // Indicate that we want to stop receiving all messages //
    args->SetBool(0, true);

    OurBrowser->GetMainFrame()->SendProcessMessage(PID_BROWSER, message);
}

bool CefApplication::_PMCheckIsEvent(CefRefPtr<CefProcessMessage>& message)
{
    // TODO: redo the packet handling here once proper storage is added

    // Check is it an event message //
    if(message->GetName() == "OnEvent") {
        // Get the packet //
        sf::Packet tmppacket;

        CefRefPtr<CefListValue> args = message->GetArgumentList();

        CefRefPtr<CefBinaryValue> bval = args->GetBinary(0);

        TmpStorageForCheckIsEvent.resize(bval->GetSize());

        bval->GetData(TmpStorageForCheckIsEvent.data(), bval->GetSize(), 0);

        tmppacket.append(TmpStorageForCheckIsEvent.data(), bval->GetSize());

        // Read the data from the packet //
        Event received(tmppacket);

        // Fire it //
        NativeCoreLeviathanAPI->HandlePacket(received);

        return true;

    } else if(message->GetName() == "OnGeneric") {
        // Get the packet //
        sf::Packet tmppacket;

        CefRefPtr<CefListValue> args = message->GetArgumentList();

        CefRefPtr<CefBinaryValue> bval = args->GetBinary(0);

        TmpStorageForCheckIsEvent.resize(bval->GetSize());

        bval->GetData(TmpStorageForCheckIsEvent.data(), bval->GetSize(), 0);

        tmppacket.append(TmpStorageForCheckIsEvent.data(), bval->GetSize());

        // Read the data from the packet //
        GenericEvent received(tmppacket);

        // Fire it //
        NativeCoreLeviathanAPI->HandlePacket(received);

        return true;
    }

    return false;
}
