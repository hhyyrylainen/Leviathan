// ------------------------------------ //
#include "Graphics.h"

#include "Application/AppDefine.h"
#include "Application/GameConfiguration.h"
#include "Common/StringOperations.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GUIOverlayRenderer.h"
#include "GeometryHelpers.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Threading/ThreadingManager.h"
#include "Window.h"

#include "BsApplication.h"
#include "Components/BsCCamera.h"
#include "Scene/BsSceneObject.h"

#include "bsfCore/Material/BsShaderManager.h"


// Temporary BSF includes before the application has a light-weigth alternative init
#include "Animation/BsAnimationManager.h"
#include "Audio/BsAudio.h"
#include "Audio/BsAudioManager.h"
#include "CoreThread/BsCoreObjectManager.h"
#include "CoreThread/BsCoreThread.h"
#include "Importer/BsImporter.h"
#include "Localization/BsStringTableManager.h"
#include "Managers/BsGpuProgramManager.h"
#include "Managers/BsMeshManager.h"
#include "Managers/BsQueryManager.h"
#include "Managers/BsRenderStateManager.h"
#include "Managers/BsRenderWindowManager.h"
#include "Managers/BsResourceListenerManager.h"
#include "Material/BsShaderManager.h"
#include "Math/BsVector2.h"
#include "Particles/BsParticleManager.h"
#include "Particles/BsVectorField.h"
#include "Physics/BsPhysics.h"
#include "Physics/BsPhysicsManager.h"
#include "Platform/BsPlatform.h"
#include "Profiling/BsProfilerCPU.h"
#include "Profiling/BsProfilerGPU.h"
#include "Profiling/BsProfilingManager.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Renderer/BsParamBlocks.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererManager.h"
#include "Resources/BsResources.h"
#include "Scene/BsGameObjectManager.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "Threading/BsTaskScheduler.h"
#include "Threading/BsThreadPool.h"
#include "Utility/BsDeferredCallManager.h"
#include "Utility/BsDynLib.h"
#include "Utility/BsDynLibManager.h"
#include "Utility/BsMessageHandler.h"
#include "Utility/BsTime.h"
#include "bsfCore/Managers/BsRenderAPIManager.h"

// BsApplication
#include "2D/BsSpriteManager.h"
#include "BsEngineConfig.h"
#include "CoreThread/BsCoreObjectManager.h"
#include "CoreThread/BsCoreThread.h"
#include "Debug/BsDebugDraw.h"
#include "FileSystem/BsFileSystem.h"
#include "GUI/BsGUIManager.h"
#include "GUI/BsProfilerOverlay.h"
#include "GUI/BsShortcutManager.h"
#include "Importer/BsImporter.h"
#include "Input/BsVirtualInput.h"
#include "Platform/BsCursor.h"
#include "Platform/BsPlatform.h"
#include "Profiling/BsProfilingManager.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRendererMaterialManager.h"
#include "Resources/BsBuiltinResources.h"
#include "Resources/BsEngineShaderIncludeHandler.h"
#include "Resources/BsPlainTextImporter.h"
#include "Resources/BsResources.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "Script/BsScriptManager.h"



#include <SDL.h>
#include <SDL_syswm.h>

#include <future>
#include <regex>

#ifdef __linux
#include "XLibInclude.h"
#endif


using namespace Leviathan;
// ------------------------------------ //

class LeviathanBSFShaderIncludeHandler : public bs::DefaultShaderIncludeHandler {
public:
    virtual bs::HShaderInclude findInclude(const bs::String& name) const override
    {
        // If the file path is valid just pass it as is
        const std::string converted(name.c_str(), name.size());
        if(FileSystem::FileExists(converted))
            return bs::DefaultShaderIncludeHandler::findInclude(name);

        // We resolve the path and then give it to bsf
        std::string searched = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT,
            StringOperations::RemoveExtension<std::string>(converted),
            StringOperations::GetExtension<std::string>(converted), true);

        if(searched.empty()) {
            LOG_WARNING("LeviathanBSFShaderIncludeHandler: could not locate file anywhere: " +
                        converted);
            return nullptr;
        }

        return bs::DefaultShaderIncludeHandler::findInclude(
            bs::String(searched.c_str(), searched.size()));
    }
};

class LeviathanBSFApplication : public bs::CoreApplication {
public:
    LeviathanBSFApplication(const bs::START_UP_DESC& desc) : bs::CoreApplication(desc) {}

    bs::SPtr<bs::IShaderIncludeHandler> getShaderIncludeHandler() const override
    {
        return bs::bs_shared_ptr_new<LeviathanBSFShaderIncludeHandler>();
    }

