// ------------------------------------ //
#include "Graphics.h"

#include "Application/AppDefine.h"
#include "Application/CrashHandler.h"
#include "Application/GameConfiguration.h"
#include "Common/StringOperations.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GUIOverlayRenderer.h"
#include "GeometryHelpers.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Threading/ThreadingManager.h"
#include "Window.h"
#include "WindowRenderingResources.h"

#include "BsApplication.h"
#include "Components/BsCCamera.h"
#include "CoreThread/BsCoreThread.h"
#include "Importer/BsImporter.h"
#include "Resources/BsEngineShaderIncludeHandler.h"
#include "Resources/BsResources.h"
#include "Scene/BsSceneObject.h"
#include "bsfCore/Animation/BsAnimationClip.h"
#include "bsfCore/Material/BsMaterial.h"
#include "bsfCore/Material/BsShaderManager.h"
#include "bsfCore/Mesh/BsMesh.h"
#include "bsfCore/RenderAPI/BsRenderAPI.h"
#include "bsfCore/Resources/BsResourceManifest.h"


#if GL_SUPPORTED
#include "DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#endif

#if VULKAN_SUPPORTED
#include "DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#endif

#include "DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h"

// part of the hack
#undef LOG_ERROR

#include "DiligentCore/Common/interface/RefCntAutoPtr.h"

// hack workaround
#undef LOG_ERROR
#define LOG_ERROR(x) Logger::Get()->Error(x);


#if defined(__linux__)
#if VULKAN_SUPPORTED
#include "XLibInclude.h"
#include <xcb/xcb.h>
#endif
#endif


#include <SDL.h>
#include <SDL_syswm.h>

#include <future>
#include <regex>

#ifdef __linux
#include "XLibInclude.h"
#endif

#include <filesystem>

using namespace Leviathan;
// ------------------------------------ //
namespace Leviathan {
#if VULKAN_SUPPORTED
struct XCBInfo {
    xcb_connection_t* connection = nullptr;
    uint32_t window = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    xcb_intern_atom_reply_t* atom_wm_delete_window = nullptr;
};
#endif
// ------------------------------------ //
} // namespace Leviathan
// ------------------------------------ //
void DiligentErrorCallback(Diligent::DebugMessageSeverity severity,
    const Diligent::Char* message, const char* function, const char* file, int line)
{
    // Forward to global logger if one exists
    auto log = Logger::Get();

    if(log) {
        switch(severity) {
        case Diligent::DebugMessageSeverity::Info: {
            log->Write(std::string("[INFO][DILIGENT] ") + message);
            return;
        }
        case Diligent::DebugMessageSeverity::Warning: {
            log->Write(std::string("[WARNING][DILIGENT] ") + message);
            return;
        }
        case Diligent::DebugMessageSeverity::Error: {
            log->Write(std::string("[ERROR][DILIGENT] ") + message);
            return;
        }
        case Diligent::DebugMessageSeverity::FatalError:
        default: {
            log->Fatal(std::string("[DILIGENT] ") + message + ", at: " + file + "(" +
                       std::to_string(line) + ")");
            LOG_FATAL("fatal diligent message returned from Log::Fatal");
            return;
        }
        }
    }

    // TODO: what to do when can't log
}

class LeviathanBSFShaderIncludeHandler : public bs::EngineShaderIncludeHandler {
public:
    virtual bs::HShaderInclude findInclude(const bs::String& name) const override
    {
        // If the file path is valid just pass it as is
        const std::string converted(name.c_str(), name.size());
        if(FileSystem::FileExists(converted))
            return bs::EngineShaderIncludeHandler::findInclude(name);

        // We resolve the path and then give it to bsf
        std::string searched = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT,
            StringOperations::RemoveExtension<std::string>(converted),
            StringOperations::GetExtension<std::string>(converted), true);

