// ------------------------------------ //
#include "GuiView.h"

#include "CEFConversionHelpers.h"
#include "Engine.h"
#include "Events/EventHandler.h"
#include "Exceptions.h"
#include "GlobalCEFHandler.h"
#include "GuiManager.h"
#include "KeyMapping.h"
#include "LeviathanJavaScriptAsync.h"
#include "Rendering/GeometryHelpers.h"
#include "Sound/SoundDevice.h"
#include "Threading/ThreadingManager.h"
#include "Window.h"

#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"

#include "utf8.h"

#include "bsfCore/Image/BsTexture.h"

#include <SDL.h>

// #include <cstring>
using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
constexpr auto CEF_BYTES_PER_PIXEL = 4;
// When documentation of Ogre and CEF talk about RGBA it seems they actually talk about BGRA
constexpr auto BS_PIXEL_FORMAT = bs::PF_BGRA8;

DLLEXPORT View::View(GuiManager* owner, Window* window, int renderorder,
    VIEW_SECURITYLEVEL security /*= VIEW_SECURITYLEVEL_ACCESS_ALL*/) :
    Layer(owner, window, renderorder),
    ViewSecurity(security), OurAPIHandler(new LeviathanJavaScriptAsync(this))
{}

DLLEXPORT View::~View() {}
// ------------------------------------ //
DLLEXPORT bool View::Init(const std::string& filetoload, const NamedVars& headervars)
{
    // Lock us //
    GUARD_LOCK();

    // We now only create the texture once we get some data to display
    LEVIATHAN_ASSERT(bs::PixelUtil::getNumElemBytes(BS_PIXEL_FORMAT) == CEF_BYTES_PER_PIXEL,
        "texture format size has changed");

    // Now we can create the browser //
    CefWindowInfo info;

    // We use our own window things //
    // This should be using transparent painting automatically
    // TODO:
    // info.SetAsWindowless(Wind->GetNativeHandle());
    info.SetAsWindowless(0);

    // Customize the settings //
    CefBrowserSettings settings;

    // Created synchronously but the pointer is still linked there
    // Created asynchronously, the pointer will be linked in the OnAfterCreated function //
    // TODO: determine which is better
    // CefBrowserHost::CreateBrowserSync(info, this, filetoload, settings, nullptr);
    CefBrowserHost::CreateBrowser(info, this, filetoload, settings, nullptr, nullptr);

    return true;
}

DLLEXPORT void View::_DoReleaseResources()
{
    // Stop all events //
    UnRegisterAllEvents();

    // Lock us //
    GUARD_LOCK();

    if(Owner) {
        Owner->NotifyAboutLayer(RenderOrder, nullptr);
    }

    // Kill the javascript async //
    OurAPIHandler->BeforeRelease();

    // Destroy the browser first //


    // Force release so nothing can stop it //
    if(OurBrowser.get()) {
        OurBrowser->GetHost()->CloseBrowser(true);
    }

    // Release our objects //
    SAFE_DELETE(OurAPIHandler);

    // Destroy all remaining proxy targets
    ProxyedObjects.clear();

    // if(Node) {
    //     Node->destroy();
    // }

    // Material = nullptr;
    // Renderable = nullptr;

    Texture = nullptr;
    ClearDataBuffers();
    IntermediateTextureBuffer.clear();
    IntermediateTextureBuffer.shrink_to_fit();
}
// ------------------------------------ //
DLLEXPORT void View::_OnWindowResized()
{
    if(OurBrowser.get()) {

        OurBrowser->GetHost()->WasResized();
    }
}

