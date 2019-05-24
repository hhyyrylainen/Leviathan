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

class Graphics::Private {
public:
    void* loadPlugin(const bs::String& pluginName, bs::DynLib** library = nullptr,
        void* passThrough = nullptr)
    {
        using namespace bs;

        DynLib* loadedLibrary = gDynLibManager().load(pluginName);
        if(library != nullptr)
            *library = loadedLibrary;

        void* retVal = nullptr;
        if(loadedLibrary != nullptr) {
            if(passThrough == nullptr) {
                typedef void* (*LoadPluginFunc)();

                LoadPluginFunc loadPluginFunc =
                    (LoadPluginFunc)loadedLibrary->getSymbol("loadPlugin");

                if(loadPluginFunc != nullptr)
                    retVal = loadPluginFunc();
            } else {
                typedef void* (*LoadPluginFunc)(void*);

                LoadPluginFunc loadPluginFunc =
                    (LoadPluginFunc)loadedLibrary->getSymbol("loadPlugin");

                if(loadPluginFunc != nullptr)
                    retVal = loadPluginFunc(passThrough);
            }

            // UpdatePluginFunc loadPluginFunc =
            //     (UpdatePluginFunc)loadedLibrary->getSymbol("updatePlugin");

            // if(loadPluginFunc != nullptr)
            //     mPluginUpdateFunctions[loadedLibrary] = loadPluginFunc;
        }

        return retVal;
    }

    void unloadPlugin(bs::DynLib* library)
    {
        typedef void (*UnloadPluginFunc)();

        UnloadPluginFunc unloadPluginFunc =
            (UnloadPluginFunc)library->getSymbol("unloadPlugin");

        if(unloadPluginFunc != nullptr)
            unloadPluginFunc();

        // mPluginUpdateFunctions.erase(library);
        bs::gDynLibManager().unload(library);
    }

    /**	Called when the frame finishes rendering. */
    void frameRenderingFinishedCallback()
    {
        bs::Lock lock(mFrameRenderingFinishedMutex);

        mIsFrameRenderingFinished = true;
        mFrameRenderingFinishedCondition.notify_one();
    }

    /**	Called by the core thread to begin profiling. */
    void beginCoreProfiling()
    {
        bs::gProfilerCPU().beginThread("Core");
    }

    /**	Called by the core thread to end profiling. */
    void endCoreProfiling()
    {
        bs::ProfilerGPU::instance()._update();

        bs::gProfilerCPU().endThread();
        bs::gProfiler()._updateCore();
    }


public:
    bs::START_UP_DESC BSFSettings;
    bs::DynLib* RendererPlugin = nullptr;

    bool mIsFrameRenderingFinished = true;
    bs::Mutex mFrameRenderingFinishedMutex;
    bs::Signal mFrameRenderingFinishedCondition;

    bs::SPtr<GUIOverlayRenderer> GUIRenderer;
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
    Pimpl = std::make_unique<Private>();

    // Startup SDL //
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
    auto& desc = Pimpl->BSFSettings;
    desc.input = "bsfNullInput";
    desc.audio = "bsfNullAudio";
    desc.physics = "bsfNullPhysics";
    desc.renderer = "bsfRenderBeast";
    desc.physicsCooking = false;

    desc.importers.push_back("bsfFreeImgImporter");
    desc.importers.push_back("bsfFBXImporter");
    desc.importers.push_back("bsfFontImporter");
    desc.importers.push_back("bsfSL");

    std::string renderAPI;
    ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
        appdef->GetValues(), "RenderAPI", renderAPI, "Vulkan");

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
        DEBUG_BREAK;
        desc.renderAPI = "bsfD3D11RenderAPI";
    }