        if(searched.empty()) {
            if(name.find("$ENGINE$") == bs::String::npos) {
                LOG_WARNING(
                    "LeviathanBSFShaderIncludeHandler: could not locate file anywhere: " +
                    converted);
            }

            return bs::EngineShaderIncludeHandler::findInclude(name);
        }

        return bs::EngineShaderIncludeHandler::findInclude(
            bs::String(searched.c_str(), searched.size()));
    }
};

class LeviathanBSFApplication : public bs::Application {
public:
    LeviathanBSFApplication(const bs::START_UP_DESC& desc) : bs::Application(desc) {}

    bs::SPtr<bs::IShaderIncludeHandler> getShaderIncludeHandler() const override
    {
        return bs::bs_shared_ptr_new<LeviathanBSFShaderIncludeHandler>();
    }

    bs::SPtr<GUIOverlayRenderer> GUIRenderer;
};


struct Graphics::Implementation {

    // Implementation(const bs::START_UP_DESC& desc) : Description(desc) {}
    Implementation() {}

    template<class T>
    auto LoadResource(const bs::String& path)
    {
        auto asset = bs::gResources().load<T>(path);

        if(!asset) {
            LOG_ERROR(std::string("Graphics: loading asset failed: ") + path.c_str());
            return decltype(asset)(nullptr);
        }

        if(RegisteredAssets.find(path) != RegisteredAssets.end()) {
            // Already registered, fine to just return
            return asset;
        }

        // Was not registered.

        bs::gResources().getResourceManifest("Default")->registerResource(
            asset.getUUID(), path);

        RegisteredAssets.insert(path);
        return asset;
    }

    std::unordered_set<bs::String> RegisteredAssets;

    bs::START_UP_DESC Description;
    LeviathanBSFApplication* OurApp = nullptr;

    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> RenderDevice;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> ImmediateContext;
    // Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pPSO;
};

#ifdef __linux
bool HasX11Error = false;

int LeviathanX11ErrorHandler(Display* display, XErrorEvent* event)
{
    std::stringstream str;
    str << "X error received: "
        << "type " << event->type << ", "
        << "serial " << event->serial << ", "
        << "error_code " << static_cast<int>(event->error_code) << ", "
        << "request_code " << static_cast<int>(event->request_code) << ", "
        << "minor_code " << static_cast<int>(event->minor_code);

    LOG_ERROR(str.str());
    HasX11Error = true;
    return 0;
}
#endif

DLLEXPORT Graphics::Graphics() {}

Graphics::~Graphics()
{
    LEVIATHAN_ASSERT(!Initialized, "Graphics not released before destructor");
}
// ------------------------------------------- //
bool Graphics::Init(AppDef* appdef)
{
    // StartUp SDL //
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {

        LOG_ERROR("Graphics: Init: SDL init failed, error: " + std::string(SDL_GetError()));
        return false;
    }

    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

    int displays = SDL_GetNumVideoDisplays();

    LOG_INFO("SDL: display count: " + Convert::ToString(displays));

    // Get display positions
    std::vector<SDL_Rect> displayBounds;

    for(int i = 0; i < displays; i++) {

        displayBounds.push_back(SDL_Rect());

        SDL_GetDisplayBounds(i, &displayBounds.back());

        const char* nameptr = SDL_GetDisplayName(i);

        const auto name = nameptr ? std::string(nameptr) : std::string("unnamed");

        // Video modes //
        int videomodecount = SDL_GetNumDisplayModes(i);

        std::vector<std::string> videomodes;

        for(int a = 0; a < videomodecount; a++) {


            SDL_DisplayMode mode = {SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};

            if(SDL_GetDisplayMode(i, a, &mode) == 0) {

                videomodes.push_back(Convert::ToString(SDL_BITSPERPIXEL(mode.format)) +
                                     " bpp " + Convert::ToString(mode.w) + "x" +
                                     Convert::ToString(mode.h) + " at " +
                                     Convert::ToString(mode.refresh_rate) + "Hz");
            }
        }


        LOG_INFO("Display(" + Convert::ToString(i) + ", " + name + "): top left: (" +
                 Convert::ToString(displayBounds.back().x) + ", " +
                 Convert::ToString(displayBounds.back().y) +
                 ") size: " + Convert::ToString(displayBounds.back().w) + "x" +
                 Convert::ToString(displayBounds.back().h));

        // LOG_INFO("Supported modes(" + Convert::ToString(videomodes.size()) + "): ");
        // for(const auto& mode : videomodes){

        //     LOG_WRITE(" " + mode);
        // }
    }

    PrintDetectedSystemInformation();

    // if(!InitializeBSF(appdef)) {

    //     Logger::Get()->Error("Graphics: Init: failed to create bs::framework renderer");
    //     return false;
    // }

    if(!InitializeDiligent(appdef)) {

        LOG_ERROR("Graphics: Init: failed to initialize diligent engine");
        return false;
    }

#ifdef __linux
    // Set X11 error handler to not crash on non-fatal errors
    XSetErrorHandler(LeviathanX11ErrorHandler);
#endif

    Initialized = true;
    return true;
}

