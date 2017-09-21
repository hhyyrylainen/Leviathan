// ------------------------------------ //
#include "GraphicalInputEntity.h"
// ------------------------------------ //
#include "../CEGUIInclude.h"

#include "OgreVector4.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"
#include "Handlers/IDFactory.h"
#include "Engine.h"
#include "Entities/GameWorld.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "GUI/FontManager.h"
#include "GUI/GuiManager.h"
#include "Input/InputController.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "OgreCommon.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "Rendering/Graphics.h"
#include "Window.h"
#include <thread>

#include "OgreWindowEventUtilities.h"
#include "OgreRenderWindow.h"

#ifdef LEVIATHAN_USING_SDL2
#include <SDL.h>
#include <SDL_syswm.h>
#endif

using namespace Leviathan;
using namespace std;
// ------------------------------------ //

constexpr auto CLEAR_WORKSPACE_NAME = "GraphicalInputEntity_clear_workspace";

namespace Leviathan{

//! \brief Used to hold objects that are required for clearing a GraphicalInputEntity each frame
class GEntityAutoClearResources{
public:

    GEntityAutoClearResources(Ogre::Root* destroyer) :
        Root(destroyer)
    {

    }
        
    ~GEntityAutoClearResources(){

        Root->getCompositorManager2()->removeWorkspace(WorldWorkspace);
        WorldWorkspace = nullptr;
            
        Root->destroySceneManager(WorldsScene);
        WorldsScene = nullptr;
        WorldSceneCamera = nullptr;
    }
        
        
    Ogre::Camera* WorldSceneCamera = nullptr;
    Ogre::SceneManager* WorldsScene = nullptr;
    Ogre::CompositorWorkspace* WorldWorkspace = nullptr;

    Ogre::Root* Root;
};

}

