// ------------------------------------ //
#include "GuiView.h"

#include "Engine.h"
#include "Exceptions.h"
#include "GlobalCEFHandler.h"
#include "GuiManager.h"
#include "KeyMapping.h"
#include "LeviathanJavaScriptAsync.h"
#include "Rendering/GeometryHelpers.h"
#include "Sound/SoundDevice.h"
#include "Threading/ThreadingManager.h"
#include "Window.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgreItem.h"
#include "OgreMaterialManager.h"
#include "OgreMeshManager2.h"
#include "OgreSceneManager.h"
#include "OgreSubMesh2.h"
#include "OgreTechnique.h"
#include "OgreTextureManager.h"

#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"

#include "utf8.h"

#include <SDL.h>

#include <cstring>
using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
constexpr auto CEF_BYTES_PER_PIXEL = 4;

DLLEXPORT View::View(GuiManager* owner, Window* window,
    VIEW_SECURITYLEVEL security /*= VIEW_SECURITYLEVEL_ACCESS_ALL*/) :
    Layer(owner, window),
    ViewSecurity(security), OurAPIHandler(new LeviathanJavaScriptAsync(this)),
    TextureName("_ChromeOverlay_for_gui_" + Convert::ToString(ID)),
    MaterialName(TextureName + "_material")
{}

DLLEXPORT View::~View() {}
// ------------------------------------ //
DLLEXPORT bool View::Init(const std::string& filetoload, const NamedVars& headervars)
{
    // Lock us //
    GUARD_LOCK();

    // Create the Ogre texture and material first //

    int32_t width;
    int32_t height;
    Wind->GetSize(width, height);

    // Create a material and a texture that we can update //
    Texture = Ogre::TextureManager::getSingleton().createManual(TextureName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, width,
        height, 0, Ogre::PF_B8G8R8A8, Ogre::TU_DYNAMIC, nullptr, true);

    LEVIATHAN_ASSERT(
        Ogre::PixelUtil::getNumElemBytes(Ogre::PF_B8G8R8A8) == CEF_BYTES_PER_PIXEL,
        "Ogre texture size has changed");

    // Fill in some test data //
    Ogre::v1::HardwarePixelBufferSharedPtr pixelBuffer = Texture->getBuffer();

    // Lock buffer and get a target box for writing //
    pixelBuffer->lock(Ogre::v1::HardwareBuffer::HBL_DISCARD);
    const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();

    // Create a pointer to the destination //
    uint8_t* destptr = static_cast<uint8_t*>(pixelBox.data);

    // Clear it
    std::memset(destptr, 0, pixelBuffer->getSizeInBytes());

    // Unlock the buffer //
    pixelBuffer->unlock();

    Ogre::SceneManager* scene = Wind->GetOverlayScene();

    Node = scene->createSceneNode(Ogre::SCENE_STATIC);

    // This needs to be manually destroyed later
    QuadMesh = GeometryHelpers::CreateScreenSpaceQuad(TextureName + "mesh", -1, -1, 2, 2);

    // Duplicate the material and set the texture on it
    Ogre::MaterialPtr baseMaterial =
        Ogre::MaterialManager::getSingleton().getByName("GUIOverlay");

    if(!baseMaterial)
        LOG_FATAL("GuiView: GUIOverlay material doesn't exists! are the core Leviathan "
                  "materials and shaders copied?");

    Ogre::MaterialPtr customizedMaterial = baseMaterial->clone(MaterialName);

    Ogre::TextureUnitState* textureState =
        customizedMaterial->getTechnique(0)->getPass(0)->createTextureUnitState();
    textureState->setTexture(Texture);
    textureState->setHardwareGammaEnabled(true);
    customizedMaterial->compile();

    QuadMesh->getSubMesh(0)->setMaterialName(MaterialName);

    // Setup render queue for it
    // TODO: different queues for different GUIs
    scene->getRenderQueue()->setRenderQueueMode(1, Ogre::RenderQueue::FAST);

    QuadItem = scene->createItem(QuadMesh, Ogre::SCENE_STATIC);
    QuadItem->setCastShadows(false);

    // Need to edit the render queue and add it to an early one
    QuadItem->setRenderQueueGroup(1);

    // Add it
    Node->attachObject(QuadItem);

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
    CefBrowserHost::CreateBrowser(info, this, filetoload, settings, nullptr);

    return true;
}