DLLEXPORT void Graphics::Release()
{
    if(Initialized) {

        ShutdownBSF();
        ShutdownDiligent();

        SDL_Quit();
    }

    Initialized = false;
    FirstWindowCreated = false;
    Pimpl.reset();
}
// ------------------------------------ //
void Graphics::PrintDetectedSystemInformation()
{
    // There is no longer any graphics engine printing this info so we need to do this
    // ourselves
    LOG_WRITE("TODO: CPU and GPU system info printing redo");

    std::stringstream sstream;

    sstream << "Start of graphics system information:\n"
            << "// ------------------------------------ //\n";

    // This code is adapted from bs::Debug::saveTextLog

    sstream << "BSF version: " << BS_VERSION_MAJOR << "." << BS_VERSION_MINOR << "."
            << BS_VERSION_PATCH << "\n";

    sstream << "This build is for: ";
#ifdef _WIN32
    sstream << "windows";
#elif defined(__linux)
    sstream << "linux";
#else
    sstream << "unknown";
#endif

    sstream << "\n";

    bs::SystemInfo systemInfo = bs::PlatformUtility::getSystemInfo();
    sstream << "OS version: " << systemInfo.osName << " "
            << (systemInfo.osIs64Bit ? "64-bit" : "32-bit") << "\n";
    sstream << "CPU information:\n";
    sstream << "CPU vendor: " << systemInfo.cpuManufacturer << "\n";
    sstream << "CPU name: " << systemInfo.cpuModel << "\n";
    sstream << "CPU clock speed: " << systemInfo.cpuClockSpeedMhz << "Mhz\n";
    sstream << "CPU core count: " << systemInfo.cpuNumCores << "\n";

    sstream << "\n";
    sstream << "GPU List:\n";

    // NOTE: this doesn't work on my Linux computer (returns an empty list)
    if(systemInfo.gpuInfo.numGPUs == 1)
        sstream << "GPU: " << systemInfo.gpuInfo.names[0] << "\n";
    else {
        for(bs::UINT32 i = 0; i < systemInfo.gpuInfo.numGPUs; i++)
            sstream << "GPU #" << i << ": " << systemInfo.gpuInfo.names[i] << "\n";
    }

    sstream << "// ------------------------------------ //";

    LOG_INFO(sstream.str());
}

