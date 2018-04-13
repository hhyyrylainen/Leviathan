// ------------------------------------ //
#include "GuiCEFApplication.h"

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
void CefApplication::OnContextInitialized() {}
// ------------------------------------ //
void CefApplication::OnBeforeCommandLineProcessing(
    const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
    // Check if we want to change something
}

void CefApplication::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) {}

void CefApplication::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
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

        CustomExtensions.push_back(std::make_unique<CustomExtension>(name, contents, factory));
    }
}

bool CefApplication::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    // Handle IPC messages from the browser process...

    if(RendererRouter->OnProcessMessageReceived(browser, source_process, message))
        return true;

    if(_PMCheckIsEvent(message))
        return true;

    // Not handled //
    return false;
}

DLLEXPORT void CefApplication::RegisterCustomExtension(
    std::unique_ptr<CustomExtension>&& extension)
{
    CustomExtensions.push_back(std::move(extension));
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
    OurBrowser->SendProcessMessage(PID_BROWSER, message);
}

void CefApplication::StopListeningForEvents()
{
    // Tell that we no longer want any //
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("LEvents");

    CefRefPtr<CefListValue> args = message->GetArgumentList();
    // Indicate that we want to stop receiving all messages //
    args->SetBool(0, true);

    OurBrowser->SendProcessMessage(PID_BROWSER, message);
}

bool CefApplication::_PMCheckIsEvent(CefRefPtr<CefProcessMessage>& message)
{
    // Check is it an event message //
    if(message->GetName() == "OnEvent") {
        // Get the packet //
        sf::Packet tmppacket;

        CefRefPtr<CefListValue> args = message->GetArgumentList();

        CefRefPtr<CefBinaryValue> bval = args->GetBinary(0);

        void* tmpdatablock = new char[bval->GetSize()];

        bval->GetData(tmpdatablock, bval->GetSize(), 0);

        tmppacket.append(tmpdatablock, bval->GetSize());

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

        void* tmpdatablock = new char[bval->GetSize()];

        bval->GetData(tmpdatablock, bval->GetSize(), 0);

        tmppacket.append(tmpdatablock, bval->GetSize());

        // Read the data from the packet //
        GenericEvent received(tmppacket);

        // Fire it //
        NativeCoreLeviathanAPI->HandlePacket(received);

        return true;
    }

    return false;
}
