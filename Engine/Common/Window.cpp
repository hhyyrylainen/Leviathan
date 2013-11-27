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
// we must have an int of size 32 bits //
#pragma intrinsic(_BitScanForward)

static_assert(sizeof(int) == 4, "int must be 4 bytes long for bit scan function");


DLLEXPORT Leviathan::Window::Window(Ogre::RenderWindow* owindow, GraphicalInputEntity* owner) : OWindow(owindow), m_hwnd(NULL), 
	WindowsInputManager(NULL), WindowMouse(NULL), WindowKeyboard(NULL), inputreceiver(NULL), LastFrameDownMouseButtons(0), 
	ForceMouseVisible(false), CursorState(true), MouseCaptured(false), FirstInput(true)
{

	OwningWindow = owner;
	// update focused this way //
	Focused = (m_hwnd = GetRenderWindowHandle(OWindow)) == GetForegroundWindow() ? true: false;

	// register as listener to get update notifications //
	Ogre::WindowEventUtilities::addWindowEventListener(OWindow, this);

	// cursor on top of window's windows isn't hidden //
	ApplicationWantCursorState = false;

	SetupOISForThisWindow();
	_CreateOverlayScene();
}

DLLEXPORT Leviathan::Window::~Window(){
	// unregister, just in case //
	Ogre::WindowEventUtilities::removeWindowEventListener(OWindow, this);
	// close window (might be closed already) //
	//OWindow->destroy();

	// release Ogre resources //
	Ogre::Root::getSingleton().destroySceneManager(OverlayScene);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::SetHideCursor(bool toset){
	ApplicationWantCursorState = toset;

	if(!ApplicationWantCursorState || ForceMouseVisible){
		// show cursor //
		if(!CursorState){
			CursorState = true;
			ShowCursor(TRUE);
		}
	} else {
		// hide cursor //
		if(CursorState){
			CursorState = false;
			ShowCursor(FALSE);
		}
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


DLLEXPORT bool Leviathan::Window::IsMouseOutsideWindowClientArea(){
	int X, Y;
	GetRelativeMouse(X, Y);
	// check the coordinates //
	if(X < 0 || Y < 0 || X > GetWidth() || Y > GetHeight()){
		return true;
	}
	return false;
}


// ------------------------------------ //
DLLEXPORT void Leviathan::Window::CloseDown(){
	// release view ports, etc. //
	Ogre::Root::getSingleton().destroySceneManager(OverlayScene);
	OGRE_DELETE OverlayViewport;
	OverlayScene = NULL;
	OverlayViewport = NULL;
	// close the window //
	OWindow->destroy();


	ReleaseOIS();
}

DLLEXPORT void Leviathan::Window::ResizeWindow(const int &width, const int &height){
	// make ogre resize window //
	OWindow->resize(width, height);
}
// ------------------------------------ //
void Leviathan::Window::windowResized(Ogre::RenderWindow* rw){
	// TODO: add callback notification
	OwningWindow->OnResize(GetWidth(), GetHeight());
	
	UpdateOISMouseWindowSize();
}

void Leviathan::Window::windowFocusChange(Ogre::RenderWindow* rw){
	// update handle to have it up to date //
	m_hwnd = GetRenderWindowHandle(OWindow);

	//Focused = m_hwnd == GetFocus() ? true: false;
	Focused = m_hwnd == GetForegroundWindow() ? true: false;
	
	// update engine focus state (TODO: add a list of focuses to support multiple windows) //
	//Focused ? Engine::GetEngine()->GainFocus(): Engine::GetEngine()->LoseFocus();

	wstring message = L"Window focus is now ";
	message += Focused ? L"true": L"false";

	Logger::Get()->Info(message);

	// update mouse //
	_CheckMouseVisibilityStates();
	// little hack to get the context //
	_CustomMouseMakeSureMouseIsRight(OwningWindow->GetGUI()->GetContext());
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

	WindowKeyboard->setTextTranslation(OIS::Keyboard::Unicode);

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

	MouseCaptured = false;

	OIS::InputManager::destroyInputSystem(WindowsInputManager);
	// clear pointers //
	WindowKeyboard = NULL;
	WindowMouse = NULL;
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
	// quit if window closed //
	if(OWindow->isClosed() || !WindowKeyboard || !WindowMouse){

		Logger::Get()->Warning(L"Window: GatherInput: skipping due to closed input window");
		return;
	}

	// on first frame we want to manually force mouse position send //
	if(FirstInput){
		FirstInput = false;
		
		_CustomMouseMakeSureMouseIsRight(context);
		_CheckMouseVisibilityStates();
	}

	// set parameters that listener functions need //
	ThisFrameHandledCreate = false;
	inputreceiver = context;

	OwningWindow->GetInputController()->StartInputGather();

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

	// we need to just handle our own mouse capture function //
	if(MouseCaptured && Focused){

		// get mouse relative to window center //
		int xmoved = 0, ymoved = 0;

		GetRelativeMouse(xmoved, ymoved);

		// subtract center of window //
		xmoved -= GetWidth()/2;
		ymoved -= GetHeight()/2;

		// reset to center of window //
		SetMouseToCenter();

		// pass input //
		OwningWindow->GetInputController()->SendMouseMovement(xmoved, ymoved);
	}
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

	bool SentToController = false;

	if(inputreceiver->ProcessKeyDown(OISRocketKeyConvert[arg.key], SpecialKeyModifiers)){
		if(!OwningWindow->GetGUI()->ProcessKeyDown(arg.key, SpecialKeyModifiers)){

			SentToController = true;
			OwningWindow->GetInputController()->OnInputGet(arg.key, SpecialKeyModifiers, true);
		}
	}

	if(!SentToController){
		OwningWindow->GetInputController()->OnBlockedInput(arg.key, SpecialKeyModifiers, true);
	}


	// don't really know what to return
	return true;
}

bool Leviathan::Window::keyReleased(const OIS::KeyEvent &arg){
	CheckInputState();
	// pass event to active Rocket context //
	if(inputreceiver->ProcessKeyDown(OISRocketKeyConvert[arg.key], SpecialKeyModifiers)){
		// TODO: after not even GUI wanting update input object
		OwningWindow->GetInputController()->OnInputGet(arg.key, SpecialKeyModifiers, false);
	} else {
		OwningWindow->GetInputController()->OnBlockedInput(arg.key, SpecialKeyModifiers, false);
	}


	// don't really know what to return
	return true;
}

bool Leviathan::Window::mouseMoved(const OIS::MouseEvent &arg){
	CheckInputState();
	// pass event to active Rocket context //
	// send all mouse related things (except buttons) //
	const OIS::MouseState& mstate = arg.state;

	if(!MouseCaptured){
		// only pass this data if we aren't going to pass our own captured mouse //
		inputreceiver->ProcessMouseMove(mstate.X.abs, mstate.Y.abs, SpecialKeyModifiers);
		inputreceiver->ProcessMouseWheel(-mstate.Z.rel, SpecialKeyModifiers);
	}
	_CheckMouseVisibilityStates();

	// don't really know what to return
	return true;
}

bool Leviathan::Window::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
	CheckInputState();
	// pass event to active Rocket context //
	int differences = arg.state.buttons^LastFrameDownMouseButtons;

	// find differences //
	unsigned long index = 0;

	_BitScanForward(&index, differences);

	// update old state //
	LastFrameDownMouseButtons |= 1 << index;

	//wstring message = L"mouse number: "+Convert::ToWstring(index)+L" pressed; ";

	//wstringstream convert;
	//convert << std::hex << LastFrameDonwMouseButtons;
	//message += convert.str();

	//DEBUG_OUTPUT_AUTO(message);


	int Keynumber = index;
	if(!MouseCaptured)
		inputreceiver->ProcessMouseButtonDown(Keynumber, SpecialKeyModifiers);

	// don't really know what to return
	return true;
}

bool Leviathan::Window::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
	CheckInputState();
	// pass event to active Rocket context //
	int differences = arg.state.buttons^LastFrameDownMouseButtons;

	// find differences //
	unsigned long index = 0;

	_BitScanForward(&index, differences);

	// update old state //
	LastFrameDownMouseButtons ^= 1 << index;

	//wstring message = L"mouse number: "+Convert::ToWstring(index)+L" released; ";

	//wstringstream convert;
	//convert << std::hex << LastFrameDonwMouseButtons;
	//message += convert.str();

	//DEBUG_OUTPUT_AUTO(message);

	int Keynumber = index;

	inputreceiver->ProcessMouseButtonUp(Keynumber, SpecialKeyModifiers);

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

DLLEXPORT string Leviathan::Window::GetOISCharacterAsText(const OIS::KeyCode &code){
	return WindowKeyboard->getAsString(code);
}

void Leviathan::Window::_CreateOverlayScene(){
	// create scene manager //
	OverlayScene = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_EXTERIOR_FAR, "Overlay_forWindow_");

	// also needs a viewport that is last to be drawn //
	float ViewWidth = 1.f;
	float ViewHeight = 1.f;
	float ViewLeft = (1.f-ViewWidth)*0.5f;
	float ViewTop = (1.f-ViewHeight)*0.5f;

	USHORT ZOrder = 120;

	Ogre::Camera* camera = OverlayScene->createCamera("empty camera");

	OverlayViewport = OWindow->addViewport(camera, ZOrder, ViewLeft, ViewTop, ViewWidth, ViewHeight);

	OverlayScene->getRootSceneNode()->createChildSceneNode()->attachObject(camera);
	

	// set default viewport colour //
	OverlayViewport->setBackgroundColour(Ogre::ColourValue(0.f, 0.f, 0.f, 0.f));
	//OverlayViewport->setBackgroundColour(Ogre::ColourValue(0.8f, 0.2f, 0.5f, 1.f));

	// we want a transparent viewport //
	OverlayViewport->setClearEveryFrame(true, Ogre::FBT_DEPTH | Ogre::FBT_STENCIL);

	// automatic updating //
	OverlayViewport->setAutoUpdated(true);
}

DLLEXPORT void Leviathan::Window::SendCloseMessage(){
	OWindow->destroy();
}

void Leviathan::Window::_CheckMouseVisibilityStates(){
	// force cursor visible check (if outside client area or mouse is unfocused on the window) //
	if(IsMouseOutsideWindowClientArea() || !Focused){

		ForceMouseVisible = true;
	} else {

		ForceMouseVisible = false;
	}
	// update cursor state //
	SetHideCursor(ApplicationWantCursorState);
}

void Leviathan::Window::_CustomMouseMakeSureMouseIsRight(Rocket::Core::Context* context){
	// custom mouse get //
	int absx = 0, absy = 0;
	GetRelativeMouse(absx, absy);

	context->ProcessMouseMove(absx, absy, 0);
}

// ------------------ KeyCode conversion map ------------------ //
#define QUICKKEYPAIR(x, y) OIS::x, Rocket::Core::Input::y
#define SIMPLEPAIR(x, y)	L##x, OIS::y
#define QUICKONETOONEPAIR(x) OIS::KC_##x, Rocket::Core::Input::KI_##x


#define SIMPLEONETOONE(x)	WSTRINGIFY(x), OIS::KC_##x

std::map<wstring, OIS::KeyCode> Leviathan::Window::CharacterToOISConvert = boost::assign::map_list_of
	(SIMPLEONETOONE(A))
	(SIMPLEONETOONE(B))
	(SIMPLEONETOONE(C))
	(SIMPLEONETOONE(D))
	(SIMPLEONETOONE(E))
	(SIMPLEONETOONE(F))
	(SIMPLEONETOONE(G))
	(SIMPLEONETOONE(H))
	(SIMPLEONETOONE(I))
	(SIMPLEONETOONE(J))
	(SIMPLEONETOONE(K))
	(SIMPLEONETOONE(L))
	(SIMPLEONETOONE(M))
	(SIMPLEONETOONE(N))
	(SIMPLEONETOONE(O))
	(SIMPLEONETOONE(P))
	(SIMPLEONETOONE(Q))
	(SIMPLEONETOONE(R))
	(SIMPLEONETOONE(S))
	(SIMPLEONETOONE(T))
	(SIMPLEONETOONE(U))
	(SIMPLEONETOONE(V))
	(SIMPLEONETOONE(W))
	(SIMPLEONETOONE(X))
	(SIMPLEONETOONE(Y))
	(SIMPLEONETOONE(Z))

	(SIMPLEONETOONE(F1))
	(SIMPLEONETOONE(F2))
	(SIMPLEONETOONE(F3))
	(SIMPLEONETOONE(F4))
	(SIMPLEONETOONE(F5))
	(SIMPLEONETOONE(F6))
	(SIMPLEONETOONE(F7))
	(SIMPLEONETOONE(F8))
	(SIMPLEONETOONE(F9))
	(SIMPLEONETOONE(F10))
	(SIMPLEONETOONE(F11))
	(SIMPLEONETOONE(F12))
	(SIMPLEONETOONE(F13))
	(SIMPLEONETOONE(F14))
	(SIMPLEONETOONE(F15))
	// OIS doesn't support these advanced keys //
	/*(SIMPLEONETOONE(F16))
	(SIMPLEONETOONE(F17))
	(SIMPLEONETOONE(F18))
	(SIMPLEONETOONE(F19))
	(SIMPLEONETOONE(F20))
	(SIMPLEONETOONE(F21))
	(SIMPLEONETOONE(F22))
	(SIMPLEONETOONE(F23))
	(SIMPLEONETOONE(F24))*/

	(SIMPLEONETOONE(ESCAPE))
	(SIMPLEONETOONE(HOME))

	(SIMPLEONETOONE(NUMPAD0))
	(SIMPLEONETOONE(NUMPAD1))
	(SIMPLEONETOONE(NUMPAD2))
	(SIMPLEONETOONE(NUMPAD3))
	(SIMPLEONETOONE(NUMPAD4))
	(SIMPLEONETOONE(NUMPAD5))
	(SIMPLEONETOONE(NUMPAD6))
	(SIMPLEONETOONE(NUMPAD7))
	(SIMPLEONETOONE(NUMPAD8))
	(SIMPLEONETOONE(NUMPAD9))

	// some more "special" keys //
	(L"LEFTARROW", OIS::KC_LEFT)
	(L"RIGHTARROW", OIS::KC_RIGHT)
	(L"UPARROW", OIS::KC_UP)
	(L"DOWNARROW", OIS::KC_DOWN)
	(L"ESC", OIS::KC_ESCAPE)
;


DLLEXPORT OIS::KeyCode Leviathan::Window::ConvertWstringToOISKeyCode(const wstring &str){
	return CharacterToOISConvert[str];
}



map<OIS::KeyCode, Rocket::Core::Input::KeyIdentifier> Leviathan::Window::OISRocketKeyConvert = boost::assign::map_list_of
	(QUICKKEYPAIR(KC_UNASSIGNED, KI_UNKNOWN))
	(QUICKKEYPAIR(KC_ESCAPE, KI_ESCAPE))
	(QUICKONETOONEPAIR(1))
	(QUICKONETOONEPAIR(2))
	(QUICKONETOONEPAIR(3))
	(QUICKONETOONEPAIR(4))
	(QUICKONETOONEPAIR(5))
	(QUICKONETOONEPAIR(6))
	(QUICKONETOONEPAIR(7))
	(QUICKONETOONEPAIR(8))
	(QUICKONETOONEPAIR(9))
	(QUICKONETOONEPAIR(0))
	(QUICKKEYPAIR(KC_MINUS, KI_OEM_MINUS))
	(QUICKKEYPAIR(KC_EQUALS, KI_OEM_PLUS)) //
	(QUICKKEYPAIR(KC_BACK, KI_BACK))
	(QUICKKEYPAIR(KC_TAB, KI_TAB))
	(QUICKONETOONEPAIR(Q))
	(QUICKONETOONEPAIR(W))
	(QUICKONETOONEPAIR(E))
	(QUICKONETOONEPAIR(R))
	(QUICKONETOONEPAIR(T))
	(QUICKONETOONEPAIR(Y))
	(QUICKONETOONEPAIR(U))
	(QUICKONETOONEPAIR(I))
	(QUICKONETOONEPAIR(O))
	(QUICKONETOONEPAIR(P))
	(QUICKKEYPAIR(KC_LBRACKET, KI_OEM_4))
	(QUICKKEYPAIR(KC_RBRACKET, KI_OEM_6))
	(QUICKKEYPAIR(KC_RETURN, KI_RETURN))
	(QUICKKEYPAIR(KC_LCONTROL, KI_LCONTROL))
	(QUICKONETOONEPAIR(A))
	(QUICKONETOONEPAIR(S))
	(QUICKONETOONEPAIR(D))
	(QUICKONETOONEPAIR(F))
	(QUICKONETOONEPAIR(G))
	(QUICKONETOONEPAIR(H))
	(QUICKONETOONEPAIR(J))
	(QUICKONETOONEPAIR(K))
	(QUICKONETOONEPAIR(L))
	(QUICKKEYPAIR(KC_SEMICOLON, KI_OEM_1))
	(QUICKKEYPAIR(KC_APOSTROPHE, KI_OEM_7)) //
	(QUICKKEYPAIR(KC_GRAVE, KI_OEM_3))
	(QUICKKEYPAIR(KC_LSHIFT, KI_LSHIFT))
	(QUICKKEYPAIR(KC_BACKSLASH, KI_OEM_5))
	(QUICKONETOONEPAIR(Z))
	(QUICKONETOONEPAIR(X))
	(QUICKONETOONEPAIR(C))
	(QUICKONETOONEPAIR(V))
	(QUICKONETOONEPAIR(B))
	(QUICKONETOONEPAIR(N))
	(QUICKONETOONEPAIR(M))
	(QUICKKEYPAIR(KC_COMMA, KI_OEM_COMMA))
	(QUICKKEYPAIR(KC_PERIOD, KI_OEM_PERIOD))
	(QUICKKEYPAIR(KC_SLASH, KI_OEM_2))
	(QUICKKEYPAIR(KC_RSHIFT, KI_RSHIFT))
	(QUICKKEYPAIR(KC_MULTIPLY, KI_MULTIPLY))
	(QUICKKEYPAIR(KC_LMENU, KI_LMENU))
	(QUICKKEYPAIR(KC_SPACE, KI_SPACE))
	(QUICKKEYPAIR(KC_CAPITAL, KI_CAPITAL))
	(QUICKONETOONEPAIR(F1))
	(QUICKONETOONEPAIR(F2))
	(QUICKONETOONEPAIR(F3))
	(QUICKONETOONEPAIR(F4))
	(QUICKONETOONEPAIR(F5))
	(QUICKONETOONEPAIR(F6))
	(QUICKONETOONEPAIR(F7))
	(QUICKONETOONEPAIR(F8))
	(QUICKONETOONEPAIR(F9))
	(QUICKONETOONEPAIR(F10))
	(QUICKKEYPAIR(KC_NUMLOCK, KI_NUMLOCK))
	(QUICKKEYPAIR(KC_SCROLL, KI_SCROLL))
	(QUICKONETOONEPAIR(NUMPAD7))
	(QUICKONETOONEPAIR(NUMPAD8))
	(QUICKONETOONEPAIR(NUMPAD9))
	(QUICKKEYPAIR(KC_SUBTRACT, KI_SUBTRACT))
	(QUICKONETOONEPAIR(NUMPAD4))
	(QUICKONETOONEPAIR(NUMPAD5))
	(QUICKONETOONEPAIR(NUMPAD6))
	(QUICKKEYPAIR(KC_ADD, KI_ADD))
	(QUICKONETOONEPAIR(NUMPAD1))
	(QUICKONETOONEPAIR(NUMPAD2))
	(QUICKONETOONEPAIR(NUMPAD3))
	(QUICKONETOONEPAIR(NUMPAD9))
	(QUICKKEYPAIR(KC_DECIMAL, KI_DECIMAL))
	(QUICKKEYPAIR(KC_OEM_102, KI_OEM_102))
	(QUICKONETOONEPAIR(F11))
	(QUICKONETOONEPAIR(F12))
	(QUICKONETOONEPAIR(F13))
	(QUICKONETOONEPAIR(F14))
	(QUICKONETOONEPAIR(F15))
	(QUICKKEYPAIR(KC_OEM_102, KI_OEM_102))
	(QUICKKEYPAIR(KC_KANA, KI_KANA))
	(QUICKKEYPAIR(KC_ABNT_C1, KI_OEM_2)) //
	(QUICKKEYPAIR(KC_CONVERT, KI_CONVERT))
	(QUICKKEYPAIR(KC_NOCONVERT, KI_NONCONVERT))
	(QUICKKEYPAIR(KC_YEN, KI_UNKNOWN)) //
	(QUICKKEYPAIR(KC_ABNT_C2, KI_DECIMAL))
	(QUICKKEYPAIR(KC_NUMPADEQUALS, KI_OEM_NEC_EQUAL))
	(QUICKKEYPAIR(KC_PREVTRACK, KI_MEDIA_PREV_TRACK))
	(QUICKKEYPAIR(KC_AT, KI_OEM_8))
	(QUICKKEYPAIR(KC_COLON, KI_OEM_1)) //
	(QUICKKEYPAIR(KC_UNDERLINE, KI_OEM_MINUS)) //
	(QUICKKEYPAIR(KC_KANJI, KI_KANJI))
	(QUICKKEYPAIR(KC_STOP, KI_BROWSER_STOP)) //
	(QUICKKEYPAIR(KC_AX, KI_OEM_AX))
	(QUICKKEYPAIR(KC_UNLABELED, KI_UNKNOWN))
	(QUICKKEYPAIR(KC_NEXTTRACK, KI_MEDIA_NEXT_TRACK))
	(QUICKKEYPAIR(KC_NUMPADENTER, KI_NUMPADENTER))
	(QUICKKEYPAIR(KC_RCONTROL, KI_RCONTROL))
	(QUICKKEYPAIR(KC_MUTE, KI_VOLUME_MUTE))
	(QUICKKEYPAIR(KC_CALCULATOR, KI_UNKNOWN))
	(QUICKKEYPAIR(KC_PLAYPAUSE, KI_MEDIA_PLAY_PAUSE))
	(QUICKKEYPAIR(KC_MEDIASTOP, KI_MEDIA_STOP))
	(QUICKKEYPAIR(KC_VOLUMEDOWN, KI_VOLUME_DOWN))
	(QUICKKEYPAIR(KC_VOLUMEUP, KI_VOLUME_UP))
	(QUICKKEYPAIR(KC_WEBHOME, KI_BROWSER_HOME))
	(QUICKKEYPAIR(KC_NUMPADCOMMA, KI_DECIMAL)) //
	(QUICKKEYPAIR(KC_DIVIDE, KI_DIVIDE))
	(QUICKKEYPAIR(KC_SYSRQ, KI_UNKNOWN)) //
	(QUICKKEYPAIR(KC_RMENU, KI_RMENU))
	(QUICKKEYPAIR(KC_PAUSE, KI_PAUSE))
	(QUICKKEYPAIR(KC_HOME, KI_HOME))
	(QUICKKEYPAIR(KC_UP, KI_UP))
	(QUICKKEYPAIR(KC_PGUP, KI_PRIOR))
	(QUICKKEYPAIR(KC_LEFT, KI_LEFT))
	(QUICKKEYPAIR(KC_RIGHT, KI_RIGHT))
	(QUICKKEYPAIR(KC_END, KI_END))
	(QUICKKEYPAIR(KC_DOWN, KI_DOWN))
	(QUICKKEYPAIR(KC_PGDOWN, KI_NEXT))
	(QUICKKEYPAIR(KC_INSERT, KI_INSERT))
	(QUICKKEYPAIR(KC_DELETE, KI_DELETE))
	(QUICKKEYPAIR(KC_LWIN, KI_LWIN))
	(QUICKKEYPAIR(KC_RWIN, KI_RWIN))
	(QUICKKEYPAIR(KC_APPS, KI_APPS))
	(QUICKKEYPAIR(KC_POWER, KI_POWER))
	(QUICKKEYPAIR(KC_SLEEP, KI_SLEEP))
	(QUICKKEYPAIR(KC_WAKE, KI_WAKE))
	(QUICKKEYPAIR(KC_WEBSEARCH, KI_BROWSER_SEARCH))
	(QUICKKEYPAIR(KC_WEBFAVORITES, KI_BROWSER_FAVORITES))
	(QUICKKEYPAIR(KC_WEBREFRESH, KI_BROWSER_REFRESH))
	(QUICKKEYPAIR(KC_WEBSTOP, KI_BROWSER_STOP))
	(QUICKKEYPAIR(KC_WEBFORWARD, KI_BROWSER_FORWARD))
	(QUICKKEYPAIR(KC_WEBBACK, KI_BROWSER_BACK))
	(QUICKKEYPAIR(KC_MYCOMPUTER, KI_UNKNOWN)) //
	(QUICKKEYPAIR(KC_MAIL, KI_LAUNCH_MAIL))
	(QUICKKEYPAIR(KC_MEDIASELECT, KI_LAUNCH_MEDIA_SELECT))
;

/*
key_identifiers[OIS::KC_UNASSIGNED] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_ESCAPE] = Rocket::Core::Input::KI_ESCAPE;
key_identifiers[OIS::KC_1] = Rocket::Core::Input::KI_1;
key_identifiers[OIS::KC_2] = Rocket::Core::Input::KI_2;
key_identifiers[OIS::KC_3] = Rocket::Core::Input::KI_3;
key_identifiers[OIS::KC_4] = Rocket::Core::Input::KI_4;
key_identifiers[OIS::KC_5] = Rocket::Core::Input::KI_5;
key_identifiers[OIS::KC_6] = Rocket::Core::Input::KI_6;
key_identifiers[OIS::KC_7] = Rocket::Core::Input::KI_7;
key_identifiers[OIS::KC_8] = Rocket::Core::Input::KI_8;
key_identifiers[OIS::KC_9] = Rocket::Core::Input::KI_9;
key_identifiers[OIS::KC_0] = Rocket::Core::Input::KI_0;
key_identifiers[OIS::KC_MINUS] = Rocket::Core::Input::KI_OEM_MINUS;
key_identifiers[OIS::KC_EQUALS] = Rocket::Core::Input::KI_OEM_PLUS;
key_identifiers[OIS::KC_BACK] = Rocket::Core::Input::KI_BACK;
key_identifiers[OIS::KC_TAB] = Rocket::Core::Input::KI_TAB;
key_identifiers[OIS::KC_Q] = Rocket::Core::Input::KI_Q;
key_identifiers[OIS::KC_W] = Rocket::Core::Input::KI_W;
key_identifiers[OIS::KC_E] = Rocket::Core::Input::KI_E;
key_identifiers[OIS::KC_R] = Rocket::Core::Input::KI_R;
key_identifiers[OIS::KC_T] = Rocket::Core::Input::KI_T;
key_identifiers[OIS::KC_Y] = Rocket::Core::Input::KI_Y;
key_identifiers[OIS::KC_U] = Rocket::Core::Input::KI_U;
key_identifiers[OIS::KC_I] = Rocket::Core::Input::KI_I;
key_identifiers[OIS::KC_O] = Rocket::Core::Input::KI_O;
key_identifiers[OIS::KC_P] = Rocket::Core::Input::KI_P;
key_identifiers[OIS::KC_LBRACKET] = Rocket::Core::Input::KI_OEM_4;
key_identifiers[OIS::KC_RBRACKET] = Rocket::Core::Input::KI_OEM_6;
key_identifiers[OIS::KC_RETURN] = Rocket::Core::Input::KI_RETURN;
key_identifiers[OIS::KC_LCONTROL] = Rocket::Core::Input::KI_LCONTROL;
key_identifiers[OIS::KC_A] = Rocket::Core::Input::KI_A;
key_identifiers[OIS::KC_S] = Rocket::Core::Input::KI_S;
key_identifiers[OIS::KC_D] = Rocket::Core::Input::KI_D;
key_identifiers[OIS::KC_F] = Rocket::Core::Input::KI_F;
key_identifiers[OIS::KC_G] = Rocket::Core::Input::KI_G;
key_identifiers[OIS::KC_H] = Rocket::Core::Input::KI_H;
key_identifiers[OIS::KC_J] = Rocket::Core::Input::KI_J;
key_identifiers[OIS::KC_K] = Rocket::Core::Input::KI_K;
key_identifiers[OIS::KC_L] = Rocket::Core::Input::KI_L;
key_identifiers[OIS::KC_SEMICOLON] = Rocket::Core::Input::KI_OEM_1;
key_identifiers[OIS::KC_APOSTROPHE] = Rocket::Core::Input::KI_OEM_7;
key_identifiers[OIS::KC_GRAVE] = Rocket::Core::Input::KI_OEM_3;
key_identifiers[OIS::KC_LSHIFT] = Rocket::Core::Input::KI_LSHIFT;
key_identifiers[OIS::KC_BACKSLASH] = Rocket::Core::Input::KI_OEM_5;
key_identifiers[OIS::KC_Z] = Rocket::Core::Input::KI_Z;
key_identifiers[OIS::KC_X] = Rocket::Core::Input::KI_X;
key_identifiers[OIS::KC_C] = Rocket::Core::Input::KI_C;
key_identifiers[OIS::KC_V] = Rocket::Core::Input::KI_V;
key_identifiers[OIS::KC_B] = Rocket::Core::Input::KI_B;
key_identifiers[OIS::KC_N] = Rocket::Core::Input::KI_N;
key_identifiers[OIS::KC_M] = Rocket::Core::Input::KI_M;
key_identifiers[OIS::KC_COMMA] = Rocket::Core::Input::KI_OEM_COMMA;
key_identifiers[OIS::KC_PERIOD] = Rocket::Core::Input::KI_OEM_PERIOD;
key_identifiers[OIS::KC_SLASH] = Rocket::Core::Input::KI_OEM_2;
key_identifiers[OIS::KC_RSHIFT] = Rocket::Core::Input::KI_RSHIFT;
key_identifiers[OIS::KC_MULTIPLY] = Rocket::Core::Input::KI_MULTIPLY;
key_identifiers[OIS::KC_LMENU] = Rocket::Core::Input::KI_LMENU;
key_identifiers[OIS::KC_SPACE] = Rocket::Core::Input::KI_SPACE;
key_identifiers[OIS::KC_CAPITAL] = Rocket::Core::Input::KI_CAPITAL;
key_identifiers[OIS::KC_F1] = Rocket::Core::Input::KI_F1;
key_identifiers[OIS::KC_F2] = Rocket::Core::Input::KI_F2;
key_identifiers[OIS::KC_F3] = Rocket::Core::Input::KI_F3;
key_identifiers[OIS::KC_F4] = Rocket::Core::Input::KI_F4;
key_identifiers[OIS::KC_F5] = Rocket::Core::Input::KI_F5;
key_identifiers[OIS::KC_F6] = Rocket::Core::Input::KI_F6;
key_identifiers[OIS::KC_F7] = Rocket::Core::Input::KI_F7;
key_identifiers[OIS::KC_F8] = Rocket::Core::Input::KI_F8;
key_identifiers[OIS::KC_F9] = Rocket::Core::Input::KI_F9;
key_identifiers[OIS::KC_F10] = Rocket::Core::Input::KI_F10;
key_identifiers[OIS::KC_NUMLOCK] = Rocket::Core::Input::KI_NUMLOCK;
key_identifiers[OIS::KC_SCROLL] = Rocket::Core::Input::KI_SCROLL;
key_identifiers[OIS::KC_NUMPAD7] = Rocket::Core::Input::KI_7;
key_identifiers[OIS::KC_NUMPAD8] = Rocket::Core::Input::KI_8;
key_identifiers[OIS::KC_NUMPAD9] = Rocket::Core::Input::KI_9;
key_identifiers[OIS::KC_SUBTRACT] = Rocket::Core::Input::KI_SUBTRACT;
key_identifiers[OIS::KC_NUMPAD4] = Rocket::Core::Input::KI_4;
key_identifiers[OIS::KC_NUMPAD5] = Rocket::Core::Input::KI_5;
key_identifiers[OIS::KC_NUMPAD6] = Rocket::Core::Input::KI_6;
key_identifiers[OIS::KC_ADD] = Rocket::Core::Input::KI_ADD;
key_identifiers[OIS::KC_NUMPAD1] = Rocket::Core::Input::KI_1;
key_identifiers[OIS::KC_NUMPAD2] = Rocket::Core::Input::KI_2;
key_identifiers[OIS::KC_NUMPAD3] = Rocket::Core::Input::KI_3;
key_identifiers[OIS::KC_NUMPAD0] = Rocket::Core::Input::KI_0;
key_identifiers[OIS::KC_DECIMAL] = Rocket::Core::Input::KI_DECIMAL;
key_identifiers[OIS::KC_OEM_102] = Rocket::Core::Input::KI_OEM_102;
key_identifiers[OIS::KC_F11] = Rocket::Core::Input::KI_F11;
key_identifiers[OIS::KC_F12] = Rocket::Core::Input::KI_F12;
key_identifiers[OIS::KC_F13] = Rocket::Core::Input::KI_F13;
key_identifiers[OIS::KC_F14] = Rocket::Core::Input::KI_F14;
key_identifiers[OIS::KC_F15] = Rocket::Core::Input::KI_F15;
key_identifiers[OIS::KC_KANA] = Rocket::Core::Input::KI_KANA;
key_identifiers[OIS::KC_ABNT_C1] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_CONVERT] = Rocket::Core::Input::KI_CONVERT;
key_identifiers[OIS::KC_NOCONVERT] = Rocket::Core::Input::KI_NONCONVERT;
key_identifiers[OIS::KC_YEN] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_ABNT_C2] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_NUMPADEQUALS] = Rocket::Core::Input::KI_OEM_NEC_EQUAL;
key_identifiers[OIS::KC_PREVTRACK] = Rocket::Core::Input::KI_MEDIA_PREV_TRACK;
key_identifiers[OIS::KC_AT] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_COLON] = Rocket::Core::Input::KI_OEM_1;
key_identifiers[OIS::KC_UNDERLINE] = Rocket::Core::Input::KI_OEM_MINUS;
key_identifiers[OIS::KC_KANJI] = Rocket::Core::Input::KI_KANJI;
key_identifiers[OIS::KC_STOP] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_AX] = Rocket::Core::Input::KI_OEM_AX;
key_identifiers[OIS::KC_UNLABELED] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_NEXTTRACK] = Rocket::Core::Input::KI_MEDIA_NEXT_TRACK;
key_identifiers[OIS::KC_NUMPADENTER] = Rocket::Core::Input::KI_NUMPADENTER;
key_identifiers[OIS::KC_RCONTROL] = Rocket::Core::Input::KI_RCONTROL;
key_identifiers[OIS::KC_MUTE] = Rocket::Core::Input::KI_VOLUME_MUTE;
key_identifiers[OIS::KC_CALCULATOR] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_PLAYPAUSE] = Rocket::Core::Input::KI_MEDIA_PLAY_PAUSE;
key_identifiers[OIS::KC_MEDIASTOP] = Rocket::Core::Input::KI_MEDIA_STOP;
key_identifiers[OIS::KC_VOLUMEDOWN] = Rocket::Core::Input::KI_VOLUME_DOWN;
key_identifiers[OIS::KC_VOLUMEUP] = Rocket::Core::Input::KI_VOLUME_UP;
key_identifiers[OIS::KC_WEBHOME] = Rocket::Core::Input::KI_BROWSER_HOME;
key_identifiers[OIS::KC_NUMPADCOMMA] = Rocket::Core::Input::KI_SEPARATOR;
key_identifiers[OIS::KC_DIVIDE] = Rocket::Core::Input::KI_DIVIDE;
key_identifiers[OIS::KC_SYSRQ] = Rocket::Core::Input::KI_SNAPSHOT;
key_identifiers[OIS::KC_RMENU] = Rocket::Core::Input::KI_RMENU;
key_identifiers[OIS::KC_PAUSE] = Rocket::Core::Input::KI_PAUSE;
key_identifiers[OIS::KC_HOME] = Rocket::Core::Input::KI_HOME;
key_identifiers[OIS::KC_UP] = Rocket::Core::Input::KI_UP;
key_identifiers[OIS::KC_PGUP] = Rocket::Core::Input::KI_PRIOR;
key_identifiers[OIS::KC_LEFT] = Rocket::Core::Input::KI_LEFT;
key_identifiers[OIS::KC_RIGHT] = Rocket::Core::Input::KI_RIGHT;
key_identifiers[OIS::KC_END] = Rocket::Core::Input::KI_END;
key_identifiers[OIS::KC_DOWN] = Rocket::Core::Input::KI_DOWN;
key_identifiers[OIS::KC_PGDOWN] = Rocket::Core::Input::KI_NEXT;
key_identifiers[OIS::KC_INSERT] = Rocket::Core::Input::KI_INSERT;
key_identifiers[OIS::KC_DELETE] = Rocket::Core::Input::KI_DELETE;
key_identifiers[OIS::KC_LWIN] = Rocket::Core::Input::KI_LWIN;
key_identifiers[OIS::KC_RWIN] = Rocket::Core::Input::KI_RWIN;
key_identifiers[OIS::KC_APPS] = Rocket::Core::Input::KI_APPS;
key_identifiers[OIS::KC_POWER] = Rocket::Core::Input::KI_POWER;
key_identifiers[OIS::KC_SLEEP] = Rocket::Core::Input::KI_SLEEP;
key_identifiers[OIS::KC_WAKE] = Rocket::Core::Input::KI_WAKE;
key_identifiers[OIS::KC_WEBSEARCH] = Rocket::Core::Input::KI_BROWSER_SEARCH;
key_identifiers[OIS::KC_WEBFAVORITES] = Rocket::Core::Input::KI_BROWSER_FAVORITES;
key_identifiers[OIS::KC_WEBREFRESH] = Rocket::Core::Input::KI_BROWSER_REFRESH;
key_identifiers[OIS::KC_WEBSTOP] = Rocket::Core::Input::KI_BROWSER_STOP;
key_identifiers[OIS::KC_WEBFORWARD] = Rocket::Core::Input::KI_BROWSER_FORWARD;
key_identifiers[OIS::KC_WEBBACK] = Rocket::Core::Input::KI_BROWSER_BACK;
key_identifiers[OIS::KC_MYCOMPUTER] = Rocket::Core::Input::KI_UNKNOWN;
key_identifiers[OIS::KC_MAIL] = Rocket::Core::Input::KI_LAUNCH_MAIL;
key_identifiers[OIS::KC_MEDIASELECT] = Rocket::Core::Input::KI_LAUNCH_MEDIA_SELECT;

*/