DLLEXPORT std::string Graphics::GetUsedAPIName() const
{
    switch(SelectedAPI) {
    case GRAPHICS_API::D3D11: {
        return "DirectX11";
    }
    case GRAPHICS_API::D3D12: {
        return "DirectX12";
    }
    case GRAPHICS_API::OpenGL: {
        return "OpenGL";
    }
    case GRAPHICS_API::OpenGLES: {
        return "OpenGLES";
    }
    case GRAPHICS_API::Vulkan: {
        return "Vulkan";
    }
    case GRAPHICS_API::Metal: {
        return "Metal";
    }
    }

    return "error";
}
// ------------------------------------ //
bool Graphics::SelectPreferredGraphicsAPI(AppDef* appdef)
{
    const auto defaultRenderer = "Vulkan";

    std::string renderAPI;
    ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
        appdef->GetValues(), "RenderAPI", renderAPI, defaultRenderer);

    LOG_INFO("Graphics: preferred rendering API: '" + renderAPI + "'");

    const std::regex vulkan(
        "Vulkan", std::regex_constants::ECMAScript | std::regex_constants::icase);
    const std::regex opengl(
        "OpenGL", std::regex_constants::ECMAScript | std::regex_constants::icase);

    const std::regex openglES(
        "OpenGLES", std::regex_constants::ECMAScript | std::regex_constants::icase);

    const std::regex metal(
        "Metal", std::regex_constants::ECMAScript | std::regex_constants::icase);

    const std::regex D3D11(
        "DirectX11", std::regex_constants::ECMAScript | std::regex_constants::icase);

    const std::regex D3D12(
        "DirectX12", std::regex_constants::ECMAScript | std::regex_constants::icase);

    if(std::regex_match(renderAPI, vulkan)) {
        SelectedAPI = GRAPHICS_API::Vulkan;
    } else if(std::regex_match(renderAPI, openglES)) {
        SelectedAPI = GRAPHICS_API::OpenGLES;
    } else if(std::regex_match(renderAPI, opengl)) {
        SelectedAPI = GRAPHICS_API::OpenGL;
    } else if(std::regex_match(renderAPI, metal)) {
        SelectedAPI = GRAPHICS_API::Metal;
    } else if(std::regex_match(renderAPI, D3D11)) {
        SelectedAPI = GRAPHICS_API::D3D11;
    } else if(std::regex_match(renderAPI, D3D12)) {
        SelectedAPI = GRAPHICS_API::D3D12;
    } else {
        LOG_ERROR("Graphics: unknown render API selected: " + renderAPI);
        return false;
    }

    return true;
}

bool Graphics::CheckAndInitializeSelectedAPI()
{
    LOG_INFO("Graphics: selected render API: " + GetUsedAPIName());

    switch(SelectedAPI) {
#if D3D11_SUPPORTED
    case GRAPHICS_API::D3D11: {
        DEBUG_BREAK;

        // Fallback for this is GRAPHICS_API::OpenGL
        break;
    }
#endif // D3D11_SUPPORTED

#if D3D12_SUPPORTED
    case GRAPHICS_API::D3D12: {
        DEBUG_BREAK;

        // Fallback for this is GRAPHICS_API::D3D11
        break;
    }
#endif // D3D12_SUPPORTED

#if GL_SUPPORTED
    case GRAPHICS_API::OpenGL: {
        // This has no initialization actions
        // This has no fallback
        break;
    }
#endif // GL_SUPPORTED

#if GLES_SUPPORTED
    case GRAPHICS_API::OpenGLES: {
        DEBUG_BREAK;
        break;
    }
#endif // GLES_SUPPORTED

#if VULKAN_SUPPORTED
    case GRAPHICS_API::Vulkan: {
        LOG_INFO("Attempting to create a vulkan device and context");

        Diligent::EngineVkCreateInfo EngVkAttribs;

        EngVkAttribs.DebugMessageCallback = &DiligentErrorCallback;

        // TODO: configure validation
        // #ifdef _DEBUG
        EngVkAttribs.EnableValidation = true;
        // #endif
        auto* pFactoryVk = Diligent::GetEngineFactoryVk();
        pFactoryVk->CreateDeviceAndContextsVk(
            EngVkAttribs, &Pimpl->RenderDevice, &Pimpl->ImmediateContext);

        if(!Pimpl->RenderDevice || !Pimpl->ImmediateContext) {
            LOG_ERROR("Graphics: vulkan device creation failed, using fallback");

#if D3D12_SUPPORTED
            SelectedAPI = GRAPHICS_API::D3D12;
            return CheckAndInitializeSelectedAPI();
#endif
#if GL_SUPPORTED
            SelectedAPI = GRAPHICS_API::OpenGL;
            return CheckAndInitializeSelectedAPI();
#endif

            LOG_ERROR("No fallback found");
            return false;
        }

        LOG_INFO("Graphics: vulkan device and context created");
        break;
    }
#endif // VULKAN_SUPPORTED

#if METAL_SUPPORTED
    case GRAPHICS_API::Metal: {
        DEBUG_BREAK;
        break;
    }
#endif // METAL_SUPPORTED
    default:
        LOG_ERROR("Graphics: selected API is unavailable on current platform");
        return false;
    }

    return true;
}