DLLEXPORT void View::_OnFocusChanged()
{
    if(OurBrowser.get()) {

        OurBrowser->GetHost()->SendFocusEvent(OurFocus);
    }
}
// ------------------------------------ //
DLLEXPORT bool View::OnReceiveKeyInput(const SDL_Event& sdlevent, bool down, bool textinput,
    int mousex, int mousey, int specialkeymodifiers, int cefspecialkeymodifiers)
{
    CefKeyEvent key_event;
    key_event.modifiers = cefspecialkeymodifiers;

#if defined(OS_WIN)
    if(!textinput) {
        // GDK compatible key code
        int asX11KeyCode = KeyMapping::SDLKeyToX11Key(sdlevent.key.keysym);

        // Modified to work here
        // Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
        // reserved. Use of this source code is governed by a BSD-style license that
        // can be found in the LICENSE file.
        // Based on WebKeyboardEventBuilder::Build from
        // content/browser/renderer_host/input/web_input_event_builders_gtk.cc.

        KeyboardCode windows_key_code = KeyMapping::GdkEventToWindowsKeyCode(asX11KeyCode);
        key_event.windows_key_code =
            KeyMapping::GetWindowsKeyCodeWithoutLocation(windows_key_code);

        // List is the lParam of WM_KEYDOWN or WM_SYSKEYDOWN
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms646280(v=vs.85).aspx
        // Also set the repeat count to 1 here
        key_event.native_key_code = 1;

        // WM_SYSKEYDOWN detection (hopefully...)
        if(key_event.modifiers & EVENTFLAG_ALT_DOWN)
            key_event.is_system_key = true;

    } else {
        // text input
        try {
            auto begin = std::begin(sdlevent.text.text);
            key_event.windows_key_code = utf8::next(begin, begin + strlen(sdlevent.text.text));
        } catch(const utf8::exception& e) {
            LOG_ERROR(
                "Window: CEF input handling failed to convert utf8 to utf32 codepoint: " +
                std::string(e.what()));
        }

        // This is the lParam of WM_CHAR
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms646276(v=vs.85).aspx
        // Just set the repeat count to 1 here
        key_event.native_key_code = 1;
    }

#elif defined(OS_LINUX)
    if(!textinput) {

        // GDK compatible key code
        int asX11KeyCode = KeyMapping::SDLKeyToX11Key(sdlevent.key.keysym);

        // Modified to work here
        // Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
        // reserved. Use of this source code is governed by a BSD-style license that
        // can be found in the LICENSE file.
        // Based on WebKeyboardEventBuilder::Build from
        // content/browser/renderer_host/input/web_input_event_builders_gtk.cc.

        KeyboardCode windows_key_code = KeyMapping::GdkEventToWindowsKeyCode(asX11KeyCode);
        key_event.windows_key_code =
            KeyMapping::GetWindowsKeyCodeWithoutLocation(windows_key_code);

        key_event.native_key_code = windows_key_code;
        // key_event.native_key_code = asX11KeyCode;


        // if(event->keyval >= GDK_KP_Space && event->keyval <= GDK_KP_9)
        //     key_event.modifiers |= EVENTFLAG_IS_KEY_PAD;
        if(key_event.modifiers & EVENTFLAG_ALT_DOWN)
            key_event.is_system_key = true;

        // This is VKEY_RETURN = 0x0D
        if(windows_key_code == 0x0D) {
            // We need to treat the enter key as a key press of character \r.  This
            // is apparently just how webkit handles it and what it expects.
            key_event.unmodified_character = '\r';
        } else {
            key_event.unmodified_character = sdlevent.key.keysym.sym;
        }

        // If ctrl key is pressed down, then control character shall be input.
        if(key_event.modifiers & EVENTFLAG_CONTROL_DOWN) {
            key_event.character = KeyMapping::GetControlCharacter(
                windows_key_code, key_event.modifiers & EVENTFLAG_SHIFT_DOWN);
        } else {

            // key_event.character = key_event.unmodified_character;
        }
    } else {
        // text input
        try {
            auto begin = std::begin(sdlevent.text.text);
            key_event.character = utf8::next(begin, begin + strlen(sdlevent.text.text));
            key_event.unmodified_character = key_event.character;
            // VKEY_A 0x41 (this isn't used so this is just to quiet warnings)
            key_event.windows_key_code = 0x41;
            // Not needed
            // key_event.native_key_code = ;
        } catch(const utf8::exception& e) {
            LOG_ERROR(
                "Window: CEF input handling failed to convert utf8 to utf32 codepoint: " +
                std::string(e.what()));
        }
    }
#elif defined(OS_MACOSX)
#error TODO: mac version. See: cef/tests/cefclient/browser/browser_window_osr_mac.mm
    // for how the event should be structured
#else
#error Unknown platform
#endif // _WIN32

    // Skip input if unkown key
    if(key_event.windows_key_code == 0)
        return false;

    if(down) {

        if(!textinput) {
            key_event.type = KEYEVENT_RAWKEYDOWN;
            GetBrowserHost()->SendKeyEvent(key_event);
        } else {
            // Send char event //
            key_event.type = KEYEVENT_CHAR;
            GetBrowserHost()->SendKeyEvent(key_event);
        }
    } else {
        if(!textinput) {
            key_event.type = KEYEVENT_KEYUP;
            GetBrowserHost()->SendKeyEvent(key_event);
        }
    }


    // Detecting if the key press was actually used is pretty difficult and might cause lag
    // so we don't do that
    return true;
}

