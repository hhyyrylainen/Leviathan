#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GRAPHICALINPUTENTITY
#include "GraphicalInputEntity.h"
#endif
#include "OgreCommon.h"
#include "Rendering/Graphics.h"
#include "Entities/GameWorld.h"
#include "FileSystem.h"
#include "Engine.h"
#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "CEGUI/RendererModules/Ogre/Renderer.h"
#include "CEGUI/SchemeManager.h"
#include "GUI/FontManager.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GraphicalInputEntity::GraphicalInputEntity(Graphics* windowcreater, AppDef* windowproperties) : MouseCaptureState(false), 
	CEGUIRenderer(NULL)
{

	// create window //

	const WindowDataDetails& WData = windowproperties->GetWindowDetails();

	// get vsync (this is rather expensive so it is stored) //
	bool vsync = windowproperties->GetVSync();

	// set some rendering specific parameters //
	Ogre::NameValuePairList WParams;

	// variables //
	int FSAA = 4;

	// get variables from engine configuration file //
	ObjectFileProcessor::LoadValueFromNamedVars<int>(windowproperties->GetValues(), L"FSAA", FSAA, 4, true, L"Graphics: Init:");

	Ogre::String fsaastr = Convert::ToString(FSAA);

	WParams["FSAA"] = fsaastr;
	WParams["vsync"] = vsync ? "true": "false";

	Ogre::String wcaption = Convert::WstringToString(WData.Title);
	// quicker access to the window //
	Ogre::RenderWindow* tmpwindow = windowcreater->GetOgreRoot()->createRenderWindow(wcaption, WData.Width, WData.Height, !WData.Windowed, &WParams);

	Ogre::CompositorManager2* compositor = NULL;

	// load resource groups since it is safe now //
	if(++GlobalWindowCount == 1){
		// Initialize the compositor //
		windowcreater->GetOgreRoot()->initialiseCompositor();

		
		// Notify engine to register threads to work with Ogre //
		Engine::GetEngine()->_NotifyThreadsRegisterOgre();
		FileSystem::RegisterOGREResourceGroups();

		compositor = windowcreater->GetOgreRoot()->getCompositorManager2();

		// These are loaded from files //
		//// Create the definition for the main window workspace //
		//Ogre::CompositorWorkspaceDef* maindef = compositor->addWorkspaceDefinition("WindowMainWorkspace");
		//// Create the definition for world workspace //
		//Ogre::CompositorWorkspaceDef* worlddef = compositor->addWorkspaceDefinition("WorldsWorkspace");

		// Create the GUI system //


		CEGUI::OgreRenderer& guirenderer = CEGUI::OgreRenderer::bootstrapSystem(*tmpwindow);
		CEGUIRenderer = &guirenderer;

		// Instantiate CEGUI script bridge //

		// Link it //
		CEGUI::System::getSingleton().setScriptingModule(0);

		// Print the used renderer //
		Logger::Get()->Info(L"GUI using CEGUI renderer: "+Convert::StringToWstring(guirenderer.getIdentifierString().c_str()));

		// Load the taharez look //
		CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");

		// Load the GUI fonts //
		windowcreater->GetFontManager()->LoadAllFonts();
	}
	// create the actual window //
	DisplayWindow = new Window(tmpwindow, this);
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
	WindowsGui = new Gui::GuiManager();
	if(!WindowsGui){
		throw ExceptionNULLPtr(L"cannot create GUI manager instance", 0, __WFUNCTION__, NULL);
	}

	if(!WindowsGui->Init(windowproperties, windowcreater, this)){

		Logger::Get()->Error(L"GraphicalInputEntity: Gui init failed");
		throw ExceptionNULLPtr(L"invalid GUI manager", 0, __WFUNCTION__, WindowsGui);
	}


	// create receiver interface //
	TertiaryReceiver = new InputController();
}

DLLEXPORT Leviathan::GraphicalInputEntity::~GraphicalInputEntity(){
	// GUI is very picky about delete order
	SAFE_RELEASEDEL(WindowsGui);


	SAFE_DELETE(DisplayWindow);
	SAFE_DELETE(TertiaryReceiver);
}

// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::ReleaseLinked(){
	// release world and object references //
	LinkedWorld.reset();
	LinkedCamera.reset();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::Render(int mspassed){

	if(LinkedWorld)
		LinkedWorld->UpdateCameraLocation(mspassed, LinkedCamera.get());

	// update input before each frame //
	WindowsGui->Render();

	// update window //
	Ogre::RenderWindow* tmpwindow = DisplayWindow->GetOgreWindow();

	// cancel actual rendering if window closed //
	if(tmpwindow->isClosed()){

		Logger::Get()->Warning(L"GraphicalInputEntity: Render: skipping render due to window being closed");
		return;
	}

	// finish rendering the window //
	tmpwindow->swapBuffers();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::LinkObjects(shared_ptr<ViewerCameraPos> camera, shared_ptr<GameWorld> world){
	LinkedCamera = camera;
	LinkedWorld = world;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::SaveScreenShot(const string &filename){
	// uses render target's capability to save it's contents //
	DisplayWindow->GetOgreWindow()->writeContentsToTimestampedFile(filename, "_window1.png");
}

DLLEXPORT void Leviathan::GraphicalInputEntity::Tick(int mspassed){
	// pass to GUI //
	WindowsGui->GuiTick(mspassed);
}

DLLEXPORT void Leviathan::GraphicalInputEntity::OnResize(int width, int height){
	// send to GUI //
	WindowsGui->OnResize();
}

DLLEXPORT void Leviathan::GraphicalInputEntity::UnlinkAll(){
	LinkedWorld.reset();
	LinkedCamera.reset();
}

DLLEXPORT bool Leviathan::GraphicalInputEntity::SetMouseCapture(bool state){
	if(MouseCaptureState == state)
		return true;

	MouseCaptureState = state;

	// handle changing state //
	if(!MouseCaptureState){

		// set mouse visible and disable capturing //
		WindowsGui->SetMouseFileVisibleState(true);
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
		WindowsGui->SetMouseFileVisibleState(false);
		DisplayWindow->SetCaptureMouse(true);
		DisplayWindow->SetMouseToCenter();

		// set static ptr to this //
		InputCapturer = this;
	}
	return true;
}

DLLEXPORT void Leviathan::GraphicalInputEntity::OnFocusChange(bool focused){
	WindowsGui->OnFocusChanged(focused);
}

GraphicalInputEntity* Leviathan::GraphicalInputEntity::InputCapturer = NULL;

int Leviathan::GraphicalInputEntity::GlobalWindowCount = 0;

DLLEXPORT int Leviathan::GraphicalInputEntity::GetGlobalWindowCount(){
	return GlobalWindowCount;
}