bool Graphics::InitializeDiligent(AppDef* appdef)
{
    if(!SelectPreferredGraphicsAPI(appdef))
        return false;

    Pimpl = std::make_unique<Implementation>();

    return CheckAndInitializeSelectedAPI();
}

void Graphics::ShutdownDiligent()
{
    if(Pimpl->ImmediateContext) {
        Pimpl->ImmediateContext->Flush();
    }
}
// ------------------------------------------- //
std::unique_ptr<WindowRenderingResources> Graphics::RegisterCreatedWindow(Window& window)
{
    LOG_INFO("Graphics: creating rendering resources for a window");

    Diligent::SwapChainDesc SCDesc;
    // Uint32        NumDeferredCtx = 0;

    auto windowResources = std::make_unique<WindowRenderingResources>();

    switch(SelectedAPI) {
#if D3D11_SUPPORTED
    case GRAPHICS_API::D3D11: {
        DEBUG_BREAK;
        break;
    }
#endif // D3D11_SUPPORTED

#if D3D12_SUPPORTED
    case GRAPHICS_API::D3D12: {
        DEBUG_BREAK;
        break;
    }
#endif // D3D12_SUPPORTED

#if GL_SUPPORTED
    case GRAPHICS_API::OpenGL: {
        auto* pFactoryOpenGL = Diligent::GetEngineFactoryOpenGL();

        Diligent::EngineGLCreateInfo CreationAttribs;
        CreationAttribs.pNativeWndHandle = reinterpret_cast<void*>(window.GetNativeHandle());

#if defined(__linux__)
        CreationAttribs.pDisplay = reinterpret_cast<Display*>(window.GetWindowXDisplay());
#endif

        LEVIATHAN_ASSERT(
            !Pimpl->RenderDevice, "opengl multiple windows probably needs fixing");

        pFactoryOpenGL->CreateDeviceAndSwapChainGL(CreationAttribs, &Pimpl->RenderDevice,
            &Pimpl->ImmediateContext, SCDesc, &windowResources->WindowsSwapChain);

        if(!Pimpl->RenderDevice || !Pimpl->ImmediateContext) {
            LOG_FATAL("Graphics: opengl device creation failed");
            return nullptr;
        }

        if(!&windowResources->WindowsSwapChain) {
            LOG_FATAL("OpenGL swapchain creation failed");
            return nullptr;
        }

        break;
    }
#endif // GL_SUPPORTED

#if GLES_SUPPORTED
    case GRAPHICS_API::OpenGLES: {
        DEBUG_BREAK;
        break;
    }
#endif // GLES_SUPPORTED

#if VULKAN_SUPPORTED
    case GRAPHICS_API::Vulkan: {
        LOG_INFO("Attempting to create a vulkan swap chain");

        XCBInfo info;

        int32_t width, height;
        window.GetSize(width, height);
        info.width = width;
        info.height = height;
        info.window = window.GetNativeHandle();
        // info.atom_wm_delete_window

#if defined(__linux__)
        // Convert the SDL's window connection to xcb connection that is needed by diligent
        info.connection =
            XGetXCBConnection(reinterpret_cast<Display*>(window.GetWindowXDisplay()));
#endif

        auto* pFactoryVk = Diligent::GetEngineFactoryVk();

        pFactoryVk->CreateSwapChainVk(Pimpl->RenderDevice, Pimpl->ImmediateContext, SCDesc,
            &info, &windowResources->WindowsSwapChain);

        if(!&windowResources->WindowsSwapChain) {
            LOG_FATAL("Vulkan swapchain creation failed. TODO: it would still technically be "
                      "possible to fallback to opengl");
            return nullptr;
        }

        LOG_INFO("Graphics: vulkan swap chain created for window");
        break;
    }
#endif // VULKAN_SUPPORTED

#if METAL_SUPPORTED
    case GRAPHICS_API::Metal: {
        DEBUG_BREAK;
        break;
    }
#endif // METAL_SUPPORTED
    default: LOG_FATAL("Graphics: invalid graphics API selected in window creation");
    }

    return windowResources;

    /*
        if(FirstWindowCreated) {
            // Register secondary window

            bs::RENDER_WINDOW_DESC windowDesc;

            windowDesc.depthBuffer = true;

            int multiSample;

            ObjectFileProcessor::LoadValueFromNamedVars<int>(
                Engine::Get()->GetDefinition()->GetValues(), "WindowMultiSampleCount",
    multiSample, 1);

            windowDesc.multisampleCount = multiSample;
            // windowDesc.multisampleHint = "";
            // Not sure what all settings need to be copied
            windowDesc.fullscreen = window.IsFullScreen() false;
            windowDesc.vsync = false;

            int32_t width, height;
            window.GetSize(width, height);
            windowDesc.videoMode = bs::VideoMode(
                width, height, Pimpl->Description.primaryWindowDesc.videoMode.refreshRate, 0);

    #ifdef _WIN32
            windowDesc.platformSpecific["externalWindowHandle"] =
                std::to_string((uint64_t)window.GetNativeHandle());
    #else
            windowDesc.platformSpecific["externalWindowHandle"] =
                std::to_string(window.GetNativeHandle());

            windowDesc.platformSpecific["externalDisplay"] =
                std::to_string(window.GetWindowXDisplay());
    #endif

            auto window = bs::RenderWindow::create(windowDesc);

            if(!window)
                LOG_FATAL("Failed to create additional BSF window");

            return window;

        } else {
            // Finish initializing graphics
            FirstWindowCreated = true;
            LOG_INFO("Graphics: doing bs::framework initialization after creating first
    window");

            // Setup first window properties
            auto& windowDesc = Pimpl->Description.primaryWindowDesc;
            windowDesc.depthBuffer = true;

            int multiSample;

            ObjectFileProcessor::LoadValueFromNamedVars<int>(
                Engine::Get()->GetDefinition()->GetValues(), "WindowMultiSampleCount",
    multiSample, 1);

            windowDesc.multisampleCount = multiSample;
            // windowDesc.multisampleHint = "";
            // Not sure what all settings need to be copied
            windowDesc.fullscreen = window.IsFullScreen() false;
            windowDesc.vsync = false;

            // Fill video mode info from SDL
            SDL_DisplayMode dm;
            if(SDL_GetDesktopDisplayMode(0, &dm) != 0) {
                LOG_ERROR("Graphics: RegisterCreatedWindow: failed to get desktop display
    mode:" + std::string(SDL_GetError())); return nullptr;
            }

            int32_t width, height;
            window.GetSize(width, height);
            windowDesc.videoMode = bs::VideoMode(width, height, dm.refresh_rate, 0);

    #ifdef _WIN32
            windowDesc.platformSpecific["externalWindowHandle"] =
                std::to_string((uint64_t)window.GetNativeHandle());
    #else
            windowDesc.platformSpecific["externalWindowHandle"] =
                std::to_string(window.GetNativeHandle());

            windowDesc.platformSpecific["externalDisplay"] =
                std::to_string(window.GetWindowXDisplay());
    #endif

            bs::Application::startUp<LeviathanBSFApplication>(Pimpl->Description);

            Pimpl->OurApp =
                static_cast<LeviathanBSFApplication*>(bs::CoreApplication::instancePtr());

            bs::SPtr<bs::RenderWindow> bsWindow =
                bs::CoreApplication::instance().getPrimaryWindow();

            LEVIATHAN_ASSERT(bsWindow, "window creation failed");

            // Notify engine to register threads to work with Ogre //
            // Engine::GetEngine()->_NotifyThreadsRegisterOgre();

            // TODO: loading this causes a failure in bs::Material::createParamsSet
            // constexpr auto GUI_SHADER_PATH =
            // "Data/Shaders/CoreShaders/ScreenSpaceGUI.bsl.asset";

            // LEVIATHAN_ASSERT(
            //     std::filesystem::exists(GUI_SHADER_PATH), "Core GUI shader asset is
    missing");

            // auto shader = Pimpl->LoadResource<bs::Shader>(
            //     std::filesystem::absolute(GUI_SHADER_PATH).string().c_str());

            // if(!shader)
            //     LEVIATHAN_ASSERT(false, "Loading Core GUI shader asset failed");

            auto shader =
                bs::gImporter().import<bs::Shader>("Data/Shaders/CoreShaders/ScreenSpaceGUI.bsl");

            auto material = bs::Material::create(shader);

            Pimpl->OurApp->GUIRenderer =
                bs::RendererExtension::create<GUIOverlayRenderer>(GUIOverlayInitializationData{
                    GeometryHelpers::CreateScreenSpaceQuad(-1, -1, 2,
    2)->GetInternal()->getCore(), material->getCore()});

            Pimpl->OurApp->beginMainLoop();
            return bsWindow;
        } */
}