DLLEXPORT void View::OnMouseMove(const SDL_Event& event, bool outsidewindow)
{
    CefMouseEvent cevent;
    cevent.x = event.motion.x;
    cevent.y = event.motion.y;

    GetBrowserHost()->SendMouseMoveEvent(cevent, outsidewindow);
}

DLLEXPORT void View::OnMouseWheel(const SDL_Event& event)
{
    int x = static_cast<int>(event.wheel.x * CEF_MOUSE_SCROLL_MULTIPLIER);
    int y = static_cast<int>(event.wheel.y * CEF_MOUSE_SCROLL_MULTIPLIER);

    if(SDL_MOUSEWHEEL_FLIPPED == event.wheel.direction) {
        y *= -1;
    } else {
        x *= -1;
    }

    CefMouseEvent cevent;
    // Need to pass the current mouse position for the scroll to go to the right place
    Wind->GetRelativeMouse(cevent.x, cevent.y);
    GetBrowserHost()->SendMouseWheelEvent(cevent, x, y);
}

DLLEXPORT void View::OnMouseButton(const SDL_Event& event, bool down)
{
    CefMouseEvent cevent;
    cevent.x = event.button.x;
    cevent.y = event.button.y;

    int type = KeyMapping::GetCEFButtonFromSdlMouseButton(event.button.button);
    if(type == -1)
        return;

    cef_mouse_button_type_t btype = static_cast<cef_mouse_button_type_t>(type);

    GetBrowserHost()->SendMouseClickEvent(cevent, btype, !down, 1);
}

// ------------------------------------ //
bool View::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    // Get the rect //
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;

    Wind->GetPosition(x, y);
    Wind->GetSize(width, height);

    // Fill the rect //
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;

    return true;
}

void View::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    // This should always work //
    rect.x = rect.y = 0;

    int32_t width;
    int32_t height;
    Wind->GetSize(width, height);

    // Width and height can be retrieved from the window //
    rect.width = width;
    rect.height = height;
}

bool View::GetScreenPoint(
    CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY)
{
    // Get the point //
    int32_t posX;
    int32_t posY;
    Wind->GetPosition(posX, posY);

    screenX = viewX - posX;
    screenY = viewY - posY;

    return true;
}

bool View::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
{
    return false;
}

void View::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) {}

void View::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) {}

void View::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor,
    CursorType type, const CefCursorInfo& custom_cursor_info)
{
#ifdef _WIN32

    HWND hwnd = Wind->GetNativeHandle();

    if(!hwnd)
        return;

    // Should not do this, but whatever //
    // TODO: custom cursors
    Wind->SetWinCursor(cursor);
#else
#ifdef __linux
    Wind->SetX11Cursor(cursor);
#else
#error todo mac version
#endif

#endif // _WIN32
}

