// ------------------------------------ //
#include "GuiView.h"

#include "Engine.h"
#include "Exceptions.h"
#include "GlobalCEFHandler.h"
#include "Handlers/IDFactory.h"
#include "LeviathanJavaScriptAsync.h"
#include "Rendering/GeometryHelpers.h"
#include "Threading/ThreadingManager.h"
#include "Window.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgreItem.h"
#include "OgreManualObject.h"
#include "OgreMaterialManager.h"
#include "OgreMeshManager2.h"
#include "OgreSceneManager.h"
#include "OgreSubMesh2.h"
#include "OgreTechnique.h"
#include "OgreTextureManager.h"

#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"

#include <cstring>
using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
constexpr auto CEF_BYTES_PER_PIXEL = 4;

DLLEXPORT View::View(GuiManager* owner, Window* window,
    VIEW_SECURITYLEVEL security /*= VIEW_SECURITYLEVEL_ACCESS_ALL*/) :
    ID(IDFactory::GetID()),
    ViewSecurity(security), Wind(window), Owner(owner),
    OurAPIHandler(new LeviathanJavaScriptAsync(this))
{}

DLLEXPORT View::~View() {}
// ------------------------------------ //
DLLEXPORT bool View::Init(const std::string& filetoload, const NamedVars& headervars)
{
    // Lock us //
    GUARD_LOCK();

    // Create the Ogre texture and material first //
    TextureName = "_ChromeOverlay_for_gui_" + Convert::ToString(ID);
    MaterialName = TextureName + "_material";

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

    // TODO: find a way for the species to manage this to
    // avoid having tons of materials Maybe Use the species's
    // name instead. and let something like the
    // SpeciesComponent create and destroy this
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
    SetClassLongPtr(hwnd, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
    SetCursor(cursor);
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
        // work there as well 
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

    return NULL;
}

bool View::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request, bool is_redirect)
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
    if(_PMCheckIsEvent(message))
        return true;

	const auto& name = message->GetName();

	if (name == "NotifyViewInputStatus") {

		const bool inputFocused = message->GetArgumentList()->GetBool(0);

		InputFocused = inputFocused;
		// LOG_INFO("Setting InputFocused: " + std::to_string(InputFocused));
		return true;
	}

    // Not handled //
    return false;
}

bool View::_PMCheckIsEvent(CefRefPtr<CefProcessMessage>& message)
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