bool Graphics::UnRegisterWindow(Window& window)
{
    if(Pimpl) {
        Pimpl->OurApp->waitUntilFrameFinished();
    }

    if(window.GetBSFWindow() == bs::CoreApplication::instance().getPrimaryWindow()) {
        LOG_INFO("Graphics: primary window is closing, hiding it instead until shutdown");
        return true;
    }

    // TODO: additional window unregister
    return false;
}

void Graphics::ShutdownBSF()
{
    LEVIATHAN_ASSERT(Pimpl, "ShutdownBSF called when it isn't valid to do so");

    // One more frame needs to be rendered here to not crash if the outer main loop has
    // done stuff since the last frame.
    Pimpl->OurApp->runMainLoopFrame();
    Pimpl->OurApp->endMainLoop();
    Pimpl->OurApp->GUIRenderer = nullptr;

    bs::Application::shutDown();
    Pimpl->OurApp = nullptr;
}
// ------------------------------------ //
DLLEXPORT bool Graphics::Frame()
{
    // Logic for this frame is already ready, just tell bsf to render once
    Pimpl->OurApp->runMainLoopFrame();

    // At this point the frame render operation is happening on the BSF core thread, but it is
    // safe to use non-core thread objects normally, only in special cases do we need to wait
    // for a frame to end
    return true;
}
// ------------------------------------ //
DLLEXPORT void Graphics::UpdateShownOverlays(
    bs::RenderTarget& target, const std::vector<bs::SPtr<bs::Texture>>& overlays)
{
    const auto targetRenderTarget = reinterpret_cast<uint64_t>(target.getCore().get());

    std::vector<bs::SPtr<bs::ct::Texture>> coreVersion;
    coreVersion.reserve(overlays.size());

    std::transform(overlays.begin(), overlays.end(), std::back_inserter(coreVersion),
        [](const bs::SPtr<bs::Texture>& item) { return item->getCore(); });

    std::weak_ptr<GUIOverlayRenderer> rendererExtension = Pimpl->OurApp->GUIRenderer;

    bs::gCoreThread().queueCommand(
        [rendererExtension, targetRenderTarget, coreVersion = std::move(coreVersion)]() {
            const auto locked = rendererExtension.lock();
            if(locked)
                locked->UpdateShownOverlays(targetRenderTarget, coreVersion);
        });
}