void View::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x, double y) {}
// ------------------------------------ //
void View::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
    const RectList& dirtyRects, const void* buffer, int width, int height)
{
    CEF_REQUIRE_UI_THREAD();
    Engine::Get()->AssertIfNotMainThread();

    if(dirtyRects.empty()) {

        LOG_ERROR("View: OnPaint given empty dirtyRects list");
        return;
    }

    // Lock us, just for fun //
    // As we are always on the main thread
    GUARD_LOCK();

    // if(!Node) {
    if(Texture && Texture->isDestroyed()) {
        // Released already
        LOG_WARNING("View: OnPaint called after release");
        return;
    }

    // Calculate the size of the buffer //
    size_t buffSize = width * height * CEF_BYTES_PER_PIXEL;

    if(IntermediateTextureBuffer.size() != buffSize)
        IntermediateTextureBuffer.resize(buffSize);

    switch(type) {
    case PET_POPUP: {
        LOG_ERROR("CEF PET_POPUP not handled");
        return;
    }
    case PET_VIEW: {
        // Target is Texture
        break;
    }
    default: LOG_FATAL("Unknown paint type in View: OnPaint"); return;
    }

    // Make sure our texture is large enough //
    if(Texture && (Texture->getProperties().getWidth() != static_cast<size_t>(width) ||
                      Texture->getProperties().getHeight() != static_cast<size_t>(height))) {

        Texture->destroy();
        Texture = nullptr;
        ClearDataBuffers();
        LOG_INFO("GuiView: texture size has changed");
    }

    if(!Texture) {

        LOG_INFO("GuiView: creating texture for CEF");
        NeededTextureWidth = width;
        NeededTextureHeight = height;
    }

    // Copy the data to intermediate buffer //
    const auto firstRect = dirtyRects.front();
    if(dirtyRects.size() == 1 && firstRect.x == 0 && firstRect.y == 0 &&
        firstRect.width == width && firstRect.height == height) {

        // Can directly copy the whole thing
        std::memcpy(IntermediateTextureBuffer.data(), buffer, buffSize);

    } else {

        uint32_t* destptr = reinterpret_cast<uint32_t*>(IntermediateTextureBuffer.data());
        const uint32_t* source = static_cast<const uint32_t*>(buffer);

        const size_t rowElements = width;

        for(const auto rect : dirtyRects) {

            const auto lastX = rect.x + rect.width;
            const auto lastY = rect.y + rect.height;

            for(int y = rect.y; y < lastY; ++y) {
                for(int x = rect.x; x < lastX; ++x) {

                    // Safety check. Comment out when not debugging
                    // LEVIATHAN_ASSERT(y >= 0 && y < height, "View OnPaint y out of range");
                    // LEVIATHAN_ASSERT(x >= 0 && x < width, "View OnPaint x out of range");

                    destptr[(rowElements * y) + x] = source[(y * width) + x];
                }
            }
        }
    }

    auto* bsfBuffer = GetNextDataBuffer();

    if(!bsfBuffer) {
        LOG_WARNING("GUI: View: out of texture data buffers");
        return;
    }

    // This only works with pixel formats with no padding
    LEVIATHAN_ASSERT((*bsfBuffer)->getSize() == buffSize, "CEF and BSF buffer size mismatch");

    // Copy intermediate buffer
    std::memcpy((*bsfBuffer)->getData(), IntermediateTextureBuffer.data(), buffSize);

    if(!Texture) {
        Texture = bs::Texture::create(*bsfBuffer, bs::TU_DYNAMIC);
        Owner->NotifyAboutLayer(RenderOrder, Texture);
    } else {
        Texture->writeData(*bsfBuffer, 0, 0, true);
    }
}

bs::SPtr<bs::PixelData> View::_OnNewBufferNeeded()
{
    return bs::PixelData::create(NeededTextureWidth, NeededTextureHeight, 1, BS_PIXEL_FORMAT);
}
// ------------------------------------ //
void View::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
    // A render process has crashed...
    OurBrowserSide->OnRenderProcessTerminated(browser);
}

void View::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    // A frame has failed to load content...
    LOG_ERROR("[CEF] Load error(" + std::to_string(errorCode) +
              "): " + std::string(errorText) + ", url: " + std::string(failedUrl));
}

void View::OnLoadEnd(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    // A frame has finished loading content...

    // Let's try fix some focusing issues //
    if(frame->IsMain()) {

        // TODO: this should probably be ran in child frames as well to allow text boxes to
        // work there as well but then we need some way to detect which frame sends the status
        // Run our text box focus setup
        frame->ExecuteJavaScript("Leviathan.SetupInputDetection()", "", 0);

        // Store our original focus //
        bool origfocus = OurFocus;
        // Lose focus and then gain it if we have focus //
        NotifyFocusUpdate(false);
        if(origfocus)
            NotifyFocusUpdate(true);
    }
}

