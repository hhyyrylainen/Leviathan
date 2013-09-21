#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_WINDOW
#include "Window.h"
#endif
#include "Engine.h"
#include "Rocket\Core\Context.h"
#include <boost\assign\list_of.hpp>
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::Window::Window(Ogre::RenderWindow* owindow, bool vsync) : OWindow(owindow), VerticalSync(vsync), m_hwnd(NULL), 
	WindowsInputManager(NULL), WindowMouse(NULL), WindowKeyboard(NULL), inputreceiver(NULL), LastFrameDonwMouseButtons(0)
{

	// update focused this way //
	Focused = (m_hwnd = GetRenderWindowHandle(OWindow)) == GetForegroundWindow() ? true: false;

	// register as listener to get update notifications //
	Ogre::WindowEventUtilities::addWindowEventListener(OWindow, this);

	// cursor on top of window's windows isn't hidden //
	CursorHidden = false;

	SetupOISForThisWindow();
}

DLLEXPORT Leviathan::Window::~Window(){
	// unregister, just in case //
	Ogre::WindowEventUtilities::removeWindowEventListener(OWindow, this);
	// close window (might be closed already) //
	//OWindow->destroy();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::SetHideCursor(bool toset){
	if(toset == CursorHidden)
		return;
	CursorHidden = toset;
	if(CursorHidden){
		ShowCursor(FALSE);
	} else {
		ShowCursor(TRUE);
	}
}

DLLEXPORT void Leviathan::Window::SetMouseToCenter(){
	// update window handle before using //
	m_hwnd = GetRenderWindowHandle(OWindow);
	if(m_hwnd == NULL){
		// window has closed //
		return;
	}

	POINT p;
	p.x = GetWidth()/2;
	p.y = GetHeight()/2;

	ClientToScreen(m_hwnd, &p);

	SetCursorPos(p.x, p.y);
}