DLLEXPORT bool Graphics::IsVerticalUVFlipped() const
{
    const auto capabilities = bs::ct::RenderAPI::instance().getCapabilities(0);

    return capabilities.conventions.ndcYAxis != bs::Conventions::Axis::Down;
}
// ------------------------------------ //
// Resource loading helpers
DLLEXPORT bs::HShader Graphics::LoadShaderByName(const std::string& name)
{
    // TODO: .asset detection

    auto file = FileSystem::Get()->SearchForFile(Leviathan::FILEGROUP_OTHER,
        // Leviathan::StringOperations::RemoveExtension(name, true),
        Leviathan::StringOperations::RemovePath(name),
        // Leviathan::StringOperations::GetExtension(name)
        "asset");

    if(file.empty()) {
        LOG_ERROR("Graphics: LoadShaderByName: could not find resource with name: " + name);
        return nullptr;
    }

    return Pimpl->LoadResource<bs::Shader>(std::filesystem::absolute(file).string().c_str());
}

DLLEXPORT bs::HTexture Graphics::LoadTextureByName(const std::string& name)
{
    auto file = FileSystem::Get()->SearchForFile(Leviathan::FILEGROUP_TEXTURE,
        // Leviathan::StringOperations::RemoveExtension(name, true),
        Leviathan::StringOperations::RemovePath(name),
        // Leviathan::StringOperations::GetExtension(name)
        "asset");

    if(file.empty()) {
        LOG_ERROR("Graphics: LoadTextureByName: could not find resource with name: " + name);
        return nullptr;
    }

    return Pimpl->LoadResource<bs::Texture>(std::filesystem::absolute(file).string().c_str());
}