DLLEXPORT void View::ReleaseResources()
{
    // Stop all events //
    UnRegisterAllEvents();

    // Lock us //
    GUARD_LOCK();

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

    Ogre::SceneManager* scene = Wind->GetOverlayScene();

    if(Node) {
        scene->destroySceneNode(Node);
        Node = nullptr;
    }

    if(QuadItem) {
        scene->destroyItem(QuadItem);
        QuadItem = nullptr;
    }

    if(QuadMesh) {
        Ogre::MeshManager::getSingleton().remove(QuadMesh);
        QuadMesh.reset();
    }

    Ogre::MaterialManager::getSingleton().remove(MaterialName);
    Ogre::TextureManager::getSingleton().remove(Texture);
    Texture.setNull();
}
// ------------------------------------ //
DLLEXPORT void View::NotifyWindowResized()
{
    if(OurBrowser.get()) {

        OurBrowser->GetHost()->WasResized();
    }
}

DLLEXPORT void View::NotifyFocusUpdate(bool focused)
{
    // Update our focus //
    OurFocus = focused;

    if(OurBrowser.get()) {

        OurBrowser->GetHost()->SendFocusEvent(focused);
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

    // LOG_INFO("Mouse scroll to CEF: " + std::to_string(y) + " " + std::to_string(x));
    CefMouseEvent cevent;
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

bool View::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    // This should always work //
    rect.x = rect.y = 0;

    int32_t width;
    int32_t height;
    Wind->GetSize(width, height);

    // Width and height can be retrieved from the window //
    rect.width = width;
    rect.height = height;

    return true;
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

    // Calculate the size of the buffer //
    size_t buffSize = width * height * CEF_BYTES_PER_PIXEL;

    Ogre::TexturePtr targettexture;

    switch(type) {
    case PET_POPUP: {
        LOG_ERROR("CEF PET_POPUP not handled");
        return;
    }
    case PET_VIEW: {
        targettexture = Texture;
        break;
    }
    default: LOG_FATAL("Unknown paint type in View: OnPaint"); return;
    }

    if(!targettexture) {
        LOG_WARNING(
            "View: OnPaint: Ogre texture is null (perhaps released already?), skipping");
        return;
    }

    // Make sure our texture is large enough //
    if(targettexture->getWidth() != static_cast<size_t>(width) ||
        targettexture->getHeight() != static_cast<size_t>(height)) {

        // Free resources and then change the size //
        targettexture->freeInternalResources();
        targettexture->setWidth(width);
        targettexture->setHeight(height);
        targettexture->createInternalResources();

        LOG_INFO("GuiView: recreated texture for CEF browser");
    }

    // Copy it to our texture //
    Ogre::v1::HardwarePixelBufferSharedPtr pixelBuffer = targettexture->getBuffer();

    LEVIATHAN_ASSERT(
        pixelBuffer->getSizeInBytes() == buffSize, "CEF and Ogre buffer size mismatch");

    // Copy the data over //
    const auto firstRect = dirtyRects.front();
    if(dirtyRects.size() == 1 && firstRect.x == 0 && firstRect.y == 0 &&
        firstRect.width == width && firstRect.height == height) {

        // Can directly copy the whole thing
        pixelBuffer->lock(Ogre::v1::HardwareBuffer::HBL_DISCARD);
        const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();

        uint8_t* destptr = static_cast<uint8_t*>(pixelBox.data);
        std::memcpy(destptr, buffer, buffSize);

        pixelBuffer->unlock();

    } else {

        // Lock buffer and get a target box for writing //
        pixelBuffer->lock(Ogre::v1::HardwareBuffer::HBL_NORMAL);
        const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();

        uint32_t* destptr = static_cast<uint32_t*>(pixelBox.data);
        const uint32_t* source = static_cast<const uint32_t*>(buffer);

        const size_t rowElements = pixelBox.rowPitch;

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

        // std::memset(destptr, 255, pixelBuffer->getSizeInBytes());
        // Unlock the buffer //
        pixelBuffer->unlock();
    }

    // // Save render result
    // Ogre::Image img;
    // Texture->convertToImage(img);
    // static std::atomic<int> spamnumber = 0;
    // img.save("Test" + std::to_string(++spamnumber) + ".png");
}
// ------------------------------------ //
void View::OnProtocolExecution(
    CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)
{
    // Handle execution of external protocols...
}

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

bool View::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    const CefString& target_url, const CefString& target_frame_name,
    CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture,
    const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access)
{
    // Allow or block popup windows, customize popup window creation...
    return false;
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

CefRequestHandler::ReturnValue View::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request,
    CefRefPtr<CefRequestCallback> callback)
{
    return RV_CONTINUE;
}