    // Code from BsCoreApplication.cpp
    // These are duplicated here due to a bunch of things being private
    void WaitUntilLastFrame()
    {
        {
            Lock lock(mFrameRenderingFinishedMutex);

            while(!mIsFrameRenderingFinished) {
                bs::TaskScheduler::instance().addWorker();
                mFrameRenderingFinishedCondition.wait(lock);
                bs::TaskScheduler::instance().removeWorker();
            }
        }
    }

    void WaitBeforeStartNextFrame()
    {
        bs::Lock lock(mFrameRenderingFinishedMutex);

        while(!mIsFrameRenderingFinished) {
            bs::TaskScheduler::instance().addWorker();
            mFrameRenderingFinishedCondition.wait(lock);
            bs::TaskScheduler::instance().removeWorker();
        }

        mIsFrameRenderingFinished = false;
    }

    void frameRenderingFinishedCallback()
    {
        bs::Lock lock(mFrameRenderingFinishedMutex);

        mIsFrameRenderingFinished = true;
        mFrameRenderingFinishedCondition.notify_one();
    }

    // These are private forcing to duplicate these here
    void beginCoreProfiling()
    {
#if !BS_FORCE_SINGLETHREADED_RENDERING
        bs::gProfilerCPU().beginThread("Core");
#endif
    }

    void endCoreProfiling()
    {
        bs::ProfilerGPU::instance()._update();

#if !BS_FORCE_SINGLETHREADED_RENDERING
        bs::gProfilerCPU().endThread();
        bs::gProfiler()._updateCore();
#endif
    }

    void startUpRenderer() override
    {
        // Do nothing, we activate the renderer at a later stage
    }



    bs::SPtr<GUIOverlayRenderer> GUIRenderer;
};


struct Graphics::Private {

    Private(const bs::START_UP_DESC& desc) : Description(desc) {}

    bs::START_UP_DESC Description;
    LeviathanBSFApplication* OurApp = nullptr;
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


    if(!InitializeBSF(appdef)) {

        Logger::Get()->Error("Graphics: Init: failed to create bs::framework renderer");
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

        SDL_Quit();
    }

    Initialized = false;
    FirstWindowCreated = false;
    Pimpl.reset();
}
// ------------------------------------------- //
bool Graphics::InitializeBSF(AppDef* appdef)
{
    // Create render API settings
    bs::START_UP_DESC desc;
    desc.input = "bsfNullInput";
    desc.audio = "bsfNullAudio";
    desc.physics = "bsfNullPhysics";
    desc.renderer = "bsfRenderBeast";
    desc.physicsCooking = false;

    desc.importers.push_back("bsfFreeImgImporter");
    desc.importers.push_back("bsfFBXImporter");
    desc.importers.push_back("bsfFontImporter");
    desc.importers.push_back("bsfSL");

#ifdef _WIN32
    const auto defaultRenderer = "DirectX";
#elif defined(__linux__)
    const auto defaultRenderer = "OpenGL";
#else
    const auto defaultRenderer = "Vulkan";
#endif

    std::string renderAPI;
    ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
        appdef->GetValues(), "RenderAPI", renderAPI, defaultRenderer);

    LOG_INFO("Graphics: preferred rendering API: '" + renderAPI + "'");

    LOG_WRITE("TODO: add detection if vulkan is available or not");

    const std::regex vulkan(
        "Vulkan", std::regex_constants::ECMAScript | std::regex_constants::icase);
    const std::regex opengl(
        "OpenGL", std::regex_constants::ECMAScript | std::regex_constants::icase);

#ifdef _WIN32
    const std::regex directx(
        "DirectX\\s*(11)?", std::regex_constants::ECMAScript | std::regex_constants::icase);
#endif //_WIN32

    if(std::regex_match(renderAPI, vulkan)) {
        desc.renderAPI = "bsfVulkanRenderAPI";
    } else if(std::regex_match(renderAPI, opengl)) {
        desc.renderAPI = "bsfGLRenderAPI";
    }
#ifdef _WIN32
    else if(std::regex_match(renderAPI, directx)) {
        desc.renderAPI = "bsfD3D11RenderAPI";
    }
#endif //_WIN32
    else {
        LOG_ERROR("Graphics: unknown render API selected: " + renderAPI);
        return false;
    }

    Pimpl = std::make_unique<Private>(desc);

    return true;
}

