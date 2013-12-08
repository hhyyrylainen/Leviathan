#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GRAPHICALINPUTENTITY
#include "GraphicalInputEntity.h"
#endif
#include "OgreCommon.h"
#include "Rendering/Graphics.h"
#include "Entities/GameWorld.h"
#include "FileSystem.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GraphicalInputEntity::GraphicalInputEntity(Graphics* windowcreater, AppDef* windowproperties){

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

	// load resource groups since it is safe now //
	if(++GlobalWindowCount == 1){
		FileSystem::RegisterOGREResourceGroups();
	}

	// create the actual window //
	DisplayWindow = new Window(tmpwindow, this);
	// apply style settings (mainly ICON) //
	WData.ApplyIconToHandle(DisplayWindow->GetHandle());
	tmpwindow->setDeactivateOnFocusChange(false);

	// set the main window to be active //
	tmpwindow->setActive(true);

	// manual window updating //
	tmpwindow->setAutoUpdated(false);

	// create a window viewport //
	float ViewWidth = 1.f;
	float ViewHeight = 1.f;
	float ViewLeft = (1.f-ViewWidth)*0.5f;
	float ViewTop = (1.f-ViewHeight)*0.5f;

	USHORT ZOrder = 100;

	MainViewport = tmpwindow->addViewport(NULL, ZOrder, ViewLeft, ViewTop, ViewWidth, ViewHeight);

	// set default viewport colour //
	MainViewport->setBackgroundColour(Ogre::ColourValue(0.3f, 0.6f, 0.9f));

	// automatic updating //
	MainViewport->setAutoUpdated(true);

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

	// TODO: link to window //

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

// ------------------------------------ //

DLLEXPORT float Leviathan::GraphicalInputEntity::GetViewportAspectRatio(){
	return MainViewport->getActualWidth()/(float)MainViewport->getActualHeight();
}

DLLEXPORT void Leviathan::GraphicalInputEntity::Render(int mspassed){

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

	tmpwindow->update(false);
	// all automatically updated view ports are updated //

	// update special view ports //

	// finish rendering the window //
	tmpwindow->swapBuffers(/*DisplayWindow->GetVsync()*/);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GraphicalInputEntity::LinkObjects(shared_ptr<ViewerCameraPos> camera, shared_ptr<GameWorld> world){
	LinkedCamera = camera;
	LinkedWorld = world;

	LinkedWorld->UpdateCameraAspect(this);
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
	WindowsGui->OnResize(width, height);
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

GraphicalInputEntity* Leviathan::GraphicalInputEntity::InputCapturer = NULL;

int Leviathan::GraphicalInputEntity::GlobalWindowCount = 0;