CefRefPtr<CefResourceHandler> View::GetResourceHandler(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request)
{
    return NULL;
}

void View::OnResourceRedirect(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response, CefString& new_url)
{}

bool View::GetAuthCredentials(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    bool isProxy, const CefString& host, int port, const CefString& realm,
    const CefString& scheme, CefRefPtr<CefAuthCallback> callback)
{
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
bool View::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    // Handle IPC messages from the render process...
    if(OurBrowserSide->OnProcessMessageReceived(browser, source_process, message))
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
        bool startPaused = args->GetBool(4);

        // Create object
        AudioSource::pointer source = Engine::Get()->GetSoundDevice()->Play2DSound(
            file.ToString(), looping, startPaused);

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

        OurBrowser->SendProcessMessage(PID_RENDERER, responseMessage);
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
    } else if(operation == "Play2D") {

        // Function call
        const auto requestNumber = args->GetInt(1);

        if(requestNumber != -1)
            LOG_WARNING("GuiView: request responses for Play2D aren't done, request number: " +
                        std::to_string(requestNumber));

        const auto id = args->GetInt(2);

        const auto iter = ProxyedObjects.find(id);

        if(iter == ProxyedObjects.end()) {

            LOG_ERROR(
                "GuiView: didn't find proxyed object (for Play2D): " + std::to_string(id));
            return;
        }

        auto* casted = dynamic_cast<AudioSource*>(iter->second.get());

        if(!casted) {
            LOG_ERROR("GuiView: proxyed object is of wrong type (for Play2D), id: " +
                      std::to_string(id));
            return;
        }

        casted->Play2D();
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

                OurBrowser->SendProcessMessage(PID_RENDERER, responseMessage);
            },
            [=](const std::string& error) {
                // Failure
                CefRefPtr<CefProcessMessage> responseMessage =
                    CefProcessMessage::Create("PlayCutscene");

                CefRefPtr<CefListValue> responseArgs = responseMessage->GetArgumentList();
                responseArgs->SetString(0, "Error");
                responseArgs->SetInt(1, requestNumber);
                responseArgs->SetString(2, error);

                OurBrowser->SendProcessMessage(PID_RENDERER, responseMessage);
            });
        return;
    } else if(operation == "Cancel") {
        Owner->CancelCutscene();
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

    OurBrowser->SendProcessMessage(PID_RENDERER, message);

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

    OurBrowser->SendProcessMessage(PID_RENDERER, message);

    return 3;
}
// ------------------------------------ //
DLLEXPORT void View::ToggleElement(const std::string& name)
{
    auto frame = OurBrowser->GetMainFrame();
    frame->ExecuteJavaScript("$(\"" + name + "\").toggle()", frame->GetURL(), 0);
}