void View::OnLoadStart(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{
    // A frame has started loading content...
}

void View::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    // Browser window is closed, perform cleanup...
    OurBrowser = NULL;

    OurBrowserSide->OnBeforeClose(browser);
}

void View::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    // Browser window created successfully...
    OurBrowser = browser;

    // Create messaging functionality //
    CefMessageRouterConfig config;
    config.js_query_function = "cefQuery";
    config.js_cancel_function = "cefQueryCancel";

    OurBrowserSide = CefMessageRouterBrowserSide::Create(config);

    OurBrowserSide->AddHandler(OurAPIHandler, true);
}

void View::OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
    // Update the download status...
}

void View::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback)
{
    // Specify a file path or cancel the download...
}

bool View::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level,
    const CefString& message, const CefString& source, int line)
{
    // Log a console message...
    // Allow passing to default console //
    LOG_WRITE("[CEF] " + std::string(message) + ". \n\tIn: " + std::string(source) + " (" +
              std::to_string(line) + ")");
    return false;
}

void View::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    // Update the browser window title...
}

void View::OnAddressChange(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    // Update the URL in the address bar...
}

void View::OnLoadingStateChange(
    CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    // Update UI for browser state...
}
// ------------------------------------ //
bool View::OnKeyEvent(
    CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
    // Notify the window about this //
    // We can't determine easily here if it was used. And besides this causes a round trip to
    // the gui and back
    /*if(Wind)
        Wind->ReportKeyEventAsUsed();*/
    return false;
}

DLLEXPORT CefRefPtr<CefBrowserHost> View::GetBrowserHost()
{
    if(OurBrowser.get()) {

        return OurBrowser->GetHost();
    }

    LOG_ERROR("View: GetBrowserHost: no browser host");
    return nullptr;
}

bool View::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect)
{
    OurBrowserSide->OnBeforeBrowse(browser, frame);
    return false;
}

bool View::OnQuotaRequest(CefRefPtr<CefBrowser> browser, const CefString& origin_url,
    int64 new_size, CefRefPtr<CefRequestCallback> callback)
{
    return false;
}

bool View::OnCertificateError(CefRefPtr<CefBrowser> browser, cef_errorcode_t cert_error,
    const CefString& request_url, CefRefPtr<CefSSLInfo> ssl_info,
    CefRefPtr<CefRequestCallback> callback)
{
    return false;
}

// bool View::OnBeforePluginLoad(CefRefPtr<CefBrowser> browser, const CefString& url,
//     const CefString& policy_url, CefRefPtr<CefWebPluginInfo> info)
// {
//     return false;
// }