DLLEXPORT bs::HMesh Graphics::LoadMeshByName(const std::string& name)
{
    auto file = FileSystem::Get()->SearchForFile(Leviathan::FILEGROUP_MODEL,
        // Leviathan::StringOperations::RemoveExtension(name, true),
        Leviathan::StringOperations::RemovePath(name),
        // Leviathan::StringOperations::GetExtension(name)
        "asset");

    if(file.empty()) {
        LOG_ERROR("Graphics: LoadMeshByName: could not find resource with name: " + name);
        return nullptr;
    }

    return Pimpl->LoadResource<bs::Mesh>(std::filesystem::absolute(file).string().c_str());
}

DLLEXPORT bs::HAnimationClip Graphics::LoadAnimationClipByName(const std::string& name)
{
    auto file = FileSystem::Get()->SearchForFile(Leviathan::FILEGROUP_MODEL,
        // Leviathan::StringOperations::RemoveExtension(name, true),
        Leviathan::StringOperations::RemovePath(name),
        // Leviathan::StringOperations::GetExtension(name)
        "asset");

    if(file.empty()) {
        LOG_ERROR(
            "Graphics: LoadAnimationClipByName: could not find resource with name: " + name);
        return nullptr;
    }

    return Pimpl->LoadResource<bs::AnimationClip>(
        std::filesystem::absolute(file).string().c_str());
}

// ------------------------------------ //
// X11 errors
#ifdef __linux
DLLEXPORT bool Graphics::HasX11ErrorOccured()
{
    if(HasX11Error) {
        HasX11Error = false;
        return true;
    }

    return false;
}
#endif