bs::SPtr<bs::RenderWindow> Graphics::RegisterCreatedWindow(Window& window)
{
    if(FirstWindowCreated) {
        // Register secondary window
        // TODO: second window setup
        DEBUG_BREAK;
        return nullptr;

    } else {
        // Finish initializing graphics
        FirstWindowCreated = true;
        LOG_INFO("Graphics: doing bs::framework initialization after creating first window");

        // Setup first window properties
        auto& windowDesc = Pimpl->Description.primaryWindowDesc;
        windowDesc.depthBuffer = true;
        // Not sure what all settings need to be copied
        windowDesc.fullscreen = /* window.IsFullScreen() */ false;
        windowDesc.vsync = false;

        // Fill video mode info from SDL
        SDL_DisplayMode dm;
        if(SDL_GetDesktopDisplayMode(0, &dm) != 0) {
            LOG_ERROR("Graphics: RegisterCreatedWindow: failed to get desktop display mode:" +
                      std::string(SDL_GetError()));
            return nullptr;
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

        bs::CoreApplication::startUp<LeviathanBSFApplication>(Pimpl->Description);

        Pimpl->OurApp =
            static_cast<LeviathanBSFApplication*>(bs::CoreApplication::instancePtr());

        bs::SPtr<bs::RenderWindow> bsWindow =
            bs::CoreApplication::instance().getPrimaryWindow();

        LEVIATHAN_ASSERT(bsWindow, "window creation failed");

        // Code from BsApplication.cpp
        using namespace bs;

        PlainTextImporter* importer = bs_new<PlainTextImporter>();
        Importer::instance()._registerAssetImporter(importer);

        // VirtualInput::startUp();
        BuiltinResources::startUp();
        RendererMaterialManager::startUp();
        RendererManager::instance().initialize();
        SpriteManager::startUp();
        // GUIManager::startUp();
        // ShortcutManager::startUp();

        // bs::Cursor::startUp();
        // bs::Cursor::instance().setCursor(CursorType::Arrow);
        // Platform::setIcon(BuiltinResources::instance().getFrameworkIcon());

        SceneManager::instance().setMainRenderTarget(bsWindow);
        DebugDraw::startUp();

        // startUpScriptManager();


        // Notify engine to register threads to work with Ogre //
        // Engine::GetEngine()->_NotifyThreadsRegisterOgre();

        auto shader =
            bs::gImporter().import<bs::Shader>("Data/Shaders/CoreShaders/ScreenSpaceGUI.bsl");

        auto material = bs::Material::create(shader);

        Pimpl->OurApp->GUIRenderer =
            RendererExtension::create<GUIOverlayRenderer>(GUIOverlayInitializationData{
                GeometryHelpers::CreateScreenSpaceQuad(-1, -1, 2, 2)->getCore(),
                material->getCore()});

        return bsWindow;
    }
}

void Graphics::ShutdownBSF()
{
    if(Pimpl) {
        Pimpl->OurApp->GUIRenderer = nullptr;
        Pimpl->OurApp->WaitUntilLastFrame();
    }

    // A bunch of code here is copy pasted from BsCoreApplication.cpp
    using namespace bs;

    // This part is from BsApplication.cpp
    // Need to clear all objects before I unload any plugins, as they
    // could have allocated parts or all of those objects.
    SceneManager::instance().clearScene(true);

    // Resources too (Prefabs especially, since they hold the same data as a scene)
    Resources::instance().unloadAll();

    // Shut down before script manager as scripts could have registered shortcut callbacks
    // ShortcutManager::shutDown();

    // ScriptManager::shutDown();
    DebugDraw::shutDown();

    // Cleanup any new objects queued for destruction by unloaded scripts
    CoreObjectManager::instance().syncToCore();
    gCoreThread().update();
    gCoreThread().submitAll(true);

    // bs::Cursor::shutDown();

    // GUIManager::shutDown();
    SpriteManager::shutDown();
    BuiltinResources::shutDown();
    RendererMaterialManager::shutDown();
    // VirtualInput::shutDown();

    bs::CoreApplication::shutDown();
    Pimpl->OurApp = nullptr;
}
// ------------------------------------ //
DLLEXPORT bool Graphics::Frame()
{
    // Logic for this frame is already ready, just tell bsf to render once

    // A bunch of code here is copy pasted from BsCoreApplication.cpp
    using namespace bs;

    gProfilerCPU().beginThread("Sim");

    // This does unwanted things with X11
    // Platform::_update();

    gTime()._update();
    // gInput()._update();
    // RenderWindowManager::update needs to happen after Input::update and before
    // Input::_triggerCallbacks, so that all input is properly captured in case there is a
    // focus change, and so that focus change is registered before input events are sent out
    // (mouse press can result in code checking if a window is in focus, so it has to be up to
    // date)
    RenderWindowManager::instance()._update();
    // gInput()._triggerCallbacks();
    gDebug()._triggerCallbacks();

    // preUpdate();

    // Trigger fixed updates if required
    {
        UINT64 step;
        const UINT32 numIterations = gTime()._getFixedUpdateStep(step);

        const float stepSeconds = step / 1000000.0f;
        for(UINT32 i = 0; i < numIterations; i++) {
            // fixedUpdate();
            PROFILE_CALL(gSceneManager()._fixedUpdate(), "Scene fixed update");
            // PROFILE_CALL(gPhysics().fixedUpdate(stepSeconds), "Physics simulation");

            gTime()._advanceFixedUpdate(step);
        }
    }

    PROFILE_CALL(gSceneManager()._update(), "Scene update");
    // gAudio()._update();
    // gPhysics().update();

    // Update plugins
    // for(auto& pluginUpdateFunc : mPluginUpdateFunctions)
    //     pluginUpdateFunc.second();

    // postUpdate();

    PerFrameData perFrameData;

    // Evaluate animation after scene and plugin updates because the renderer will just now be
    // displaying the animation we sent on the previous frame, and we want the scene
    // information to match to what is displayed.
    perFrameData.animation = AnimationManager::instance().update();
    perFrameData.particles = ParticleManager::instance().update(*perFrameData.animation);

    // Send out resource events in case any were loaded/destroyed/modified
    ResourceListenerManager::instance().update();

    // Trigger any renderer task callbacks (should be done before scene object update, or core
    // sync, so objects have a chance to respond to the callback).
    RendererManager::instance().getActive()->update();

    gSceneManager()._updateCoreObjectTransforms();
    PROFILE_CALL(RendererManager::instance().getActive()->renderAll(perFrameData), "Render");

    // Core and sim thread run in lockstep. This will result in a larger input latency than if
    // I was running just a single thread. Latency becomes worse if the core thread takes
    // longer than sim thread, in which case sim thread needs to wait. Optimal solution would
    // be to get an average difference between sim/core thread and start the sim thread a bit
    // later so they finish at nearly the same time.
    Pimpl->OurApp->WaitBeforeStartNextFrame();

    gCoreThread().queueCommand(
        std::bind(&LeviathanBSFApplication::beginCoreProfiling, Pimpl->OurApp),
        CTQF_InternalQueue);
    gCoreThread().queueCommand(&Platform::_coreUpdate, CTQF_InternalQueue);
    gCoreThread().queueCommand(
        std::bind(&ct::RenderWindowManager::_update, ct::RenderWindowManager::instancePtr()),
        CTQF_InternalQueue);

    gCoreThread().update();
    gCoreThread().submitAll();

    gCoreThread().queueCommand(
        std::bind(&LeviathanBSFApplication::frameRenderingFinishedCallback, Pimpl->OurApp),
        CTQF_InternalQueue);

    gCoreThread().queueCommand(
        std::bind(&ct::QueryManager::_update, ct::QueryManager::instancePtr()),
        CTQF_InternalQueue);
    gCoreThread().queueCommand(
        std::bind(&LeviathanBSFApplication::endCoreProfiling, Pimpl->OurApp),
        CTQF_InternalQueue);

    gProfilerCPU().endThread();
    gProfiler()._update();

    // At this point I think it is possible that a rendering operation is going on in the
    // background, but hopefully it is safe to execute game logic
    return true;
}
// ------------------------------------ //
DLLEXPORT void Graphics::UpdateShownOverlays(
    const std::vector<bs::SPtr<bs::Texture>>& overlays)
{
    std::vector<bs::SPtr<bs::ct::Texture>> coreVersion;
    coreVersion.reserve(overlays.size());

    std::transform(overlays.begin(), overlays.end(), std::back_inserter(coreVersion),
        [](const bs::SPtr<bs::Texture>& item) { return item->getCore(); });

    std::weak_ptr<GUIOverlayRenderer> rendererExtension = Pimpl->OurApp->GUIRenderer;

    bs::gCoreThread().queueCommand(
        [rendererExtension, coreVersion = std::move(coreVersion)]() {
            const auto locked = rendererExtension.lock();
            if(locked)
                locked->UpdateShownOverlays(coreVersion);
        });
}

DLLEXPORT bool Graphics::IsVerticalUVFlipped() const
{
    return bs::ct::RenderAPI::instance().getCapabilities(0).conventions.uvYAxis ==
           bs::Conventions::Axis::Up;
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

    // bs::HShader shader = bs::gImporter().import<bs::Shader>(file.c_str());
    bs::HShader shader = bs::gResources().load<bs::Shader>(file.c_str());

    if(!shader) {
        LOG_ERROR("Graphics: loading asset failed: " + name);
    }

    return shader;
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

    // bs::HTexture texture = bs::gImporter().import<bs::Texture>(file.c_str());
    bs::HTexture texture = bs::gResources().load<bs::Texture>(file.c_str());

    if(!texture) {
        LOG_ERROR("Graphics: loading asset failed: " + name);
    }

    return texture;
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

    // bs::HMesh mesh = bs::gImporter().import<bs::HMesh>(file.c_str());
    bs::HMesh mesh = bs::gResources().load<bs::Mesh>(file.c_str());

    if(!mesh) {
        LOG_ERROR("Graphics: loading asset failed: " + name);
    }

    return mesh;
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