void View::OnPluginCrashed(CefRefPtr<CefBrowser> browser, const CefString& plugin_path) {}
// ------------------------------------ //
bool View::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    // Handle IPC messages from the render process...
    if(OurBrowserSide->OnProcessMessageReceived(browser, frame, source_process, message))
        return true;

    const auto& name = message->GetName();

    if(name == "NotifyViewInputStatus") {

        InputFocused = message->GetArgumentList()->GetBool(0);
        // LOG_INFO("Setting InputFocused: " + std::to_string(InputFocused));
        return true;

    } else if(name == "NotifyViewScrollableStatus") {

        ScrollableElement = message->GetArgumentList()->GetBool(0);
        // LOG_INFO("Setting ScrollableElement: " + std::to_string(ScrollableElement));
        return true;
    }

    // TODO: check access level properly
    if(ViewSecurity == VIEW_SECURITYLEVEL_BLOCKED)
        return false;

    if(_PMCheckIsEvent(name, message))
        return true;

    // Handle custom messages //
    if(name == "Custom") {

        if(GlobalCEFHandler::HandleCustomExtensionProcessMessage(
               browser, source_process, message)) {

            return true;
        }
    }

    // And handle other stuff //
    if(name == "Play2DSoundEffect") {

        if(ViewSecurity < VIEW_SECURITYLEVEL_NORMAL)
            return true;

        Engine::Get()->GetSoundDevice()->Play2DSoundEffect(
            message->GetArgumentList()->GetString(0));
        return true;
    }

    if(name == "AudioSource") {

        if(ViewSecurity < VIEW_SECURITYLEVEL_NORMAL)
            return true;

        _HandleAudioSourceMessage(message);

        return true;
    }

    if(name == "CallGenericEvent") {

        if(ViewSecurity < VIEW_SECURITYLEVEL_ACCESS_ALL)
            return true;

        const auto& args = message->GetArgumentList();

        if(args->GetSize() < 1)
            return true;

        auto event = GenericEvent::MakeShared<GenericEvent>(args->GetString(0));

        if(args->GetSize() > 1) {
            // Parameters
            CEFDictionaryToNamedVars(args->GetDictionary(1), *event->GetVariables());
        }

        Engine::Get()->GetEventHandler()->CallEvent(event);
        return true;
    }

    if(name == "PlayCutscene") {

        if(ViewSecurity < VIEW_SECURITYLEVEL_ACCESS_ALL)
            return true;

        _HandlePlayCutsceneMessage(message);

        return true;
    }

    LOG_ERROR("View: OnProcessMessageReceived: unknown message type: " + name.ToString());

    // Not handled //
    return false;
}
// ------------------------------------ //
bool View::_PMCheckIsEvent(const CefString& name, CefRefPtr<CefProcessMessage>& message)
{
    // Check does name match something //
    if(message->GetName() == "LGeneric") {
        // Get the name of the event //
        const std::string toregister = message->GetArgumentList()->GetString(0);

        // Only add if not already added //
        auto iter = RegisteredGenerics.find(toregister);

        if(iter == RegisteredGenerics.end()) {
            // Add it //
            RegisterForEvent(toregister);
        }

        return true;

    } else if(message->GetName() == "LEvent") {
        // Get the event that we need to register for //
        EVENT_TYPE toregister = static_cast<EVENT_TYPE>(message->GetArgumentList()->GetInt(0));

        // Only add if not already added //
        auto iter = RegisteredEvents.find(toregister);

        if(iter == RegisteredEvents.end()) {
            // Add it //
            RegisterForEvent(toregister);
        }


        return true;

    } else if(message->GetName() == "LEvents") {

        // This always means "unregister all" //
        UnRegisterAllEvents();
        return true;
    }


    // Not handled //
    return false;
}
// ------------------------------------ //
void View::_HandleAudioSourceMessage(const CefRefPtr<CefProcessMessage>& message)
{
    auto args = message->GetArgumentList();

    const auto& operation = args->GetString(0);

    if(operation == "new") {

        const auto requestNumber = args->GetInt(1);
        const auto& file = args->GetString(2);
        bool looping = args->GetBool(3);
        // bool startPaused = args->GetBool(4);

        // Create object
        AudioSource::pointer source =
            Engine::Get()->GetSoundDevice()->Play2DSound(file.ToString(), looping);

        if(source) {
            ProxyedObjects.insert(std::make_pair(source->GetID(), source));
        } else {
            LOG_ERROR("GuiView: failed to create AudioSource for JavaScript proxy");
        }

        // And respond with the resulting ID
        CefRefPtr<CefProcessMessage> responseMessage =
            CefProcessMessage::Create("AudioSource");

        CefRefPtr<CefListValue> responseArgs = responseMessage->GetArgumentList();
        responseArgs->SetString(0, "new");
        responseArgs->SetInt(1, requestNumber);
        responseArgs->SetInt(2, source ? source->GetID() : -1);

        OurBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER, responseMessage);
        return;

    } else if(operation == "destroy") {

        _HandleDestroyProxyMsg(args->GetInt(1));
        return;

    } else if(operation == "Pause") {

        // Function call
        const auto requestNumber = args->GetInt(1);

        if(requestNumber != -1)
            LOG_WARNING("GuiView: request responses for Pause aren't done, request number: " +
                        std::to_string(requestNumber));

        const auto id = args->GetInt(2);

        const auto iter = ProxyedObjects.find(id);

        if(iter == ProxyedObjects.end()) {

            LOG_ERROR(
                "GuiView: didn't find proxyed object (for Pause): " + std::to_string(id));
            return;
        }

        auto* casted = dynamic_cast<AudioSource*>(iter->second.get());

        if(!casted) {
            LOG_ERROR("GuiView: proxyed object is of wrong type (for Pause), id: " +
                      std::to_string(id));
            return;
        }

        casted->Pause();
        return;
    } else if(operation == "Resume") {

        // Function call
        const auto requestNumber = args->GetInt(1);

        if(requestNumber != -1)
            LOG_WARNING("GuiView: request responses for Resume aren't done, request number: " +
                        std::to_string(requestNumber));

        const auto id = args->GetInt(2);

        const auto iter = ProxyedObjects.find(id);

        if(iter == ProxyedObjects.end()) {

            LOG_ERROR(
                "GuiView: didn't find proxyed object (for Resume): " + std::to_string(id));
            return;
        }

        auto* casted = dynamic_cast<AudioSource*>(iter->second.get());

        if(!casted) {
            LOG_ERROR("GuiView: proxyed object is of wrong type (for Resume), id: " +
                      std::to_string(id));
            return;
        }

        casted->Resume();
        return;
    }


    LOG_ERROR("Got unknown AudioSource message: " + operation.ToString());
}
// ------------------------------------ //
void View::_HandlePlayCutsceneMessage(const CefRefPtr<CefProcessMessage>& message)
{
    auto args = message->GetArgumentList();

    const auto& operation = args->GetString(0);

    if(operation == "Play") {

        const auto requestNumber = args->GetInt(1);
        const auto& file = args->GetString(2);

        // Call the cutscene playing method on our GUI manager that handles everything.
        // Our lambdas store the request number to respond properly
        Owner->PlayCutscene(file.ToString(),
            [=]() {
                // Success
                CefRefPtr<CefProcessMessage> responseMessage =
                    CefProcessMessage::Create("PlayCutscene");

                CefRefPtr<CefListValue> responseArgs = responseMessage->GetArgumentList();
                responseArgs->SetString(0, "Finished");
                responseArgs->SetInt(1, requestNumber);

                OurBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER, responseMessage);
            },
            [=](const std::string& error) {
                // Failure
                CefRefPtr<CefProcessMessage> responseMessage =
                    CefProcessMessage::Create("PlayCutscene");

                CefRefPtr<CefListValue> responseArgs = responseMessage->GetArgumentList();
                responseArgs->SetString(0, "Error");
                responseArgs->SetInt(1, requestNumber);
                responseArgs->SetString(2, error);

                OurBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER, responseMessage);
            });
        return;

    } else if(operation == "Cancel") {
        Owner->CancelCutscene();
        return;
    }

    LOG_ERROR("Got unknown PlayCutscene message: " + operation.ToString());
}
// ------------------------------------ //
void View::_HandleDestroyProxyMsg(int id)
{
    const auto iter = ProxyedObjects.find(id);

    if(iter == ProxyedObjects.end()) {

        LOG_ERROR(
            "GuiView: didn't find proxyed object for destroy message: " + std::to_string(id));
        return;
    }

    ProxyedObjects.erase(iter);
}
// ------------------------------------ //
DLLEXPORT int View::OnEvent(Event* event)
{
    // Serialize it to a packet //
    sf::Packet tmppacket;

    event->AddDataToPacket(tmppacket);

    tmppacket.getData();

    // Create the message //
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("OnEvent");

    CefRefPtr<CefListValue> args = message->GetArgumentList();
    // Set the packet as binary data //
    args->SetBinary(0, CefBinaryValue::Create(tmppacket.getData(), tmppacket.getDataSize()));

    OurBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);

    return 3;
}

DLLEXPORT int View::OnGenericEvent(GenericEvent* event)
{
    // Serialize it to a packet //
    sf::Packet tmppacket;

    event->AddDataToPacket(tmppacket);

    tmppacket.getData();

    // Create the message //
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("OnGeneric");

    CefRefPtr<CefListValue> args = message->GetArgumentList();
    // Set the packet as binary data //
    args->SetBinary(0, CefBinaryValue::Create(tmppacket.getData(), tmppacket.getDataSize()));

    OurBrowser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);

    return 3;
}
// ------------------------------------ //
DLLEXPORT void View::ToggleElement(const std::string& name)
{
    auto frame = OurBrowser->GetMainFrame();
    frame->ExecuteJavaScript("$(\"" + name + "\").toggle()", frame->GetURL(), 0);
}