DLLEXPORT void Leviathan::Window::GetRelativeMouse(int& x, int& y){
	// update window handle before using //
	m_hwnd = GetRenderWindowHandle(OWindow);
	if(m_hwnd == NULL){
		// window has closed //
		x = y = 0;
		return;
	}

	POINT p;
	GetCursorPos(&p);

	ScreenToClient(m_hwnd, &p);

	x = p.x;
	y = p.y;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::CloseDown(){
	// close the window //
	OWindow->destroy();
	ReleaseOIS();
}

DLLEXPORT void Leviathan::Window::ResizeWindow(const int &width, const int &height){
	// make ogre window resize //
	OWindow->resize(width, height);
}
// ------------------------------------ //
void Leviathan::Window::windowResized(Ogre::RenderWindow* rw){
	// notify engine //
	Engine::GetEngine()->OnResize(GetWidth(), GetHeight());

	UpdateOISMouseWindowSize();
}

void Leviathan::Window::windowFocusChange(Ogre::RenderWindow* rw){
	// update handle to have it up to date //
	m_hwnd = GetRenderWindowHandle(OWindow);

	//Focused = m_hwnd == GetFocus() ? true: false;
	Focused = m_hwnd == GetForegroundWindow() ? true: false;
	
	// update engine focus state (TODO: add a list of focuses to support multiple windows) //
	Focused ? Engine::GetEngine()->GainFocus(): Engine::GetEngine()->LoseFocus();

	wstring message = L"Window focus is now ";
	message += Focused ? L"true": L"false";

	Logger::Get()->Info(message);
}
// ------------------------------------ //
HWND Leviathan::Window::GetRenderWindowHandle(Ogre::RenderWindow* owindow){

	unsigned int WindowHwnd(0);

	owindow->getCustomAttribute(Ogre::String("WINDOW"), &WindowHwnd);

	HWND reswnd = reinterpret_cast<HWND>(WindowHwnd);

	if(!IsWindow(reswnd)){
		// not a window! //
		return NULL;
	}

	return reswnd;
}
// ------------------------------------ //
bool Leviathan::Window::SetupOISForThisWindow(){
	// creation parameters //
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	OWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
#if defined OIS_WIN32_PLATFORM
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
	pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
	pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
	pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
	pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif


	// create system with parameters //
	WindowsInputManager = OIS::InputManager::createInputSystem(pl);

	// link wanted devices //
	WindowMouse = static_cast<OIS::Mouse*>(WindowsInputManager->createInputObject(OIS::OISMouse, true));
	WindowKeyboard = static_cast<OIS::Keyboard*>(WindowsInputManager->createInputObject(OIS::OISKeyboard, true));

	// there can be multiple joysticks //
	WindowJoysticks.resize(WindowsInputManager->getNumberOfDevices(OIS::OISJoyStick));

	for(size_t i = 0; i < WindowJoysticks.size(); i++){

		WindowJoysticks[i] = static_cast<OIS::JoyStick*>(WindowsInputManager->createInputObject(OIS::OISJoyStick, true));

		WindowJoysticks[i]->setEventCallback(this);
	}

	//WindowJoysticks = static_cast<OIS::JoyStick*>(WindowsInputManager->createInputObject(OIS::OISJoyStick, true));
	// no need to check all keys (we can just get events for keys that are used) //
	WindowKeyboard->setEventCallback(this);
	WindowMouse->setEventCallback(this);

	UpdateOISMouseWindowSize();

	return true;
}

void Leviathan::Window::ReleaseOIS(){

	WindowKeyboard->setEventCallback(NULL);
	WindowMouse->setEventCallback(NULL);
	// destroy objects and manager //
	WindowsInputManager->destroyInputObject(WindowMouse);
	WindowsInputManager->destroyInputObject(WindowKeyboard);

	while(WindowJoysticks.size() != 0){
		WindowsInputManager->destroyInputObject(WindowJoysticks[0]);
		WindowJoysticks.erase(WindowJoysticks.begin());
	}

	

	OIS::InputManager::destroyInputSystem(WindowsInputManager);
}

void Leviathan::Window::UpdateOISMouseWindowSize(){
	// get dimensions //
	unsigned int width, height, depth;
	int top, left;
	OWindow->getMetrics(width, height, depth, left, top);
	// get mouse state and set it's dimensions //

	const OIS::MouseState &ms = WindowMouse->getMouseState();
	ms.width = width;
	ms.height = height;
}

DLLEXPORT void Leviathan::Window::GatherInput(Rocket::Core::Context* context){
	// set parameters that listener functions need //
	ThisFrameHandledCreate = false;
	inputreceiver = context;

	// capture inputs and send events //
	WindowMouse->capture();


	// this causes window to receive events so we need to be ready to dispatch them to OIS //
	WindowKeyboard->capture();

	// joysticks //
	for(size_t i = 0; i < WindowJoysticks.size(); i++){

		WindowJoysticks[i]->capture();
	}


	// listener functions should have handled everything //
	ThisFrameHandledCreate = false;
	// everything is now processed //
	inputreceiver = NULL;
}

void Leviathan::Window::CheckInputState(){
	if(ThisFrameHandledCreate)
		return;

	// create keyboard special key states here //
	SpecialKeyModifiers = 0;
	if(WindowKeyboard->isModifierDown(OIS::Keyboard::Ctrl))
		SpecialKeyModifiers |= Rocket::Core::Input::KM_CTRL;

	if(WindowKeyboard->isModifierDown(OIS::Keyboard::Alt))
		SpecialKeyModifiers |= Rocket::Core::Input::KM_ALT;
	if(WindowKeyboard->isModifierDown(OIS::Keyboard::Shift))
		SpecialKeyModifiers |= Rocket::Core::Input::KM_SHIFT;
	if(WindowKeyboard->isKeyDown(OIS::KC_CAPITAL))
		SpecialKeyModifiers |= Rocket::Core::Input::KM_CAPSLOCK;
	if(WindowKeyboard->isKeyDown(OIS::KC_LWIN))
		SpecialKeyModifiers |= Rocket::Core::Input::KM_META;
	if(WindowKeyboard->isKeyDown(OIS::KC_NUMLOCK))
		SpecialKeyModifiers |= Rocket::Core::Input::KM_NUMLOCK;
	if(WindowKeyboard->isKeyDown(OIS::KC_SCROLL))
		SpecialKeyModifiers |= Rocket::Core::Input::KM_SCROLLLOCK;



	ThisFrameHandledCreate = true;
}
// ------------------ Input listener functions ------------------ //
bool Leviathan::Window::keyPressed(const OIS::KeyEvent &arg){
	CheckInputState();
	// pass event to active Rocket context //

	inputreceiver->ProcessKeyDown(OISRocketKeyConvert[arg.key], SpecialKeyModifiers);
	// don't really know what to return
	return true;
}

bool Leviathan::Window::keyReleased(const OIS::KeyEvent &arg){
	CheckInputState();
	// pass event to active Rocket context //
	inputreceiver->ProcessKeyDown(OISRocketKeyConvert[arg.key], SpecialKeyModifiers);
	// don't really know what to return
	return true;
}

bool Leviathan::Window::mouseMoved(const OIS::MouseEvent &arg){
	CheckInputState();
	// pass event to active Rocket context //
	// send all mouse related things (except buttons) //
	const OIS::MouseState& mstate = arg.state;

	inputreceiver->ProcessMouseMove(mstate.X.rel, mstate.Y.rel, SpecialKeyModifiers);
	inputreceiver->ProcessMouseWheel(mstate.Z.abs, SpecialKeyModifiers);

	// don't really know what to return
	return true;
}

bool Leviathan::Window::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
	CheckInputState();
	// pass event to active Rocket context //
	int Keynumber = 0;
	arg.state.buttons;

	inputreceiver->ProcessMouseButtonDown(Keynumber, SpecialKeyModifiers);

	// don't really know what to return
	return true;
}

bool Leviathan::Window::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
	CheckInputState();
	// pass event to active Rocket context //
	int Keynumber = 0;



	inputreceiver->ProcessMouseButtonDown(Keynumber, SpecialKeyModifiers);

	// don't really know what to return
	return true;
}

bool Leviathan::Window::buttonPressed(const OIS::JoyStickEvent &arg, int button){
	CheckInputState();

	return true;
}

bool Leviathan::Window::buttonReleased(const OIS::JoyStickEvent &arg, int button){
	CheckInputState();

	return true;
}

bool Leviathan::Window::axisMoved(const OIS::JoyStickEvent &arg, int axis){
	CheckInputState();

	return true;
}

// ------------------ WindowPassData ------------------ //
Leviathan::WindowPassData::WindowPassData(Window* wind, LeviathanApplication* appinterface){
	OwningWindow = wind;
	Appinterface = appinterface;
}



// ------------------ KeyCode conversion map ------------------ //
#define QUICKKEYPAIR(x, y) OIS::x, Rocket::Core::Input::y


map<OIS::KeyCode, Rocket::Core::Input::KeyIdentifier> Leviathan::Window::OISRocketKeyConvert = boost::assign::map_list_of
	(QUICKKEYPAIR(KC_UNASSIGNED, KI_UNKNOWN))
;