#endif //_WIN32
    else {
        LOG_ERROR("Graphics: unknown render API selected: " + renderAPI);
        return false;
    }

    // A bunch of code here is copy pasted from BsCoreApplication.cpp
    using namespace bs;

    // Startup BSF
    UINT32 numWorkerThreads =
        BS_THREAD_HARDWARE_CONCURRENCY - 1; // Number of cores while excluding current thread.

    Platform::_startUp();
    MemStack::beginThread();

    ShaderManager::startUp(bs_shared_ptr_new<LeviathanBSFShaderIncludeHandler>());
    MessageHandler::startUp();
    ProfilerCPU::startUp();
    ProfilingManager::startUp();
    ThreadPool::startUp<TThreadPool<ThreadDefaultPolicy>>((numWorkerThreads));
    TaskScheduler::startUp();
    TaskScheduler::instance().removeWorker();
    RenderStats::startUp();
    CoreThread::startUp();
    StringTableManager::startUp();
    DeferredCallManager::startUp();
    bs::Time::startUp();
    DynLibManager::startUp();
    CoreObjectManager::startUp();
    GameObjectManager::startUp();
    Resources::startUp();
    ResourceListenerManager::startUp();
    GpuProgramManager::startUp();
    RenderStateManager::startUp();
    ct::GpuProgramManager::startUp();
    RenderAPIManager::startUp();

    // At this point startup needs to wait until we have our first window

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
        LOG_INFO(
            "Graphics: finalizing bs::framework initialization after creating first window");

        // Setup first window properties
        auto& windowDesc = Pimpl->BSFSettings.primaryWindowDesc;
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

        windowDesc.platformSpecific["externalWindowHandle"] =
            std::to_string(window.GetNativeHandle());

        windowDesc.platformSpecific["externalDisplay"] =
            std::to_string(window.GetWindowXDisplay());

        // A bunch of code here is copy pasted from BsCoreApplication.cpp
        using namespace bs;
        bs::SPtr<bs::RenderWindow> bsWindow = RenderAPIManager::instance().initialize(
            Pimpl->BSFSettings.renderAPI, Pimpl->BSFSettings.primaryWindowDesc);

        LEVIATHAN_ASSERT(bsWindow, "window creation failed");

        ct::ParamBlockManager::startUp();
        // Input::startUp();
        RendererManager::startUp();

        Pimpl->loadPlugin(Pimpl->BSFSettings.renderer, &Pimpl->RendererPlugin);

        // Must be initialized before the scene manager, as game scene creation triggers
        // physics scene creation
        PhysicsManager::startUp(Pimpl->BSFSettings.physics, Pimpl->BSFSettings.physicsCooking);
        SceneManager::startUp();
        RendererManager::instance().setActive(Pimpl->BSFSettings.renderer);
        // RendererManager::instance().initialize();

        ProfilerGPU::startUp();
        MeshManager::startUp();
        Importer::startUp();
        // AudioManager::startUp(mStartUpDesc.audio);
        AnimationManager::startUp();
        ParticleManager::startUp();

        for(auto& importerName : Pimpl->BSFSettings.importers)
            Pimpl->loadPlugin(importerName);

        // Built-in importers
        FGAImporter* fgaImporter = bs_new<FGAImporter>();
        Importer::instance()._registerAssetImporter(fgaImporter);

        // Code from BsApplication.cpp
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

        // FileSystem::RegisterOGREResourceGroups();

        auto shader =
            bs::gImporter().import<bs::Shader>("Data/Shaders/CoreShaders/ScreenSpaceGUI.bsl");

        auto material = bs::Material::create(shader);

        Pimpl->GUIRenderer =
            RendererExtension::create<GUIOverlayRenderer>(GUIOverlayInitializationData{
                GeometryHelpers::CreateScreenSpaceQuad(-1, -1, 2, 2)->getCore(),
                material->getCore()});

        //
        // Sample content
        //
        using namespace bs;


        HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");
        HCamera sceneCamera = sceneCameraSO->addComponent<CCamera>();
        sceneCamera->setMain(true);
        sceneCameraSO->setPosition(Vector3(40.0f, 30.0f, 230.0f));
        sceneCameraSO->lookAt(Vector3(0, 0, 0));

        return bsWindow;
    }
}