// ------------------------------------ //
DLLEXPORT Leviathan::GraphicalInputEntity::GraphicalInputEntity(Graphics* windowcreater,
    AppDef* windowproperties) :
    ID(IDFactory::GetID())
{
	// create window //

	const WindowDataDetails& WData = windowproperties->GetWindowDetails();

	// get vsync (this is rather expensive so it is stored) //
	bool vsync = windowproperties->GetVSync();

	// set some rendering specific parameters //
	Ogre::NameValuePairList WParams;

	// variables //
	int FSAA;
	// get variables from engine configuration file //
	ObjectFileProcessor::LoadValueFromNamedVars<int>(windowproperties->GetValues(), "FSAA",
        FSAA, 4, Logger::Get(), "Graphics: Init:");

	Ogre::String fsaastr = Convert::ToString(FSAA);

	WParams["FSAA"] = fsaastr;
	WParams["vsync"] = vsync ? "true": "false";

	Ogre::String wcaption = WData.Title;


    SDL_Window* sdlWindow = SDL_CreateWindow(
        WData.Title.c_str(), 
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(0), 
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(0), 
        WData.Width, WData.Height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    // SDL_WINDOW_FULLSCREEN_DESKTOP works so much better than
    // SDL_WINDOW_FULLSCREEN so it should be always used

    // SDL_WINDOW_BORDERLESS
    // SDL_WINDOWPOS_UNDEFINED_DISPLAY(x)
    // SDL_WINDOWPOS_CENTERED_DISPLAY(x)

    if(!sdlWindow){
        
        LOG_FATAL("SDL Window creation failed, error: " + std::string(SDL_GetError()));
    }

    //SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);
    
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdlWindow, &wmInfo);

#ifdef _WIN32
    size_t winHandle = reinterpret_cast<size_t>(wmInfo.info.win.window);
    WParams["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned long)winHandle);
    //externalWindowHandle
#else
    WParams["parentWindowHandle"] =
        Ogre::StringConverter::toString((unsigned long)wmInfo.info.x11.display) + ":" +
        Ogre::StringConverter::toString((unsigned int)XDefaultScreen(wmInfo.info.x11.display))
        + ":" +
        Ogre::StringConverter::toString((unsigned long)wmInfo.info.x11.window);

#endif

	Ogre::RenderWindow* tmpwindow = windowcreater->GetOgreRoot()->createRenderWindow(wcaption,
        WData.Width, WData.Height, false, &WParams);


    int windowsafter = 0;

    {
        Lock lock(GlobalCountMutex);
        ++GlobalWindowCount;
        windowsafter = GlobalWindowCount;
    }

    // Do some first window initialization //
	if(windowsafter == 1){
        
		// Notify engine to register threads to work with Ogre //
		Engine::GetEngine()->_NotifyThreadsRegisterOgre();
		FileSystem::RegisterOGREResourceGroups();
        windowcreater->_LoadOgreHLMS();

		// Create the GUI system //
		CEGUI::OgreRenderer& guirenderer = CEGUI::OgreRenderer::bootstrapSystem(*tmpwindow);
		CEGUIRenderer = &guirenderer;

        FirstCEGUIRenderer = &guirenderer;

		// Print the used renderer //
		Logger::Get()->Info(std::string("GUI using CEGUI renderer: ")+
            guirenderer.getIdentifierString().c_str());

		// Load the GUI fonts //
		windowcreater->GetFontManager()->LoadAllFonts();
        
	} else {

        // Wait for the first window to initialize //
        while(!FirstCEGUIRenderer){
            
            Logger::Get()->Info("GraphicalInputEntity: waiting for first window to initialize");
            
            std::this_thread::sleep_for(MillisecondDuration(10));
        }
        
        // Create a new renderer //
        CEGUIRenderer = &CEGUI::OgreRenderer::registerWindow(*FirstCEGUIRenderer, *tmpwindow);
    }
	
	// Store this window's number
    {
        Lock lock(TotalCountMutex);
        WindowNumber = ++TotalCreatedWindows;
    }

    OWindow = tmpwindow;
    
	// create the actual window //
	DisplayWindow = new Window(sdlWindow, this);

    _CreateOverlayScene();
    
#ifdef _WIN32
	// apply style settings (mainly ICON) //
	WData.ApplyIconToHandle(DisplayWindow->GetHandle());
#else
	// \todo linux icon
#endif
	tmpwindow->setDeactivateOnFocusChange(false);


	// set the main window to be active //
	tmpwindow->setActive(true);


	// create GUI //
	WindowsGui = new GUI::GuiManager();
	if(!WindowsGui){
		throw NULLPtr("cannot create GUI manager instance");
	}

	if(!WindowsGui->Init(windowcreater, this, windowsafter == 1)){

		Logger::Get()->Error("GraphicalInputEntity: Gui init failed");
		throw NULLPtr("invalid GUI manager");
	}


	// create receiver interface //
	TertiaryReceiver = std::shared_ptr<InputController>(new InputController());
}

DLLEXPORT Leviathan::GraphicalInputEntity::~GraphicalInputEntity(){
    
	GUARD_LOCK();

    // Do teardown //
    // Release Ogre resources //
    // Release the scene //
    Ogre::Root::getSingleton().destroySceneManager(OverlayScene);
    OverlayScene = NULL;

    StopAutoClearing();

    // GUI is very picky about delete order
    SAFE_RELEASEDEL(WindowsGui);

    // Report that the window is now closed //
    Logger::Get()->Info("Window: closing window("+
        Convert::ToString(GetWindowNumber())+")");

    // Close the window //
    OWindow->destroy();
    SAFE_DELETE(DisplayWindow);
    
    TertiaryReceiver.reset();

    int windowsafter = 0;

    {
        Lock lock(GlobalCountMutex);
        --GlobalWindowCount;
        windowsafter = GlobalWindowCount;
    }

    // Destory CEGUI if we are the last window //
    if(windowsafter == 0){

        FirstCEGUIRenderer = NULL;
        CEGUI::OgreRenderer::destroySystem();

        Logger::Get()->Info("GraphicalInputEntity: all windows have been closed, "
            "should quit soon");
    }

    CEGUIRenderer = NULL;
}

GraphicalInputEntity* Leviathan::GraphicalInputEntity::InputCapturer = NULL;

int Leviathan::GraphicalInputEntity::GlobalWindowCount = 0;

Mutex Leviathan::GraphicalInputEntity::GlobalCountMutex;

int Leviathan::GraphicalInputEntity::TotalCreatedWindows = 0;

Mutex Leviathan::GraphicalInputEntity::TotalCountMutex;

CEGUI::OgreRenderer* Leviathan::GraphicalInputEntity::FirstCEGUIRenderer = NULL;

bool Leviathan::GraphicalInputEntity::AutoClearResourcesCreated = false;
Mutex Leviathan::GraphicalInputEntity::AutoClearResourcesMutex;
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::ReleaseLinked(){
    // release world and object references //
    LinkedWorld.reset();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GraphicalInputEntity::Render(int mspassed, int tick, int timeintick){
    GUARD_LOCK();
    
    if(LinkedWorld)
        LinkedWorld->Render(mspassed, tick, timeintick);

    // update input before each frame //
    WindowsGui->Render();

    // update window //
    Ogre::RenderWindow* tmpwindow = GetOgreWindow();

    // finish rendering the window //
    tmpwindow->swapBuffers();

    return true;
}
// ------------------------------------ //
DLLEXPORT bool GraphicalInputEntity::GetVsync() const{
    return OWindow->isVSyncEnabled();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::CreateAutoClearWorkspaceDefIfNotAlready(){

    Lock lock(AutoClearResourcesMutex);

    if(AutoClearResourcesCreated)
        return;

    Ogre::CompositorManager2* manager =
        Graphics::Get()->GetOgreRoot()->getCompositorManager2();

    auto templatedworkspace =
        manager->addWorkspaceDefinition(CLEAR_WORKSPACE_NAME);

    // Create a node for rendering on top of everything
    auto rendernode = manager->addNodeDefinition("GraphicalInputEntity_clear_node");

    rendernode->setNumTargetPass(1);

    // The rendernode reads in the main render target in order to clear it
    rendernode->addTextureSourceName("renderwindow", 0,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);

    // Pass for it
    Ogre::CompositorTargetDef* targetpasses = 
        rendernode->addTargetPass("renderwindow");
    targetpasses->setNumPasses(2);

    Ogre::CompositorPassClearDef* clearpass =
        static_cast<Ogre::CompositorPassClearDef*>(targetpasses->
            addPass(Ogre::PASS_CLEAR));

    // Clear all of the buffers
    clearpass->mClearBufferFlags = Ogre::FBT_DEPTH | Ogre::FBT_STENCIL | Ogre::FBT_COLOUR;

    // Doesn't work
    // clearpass->mColourValue = Ogre::ColourValue::Green;

    // Needs a new material
    // Ogre::CompositorPassQuadDef* backgroundpass =
    //     static_cast<Ogre::CompositorPassQuadDef*>(targetpasses->
    //         addPass(Ogre::PASS_QUAD));

    // backgroundpass->mMaterialName = "Stone";

    // // This will hopefully help it get properly cleared //
    // Ogre::CompositorPassSceneDef* scenepass =
    //     static_cast<Ogre::CompositorPassSceneDef*>(targetpasses->
    //         addPass(Ogre::PASS_SCENE));
    
    // Connect the main render target to the node
    templatedworkspace->connectExternal(0, "GraphicalInputEntity_clear_node", 0);

    AutoClearResourcesCreated = true;
}

DLLEXPORT void Leviathan::GraphicalInputEntity::SetAutoClearing(const string &skyboxmaterial){

    // Skip if already doing this //
    if(AutoClearResources)
        return;

    CreateAutoClearWorkspaceDefIfNotAlready();

    Ogre::Root* ogre = Graphics::Get()->GetOgreRoot();

    AutoClearResources = std::make_unique<GEntityAutoClearResources>(ogre);
    
    // create scene manager //
    AutoClearResources->WorldsScene = ogre->createSceneManager(Ogre::ST_GENERIC, 1,
        Ogre::INSTANCING_CULLING_SINGLETHREAD,
        "GraphicalInputEntity_clear_scene_"+Convert::ToString(WindowNumber));

    // create camera //
    AutoClearResources->WorldSceneCamera =
        AutoClearResources->WorldsScene->createCamera("Cam");

    // Create the workspace for this scene //
    // Which will be rendered before the overlay workspace //
    AutoClearResources->WorldWorkspace =
        ogre->getCompositorManager2()->addWorkspace(AutoClearResources->WorldsScene,
        GetOgreWindow(), AutoClearResources->WorldSceneCamera,
            CLEAR_WORKSPACE_NAME, true, 0);

    // Without a skybox CEGUI flickers... //
    if(skyboxmaterial.empty())
        return;
    
    try{
        
        AutoClearResources->WorldsScene->setSkyBox(true, skyboxmaterial);
        
    } catch(const Ogre::InvalidParametersException &e){

        Logger::Get()->Error("GraphicalInputEntity: setting auto clear skybox " +
            e.getFullDescription());
    }

}

DLLEXPORT void Leviathan::GraphicalInputEntity::StopAutoClearing(){

    AutoClearResources.reset();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::LinkObjects(std::shared_ptr<GameWorld> world)
{
    LinkedWorld = world;
}
// ------------------------------------ //
DLLEXPORT void GraphicalInputEntity::SetCustomInputController(
    std::shared_ptr<InputController> controller)
{
    GUARD_LOCK();
    
    TertiaryReceiver = controller;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::GraphicalInputEntity::GetWindowNumber() const{

    return WindowNumber;
}
// ------------------------------------ //
#define COMMON_INPUT_START     if(!InputStarted){           \
 InputStarted = true;                                       \
 DisplayWindow->GatherInput(WindowsGui->GetContextInput()); \
 }

DLLEXPORT void GraphicalInputEntity::InjectMouseMove(int xpos, int ypos){

    COMMON_INPUT_START;
    DisplayWindow->InjectMouseMove(xpos, ypos);
}

DLLEXPORT void GraphicalInputEntity::InjectMouseWheel(int xamount, int yamount){
        
    COMMON_INPUT_START;
    DisplayWindow->InjectMouseWheel(xamount, yamount);
}

DLLEXPORT void GraphicalInputEntity::InjectMouseButtonDown(int32_t whichbutton){

    COMMON_INPUT_START;
    DisplayWindow->InjectMouseButtonDown(whichbutton);
}

DLLEXPORT void GraphicalInputEntity::InjectMouseButtonUp(int32_t whichbutton){

    COMMON_INPUT_START;
    DisplayWindow->InjectMouseButtonUp(whichbutton);
}

DLLEXPORT void GraphicalInputEntity::InjectCodePoint(uint32_t utf32char){

    COMMON_INPUT_START;
    DisplayWindow->InjectCodePoint(utf32char);
}

DLLEXPORT void GraphicalInputEntity::InjectKeyDown(int32_t sdlkey){

    COMMON_INPUT_START;
    DisplayWindow->InjectKeyDown(sdlkey);
}

DLLEXPORT void GraphicalInputEntity::InjectKeyUp(int32_t sdlkey){

    COMMON_INPUT_START;
    DisplayWindow->InjectKeyUp(sdlkey);
}


DLLEXPORT void GraphicalInputEntity::InputEnd(){
    
    // Force first mouse pos update //
    if(!InputStarted){

        DisplayWindow->ReadInitialMouse(WindowsGui->GetContextInput());
    }
    
    // Everything is now processed //
    DisplayWindow->inputreceiver = NULL;
    InputStarted = false;
    
    
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::SaveScreenShot(const string &filename){
    // uses render target's capability to save it's contents //
    GetOgreWindow()->writeContentsToTimestampedFile(filename, "_window1.png");
}

DLLEXPORT void Leviathan::GraphicalInputEntity::Tick(int mspassed){
    // pass to GUI //
    WindowsGui->GuiTick(mspassed);
}

DLLEXPORT void Leviathan::GraphicalInputEntity::OnResize(int width, int height){

    // Notify Ogre //
    GetOgreWindow()->resize(width, height);
    GetOgreWindow()->windowMovedOrResized();

    // send to GUI //
	WindowsGui->OnResize();
}

DLLEXPORT void Leviathan::GraphicalInputEntity::UnlinkAll(){
	LinkedWorld.reset();
}

DLLEXPORT bool Leviathan::GraphicalInputEntity::SetMouseCapture(bool state){
	if(MouseCaptureState == state)
		return true;

	GUARD_LOCK();

	MouseCaptureState = state;

	// handle changing state //
	if(!MouseCaptureState){

		// set mouse visible and disable capturing //
		DisplayWindow->SetCaptureMouse(false);

		// reset pointer to indicate that this object no longer captures mouse to this window //
		InputCapturer = NULL;

	} else {

		if(InputCapturer != this && InputCapturer != NULL){
			// another window has input //
			MouseCaptureState = false;
			return false;
		}

		// hide mouse and tell window to capture //
		DisplayWindow->SetCaptureMouse(true);
		DisplayWindow->SetMouseToCenter();

		// set static ptr to this //
		InputCapturer = this;
	}
	return true;
}

DLLEXPORT void Leviathan::GraphicalInputEntity::OnFocusChange(bool focused){

    if(DisplayWindow->Focused == focused)
        return;

    LOG_INFO("Focus change in Window");
    
    // Update mouse //
    DisplayWindow->Focused = focused;    
    DisplayWindow->_CheckMouseVisibilityStates();

	WindowsGui->OnFocusChanged(focused);

    if(!DisplayWindow->Focused && DisplayWindow->MouseCaptured){

        LOG_WRITE("TODO: We need to force GUI on to stop mouse capture");
        LOG_FATAL("Not implemented unfocus when mouse capture is on");
    }
}

DLLEXPORT int Leviathan::GraphicalInputEntity::GetGlobalWindowCount(){
	return GlobalWindowCount;
}


void Leviathan::GraphicalInputEntity::_CreateOverlayScene(){
    // create scene manager //
    OverlayScene = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_INTERIOR, 1,
        Ogre::INSTANCING_CULLING_SINGLETHREAD, "Overlay_forWindow_"+Convert::ToString(ID));

    OverLayCamera = OverlayScene->createCamera("empty camera");
}