void Graphics::ShutdownBSF()
{
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

    // Wait until last core frame is finished before exiting
    {
        Lock lock(Pimpl->mFrameRenderingFinishedMutex);

        while(!Pimpl->mIsFrameRenderingFinished) {
            TaskScheduler::instance().addWorker();
            Pimpl->mFrameRenderingFinishedCondition.wait(lock);
            TaskScheduler::instance().removeWorker();
        }
    }

    Importer::shutDown();
    MeshManager::shutDown();
    ProfilerGPU::shutDown();

    SceneManager::shutDown();

    // Input::shutDown();

    ct::ParamBlockManager::shutDown();
    StringTableManager::shutDown();
    Resources::shutDown();
    GameObjectManager::shutDown();

    ResourceListenerManager::shutDown();
    RenderStateManager::shutDown();
    ParticleManager::shutDown();
    AnimationManager::shutDown();

    // This must be done after all resources are released since it will unload the physics
    // plugin, and some resources might be instances of types from that plugin.
    PhysicsManager::shutDown();

    RendererManager::shutDown();

    // All CoreObject related modules should be shut down now. They have likely queued
    // CoreObjects for destruction, so we need to wait for those objects to get destroyed
    // before continuing.
    CoreObjectManager::instance().syncToCore();
    gCoreThread().update();
    gCoreThread().submitAll(true);

    Pimpl->unloadPlugin(Pimpl->RendererPlugin);

    RenderAPIManager::shutDown();
    ct::GpuProgramManager::shutDown();
    GpuProgramManager::shutDown();

    CoreObjectManager::shutDown(); // Must shut down before DynLibManager to ensure all objects
                                   // are destroyed before unloading their libraries
    DynLibManager::shutDown();
    bs::Time::shutDown();
    DeferredCallManager::shutDown();

    CoreThread::shutDown();
    RenderStats::shutDown();
    TaskScheduler::shutDown();
    ThreadPool::shutDown();
    ProfilingManager::shutDown();
    ProfilerCPU::shutDown();
    MessageHandler::shutDown();
    ShaderManager::shutDown();

    MemStack::endThread();
    Platform::_shutDown();
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
    {
        Lock lock(Pimpl->mFrameRenderingFinishedMutex);

        while(!Pimpl->mIsFrameRenderingFinished) {
            TaskScheduler::instance().addWorker();
            Pimpl->mFrameRenderingFinishedCondition.wait(lock);
            TaskScheduler::instance().removeWorker();
        }

        Pimpl->mIsFrameRenderingFinished = false;
    }

    gCoreThread().queueCommand(
        std::bind(&Private::beginCoreProfiling, Pimpl.get()), CTQF_InternalQueue);
    gCoreThread().queueCommand(&Platform::_coreUpdate, CTQF_InternalQueue);
    gCoreThread().queueCommand(
        std::bind(&ct::RenderWindowManager::_update, ct::RenderWindowManager::instancePtr()),
        CTQF_InternalQueue);

    gCoreThread().update();
    gCoreThread().submitAll();

    gCoreThread().queueCommand(
        std::bind(&Private::frameRenderingFinishedCallback, Pimpl.get()), CTQF_InternalQueue);

    gCoreThread().queueCommand(
        std::bind(&ct::QueryManager::_update, ct::QueryManager::instancePtr()),
        CTQF_InternalQueue);
    gCoreThread().queueCommand(
        std::bind(&Private::endCoreProfiling, Pimpl.get()), CTQF_InternalQueue);

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

    bs::gCoreThread().queueCommand([this, coreVersion = std::move(coreVersion)]() {
        this->Pimpl->GUIRenderer->UpdateShownOverlays(coreVersion);
    });
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
