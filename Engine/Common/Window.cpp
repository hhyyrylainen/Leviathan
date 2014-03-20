#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_WINDOW
#include "Window.h"
#endif
#include "Engine.h"
#include <boost/assign/list_of.hpp>
#include "Exceptions/ExceptionNotFound.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "include/cef_browser.h"
#include "chromium/KeyboardCodes.h"
#include "include/cef_keyboard_handler.h"
#include "GlobalCEFHandler.h"
using namespace Leviathan;
// ------------------------------------ //

#ifdef _WIN32
// we must have an int of size 32 bits //
#pragma intrinsic(_BitScanForward)

static_assert(sizeof(int) == 4, "int must be 4 bytes long for bit scan function");
#else
// We must use GCC built ins
// int __builtin_ffs (unsigned int x) Returns one plus the index of the least significant 1-bit of x, or if x is zero, returns zero.
// So using __builtin_ffs(val)-1 should work

		// X11 window focus find function //
X11::XID Leviathan::Window::GetForegroundWindow(){
	// Method posted on stack overflow http://stackoverflow.com/questions/1014822/how-to-know-which-window-has-focus-and-how-to-change-it
	using namespace X11;

	XID win;

	int revert_to;
	XGetInputFocus(XDisplay, &win, &revert_to); // see man

	return win;
}


#endif

#if defined(OS_LINUX)
#include <gdk/gdkkeysyms.h>
#endif

#if defined(OS_MACOSX)
#include <Carbon/Carbon.h>
#endif

// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
#if defined(OS_MACOSX)
// A convenient array for getting symbol characters on the number keys.
const char kShiftCharsForNumberKeys[] = ")!@#$%^&*(";

// Convert an ANSI character to a Mac key code.
int GetMacKeyCodeFromChar(int key_char) {
	switch (key_char) {
	case ' ': return kVK_Space;

	case '0': case ')': return kVK_ANSI_0;
	case '1': case '!': return kVK_ANSI_1;
	case '2': case '@': return kVK_ANSI_2;
	case '3': case '#': return kVK_ANSI_3;
	case '4': case '$': return kVK_ANSI_4;
	case '5': case '%': return kVK_ANSI_5;
	case '6': case '^': return kVK_ANSI_6;
	case '7': case '&': return kVK_ANSI_7;
	case '8': case '*': return kVK_ANSI_8;
	case '9': case '(': return kVK_ANSI_9;

	case 'a': case 'A': return kVK_ANSI_A;
	case 'b': case 'B': return kVK_ANSI_B;
	case 'c': case 'C': return kVK_ANSI_C;
	case 'd': case 'D': return kVK_ANSI_D;
	case 'e': case 'E': return kVK_ANSI_E;
	case 'f': case 'F': return kVK_ANSI_F;
	case 'g': case 'G': return kVK_ANSI_G;
	case 'h': case 'H': return kVK_ANSI_H;
	case 'i': case 'I': return kVK_ANSI_I;
	case 'j': case 'J': return kVK_ANSI_J;
	case 'k': case 'K': return kVK_ANSI_K;
	case 'l': case 'L': return kVK_ANSI_L;
	case 'm': case 'M': return kVK_ANSI_M;
	case 'n': case 'N': return kVK_ANSI_N;
	case 'o': case 'O': return kVK_ANSI_O;
	case 'p': case 'P': return kVK_ANSI_P;
	case 'q': case 'Q': return kVK_ANSI_Q;
	case 'r': case 'R': return kVK_ANSI_R;
	case 's': case 'S': return kVK_ANSI_S;
	case 't': case 'T': return kVK_ANSI_T;
	case 'u': case 'U': return kVK_ANSI_U;
	case 'v': case 'V': return kVK_ANSI_V;
	case 'w': case 'W': return kVK_ANSI_W;
	case 'x': case 'X': return kVK_ANSI_X;
	case 'y': case 'Y': return kVK_ANSI_Y;
	case 'z': case 'Z': return kVK_ANSI_Z;

		// U.S. Specific mappings.  Mileage may vary.
	case ';': case ':': return kVK_ANSI_Semicolon;
	case '=': case '+': return kVK_ANSI_Equal;
	case ',': case '<': return kVK_ANSI_Comma;
	case '-': case '_': return kVK_ANSI_Minus;
	case '.': case '>': return kVK_ANSI_Period;
	case '/': case '?': return kVK_ANSI_Slash;
	case '`': case '~': return kVK_ANSI_Grave;
	case '[': case '{': return kVK_ANSI_LeftBracket;
	case '\\': case '|': return kVK_ANSI_Backslash;
	case ']': case '}': return kVK_ANSI_RightBracket;
	case '\'': case '"': return kVK_ANSI_Quote;
	}

	return -1;
}
#endif  // defined(OS_MACOSX)

// ------------------ End of CEF code ------------------ //



// ------------------ Window ------------------ //
DLLEXPORT Leviathan::Window::Window(Ogre::RenderWindow* owindow, GraphicalInputEntity* owner) : OWindow(owindow),
	WindowsInputManager(NULL), WindowMouse(NULL), WindowKeyboard(NULL), LastFrameDownMouseButtons(0),
	ForceMouseVisible(false), CursorState(true), MouseCaptured(false), FirstInput(true), InputProcessedByCEF(false)
#ifdef __GNUC__
	, XDisplay(NULL), m_hwnd(0)
#else
	, m_hwnd(NULL)
#endif
{

	OwningWindow = owner;
	// update focused this way //
	VerifyRenderWindowHandle();
	Focused = m_hwnd == GetForegroundWindow() ? true: false;

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
			Logger::Get()->Info(L"Showing cursor");
#ifdef _WIN32
			ShowCursor(TRUE);
#else
			// Restore default cursor //
			XUndefineCursor(XDisplay, m_hwnd);
#endif
		}
	} else {
		// hide cursor //
		if(CursorState){
			CursorState = false;
			Logger::Get()->Info(L"Hiding cursor");
#ifdef _WIN32
			ShowCursor(FALSE);
#else
			// Set nothing as our cursor
			XDefineCursor(XDisplay, m_hwnd, 0);
#endif
		}
	}
}

#ifdef _WIN32
DLLEXPORT void Leviathan::Window::SetMouseToCenter(){
	// update window handle before using //
	VerifyRenderWindowHandle();

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
#else
DLLEXPORT void Leviathan::Window::SetMouseToCenter(){

	VerifyRenderWindowHandle();

	if(m_hwnd == NULL){
		// window has closed //
		return;
	}

	// Use the X11 function to warp the cursor //
	XWarpPointer(XDisplay, 0, m_hwnd, 0, 0, 0, 0, GetWidth()/2, GetHeight()/2);
}
#endif

#ifdef _WIN32
DLLEXPORT void Leviathan::Window::GetRelativeMouse(int& x, int& y){
	// update window handle before using //
	VerifyRenderWindowHandle();
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
#else
DLLEXPORT void Leviathan::Window::GetRelativeMouse(int& x, int& y){
	// To not segment fault on this, we need to pass in dummy pointers to unused values //
	X11::XID spammyme;
	int spamme;
	unsigned int spammetoo;
	// Should return the right position //
	XQueryPointer(XDisplay, m_hwnd, &spammyme, &spammyme, &spamme, &spamme, &x, &y, &spammetoo);
}
#endif


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
	// \todo add callback notification
	OwningWindow->OnResize(GetWidth(), GetHeight());

	UpdateOISMouseWindowSize();
}

void Leviathan::Window::windowFocusChange(Ogre::RenderWindow* rw){
	// update handle to have it up to date //
	VerifyRenderWindowHandle();

	//Focused = m_hwnd == GetFocus() ? true: false;
	Focused = m_hwnd == GetForegroundWindow() ? true: false;

	// update mouse //
	_CheckMouseVisibilityStates();

	OwningWindow->OnFocusChange(Focused);
}
// ------------------------------------ //
#ifdef _WIN32
bool Leviathan::Window::VerifyRenderWindowHandle(){

	void* WindowHwnd(0);

	OWindow->getCustomAttribute(Ogre::String("WINDOW"), &WindowHwnd);

	m_hwnd = reinterpret_cast<HWND>(WindowHwnd);

	if(!IsWindow(m_hwnd)){
		// not a window! //
		return false;
	}

	return true;
}
#else
bool Leviathan::Window::VerifyRenderWindowHandle(){

	void* xidval(0);

	OWindow->getCustomAttribute(Ogre::String("WINDOW"), &xidval);

	m_hwnd = reinterpret_cast<X11::XID>(xidval);
	// We need the display too //
	void* xdisplay(0);

	OWindow->getCustomAttribute(Ogre::String("DISPLAY"), &xdisplay);

	XDisplay = reinterpret_cast<X11::Display*>(xdisplay);

	return true;
}
#endif
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

DLLEXPORT void Leviathan::Window::GatherInput(CefRefPtr<CefBrowserHost> browserinput){
	// quit if window closed //
	if(OWindow->isClosed() || !WindowKeyboard || !WindowMouse){

		Logger::Get()->Warning(L"Window: GatherInput: skipping due to closed input window");
		return;
	}

	inputreceiver = browserinput;

	// on first frame we want to manually force mouse position send //
	if(FirstInput){
		FirstInput = false;

		_CheckMouseVisibilityStates();
	}

	// set parameters that listener functions need //
	ThisFrameHandledCreate = false;

	// We should automatically be the input handler for this browser //

	// Capture the browser variable //
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

	const OIS::MouseState& mstate = WindowMouse->getMouseState();

	// create keyboard special key states here //
	SpecialKeyModifiers = 0;

	if(WindowKeyboard->isModifierDown(OIS::Keyboard::Ctrl))
		SpecialKeyModifiers |= EVENTFLAG_CONTROL_DOWN;
	if(WindowKeyboard->isModifierDown(OIS::Keyboard::Alt))
		SpecialKeyModifiers |= EVENTFLAG_ALT_DOWN;
	if(WindowKeyboard->isModifierDown(OIS::Keyboard::Shift))
		SpecialKeyModifiers |= EVENTFLAG_SHIFT_DOWN;
	if(WindowKeyboard->isKeyDown(OIS::KC_CAPITAL))
		SpecialKeyModifiers |= EVENTFLAG_CAPS_LOCK_ON;
	if(WindowKeyboard->isKeyDown(OIS::KC_LWIN))
		SpecialKeyModifiers |= EVENTFLAG_COMMAND_DOWN;
	if(WindowKeyboard->isKeyDown(OIS::KC_NUMLOCK))
		SpecialKeyModifiers |= EVENTFLAG_NUM_LOCK_ON;
	if(WindowKeyboard->isKeyDown(OIS::KC_LEFT))
		SpecialKeyModifiers |= EVENTFLAG_IS_LEFT;
	if(WindowKeyboard->isKeyDown(OIS::KC_RIGHT))
		SpecialKeyModifiers |= EVENTFLAG_IS_RIGHT;
	if(mstate.buttonDown(OIS::MB_Left))
		SpecialKeyModifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
	if(mstate.buttonDown(OIS::MB_Right))
		SpecialKeyModifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
	if(mstate.buttonDown(OIS::MB_Middle))
		SpecialKeyModifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
	// TODO: add 
	// EVENTFLAG_IS_KEY_PAD

	ThisFrameHandledCreate = true;
}
// ------------------ Input listener functions ------------------ //
void Leviathan::Window::DoCEFInputPass(const OIS::KeyEvent &arg, bool down){

	CefKeyEvent cef_event;

	cef_event.modifiers = SpecialKeyModifiers;
	InputProcessedByCEF = false;

//	int text;
//
//	if(arg.text == 0){
//		// We need to translate it ourselves //
//#ifdef _WIN32
//
//		text = MapVirtualKey(OISVKeyConvert[arg.key], MAPVK_VK_TO_CHAR);
//
//#endif // _WIN32
//
//	} else {
//		text = arg.text;
//	}

	// More slightly modified CEF (chromium embedded framework) code, see the license higher in this file (line 50ish) //
#if defined(OS_WIN)

	//BYTE vkey = LOBYTE(VkKeyScan(text));
	BYTE vkey = OISVKeyConvert[arg.key];
	UINT scanCode = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
	cef_event.native_key_code = (scanCode << 16) |  // key scan code
		1;  // key repeat count
#elif defined(OS_LINUX) || defined(OS_MACOSX)

#if defined(OS_LINUX)
	if (arg.key == OIS::KC_BACK)
		cef_event.native_key_code = GDK_BackSpace;
	else if (arg.key == OIS::KC_DELETE)
		cef_event.native_key_code = GDK_Delete;
	else if (arg.key == OIS::KC_DOWN)
		cef_event.native_key_code = GDK_Down;
	else if (arg.key == OIS::KC_ENTER)
		cef_event.native_key_code = GDK_KEY_KP_Enter;
	else if (arg.key == OIS::KC_ESCAPE)
		cef_event.native_key_code = GDK_Escape;
	else if (arg.key == OIS::KC_LEFT)
		cef_event.native_key_code = GDK_Left;
	else if (arg.key == OIS::KC_RIGHT)
		cef_event.native_key_code = GDK_Right;
	else if (arg.key == OIS::KC_TAB)
		cef_event.native_key_code = GDK_Tab;
	else if (arg.key == OIS::KC_UP)
		cef_event.native_key_code = GDK_Up;
	else
		cef_event.native_key_code = key_char;
#elif defined(OS_MACOSX)
	if (arg.key == OIS::KC_BACK) {
		cef_event.native_key_code = kVK_Delete;
		cef_event.unmodified_character = kBackspaceCharCode;
	} else if (arg.key == OIS::KC_DELETE) {
		cef_event.native_key_code = kVK_ForwardDelete;
		cef_event.unmodified_character = kDeleteCharCode;
	} else if (arg.key == OIS::KC_DOWN) {
		cef_event.native_key_code = kVK_DownArrow;
		cef_event.unmodified_character = /* NSDownArrowFunctionKey */ 0xF701;
	} else if (arg.key == OIS::KC_RETURN) {
		cef_event.native_key_code = kVK_Return;
		cef_event.unmodified_character = kReturnCharCode;
	} else if (arg.key == OIS::KC_ESCAPE) {
		cef_event.native_key_code = kVK_Escape;
		cef_event.unmodified_character = kEscapeCharCode;
	} else if (arg.key == OIS::KC_LEFT) {
		cef_event.native_key_code = kVK_LeftArrow;
		cef_event.unmodified_character = /* NSLeftArrowFunctionKey */ 0xF702;
	} else if (arg.key == OIS::KC_RIGHT) {
		cef_event.native_key_code = kVK_RightArrow;
		cef_event.unmodified_character = /* NSRightArrowFunctionKey */ 0xF703;
	} else if (arg.key == OIS::KC_TAB) {
		cef_event.native_key_code = kVK_Tab;
		cef_event.unmodified_character = kTabCharCode;
	} else if (arg.key == OIS::KC_UP) {
		cef_event.native_key_code = kVK_UpArrow;
		cef_event.unmodified_character = /* NSUpArrowFunctionKey */ 0xF700;
	} else {
		cef_event.native_key_code = GetMacKeyCodeFromChar(key_char);
		if (cef_event.native_key_code == -1)
			return;

		cef_event.unmodified_character = key_char;
	}

	cef_event.character = cef_event.unmodified_character;

	// Fill in |character| according to flags.
	if (cef_event.modifiers & EVENTFLAG_SHIFT_DOWN) {
		if (key_char >= '0' && key_char <= '9') {
			cef_event.character = kShiftCharsForNumberKeys[key_char - '0'];
		} else if (key_char >= 'A' && key_char <= 'Z') {
			cef_event.character = 'A' + (key_char - 'A');
		} else {
			switch (cef_event.native_key_code) {
			case kVK_ANSI_Grave:
				cef_event.character = '~';
				break;
			case kVK_ANSI_Minus:
				cef_event.character = '_';
				break;
			case kVK_ANSI_Equal:
				cef_event.character = '+';
				break;
			case kVK_ANSI_LeftBracket:
				cef_event.character = '{';
				break;
			case kVK_ANSI_RightBracket:
				cef_event.character = '}';
				break;
			case kVK_ANSI_Backslash:
				cef_event.character = '|';
				break;
			case kVK_ANSI_Semicolon:
				cef_event.character = ':';
				break;
			case kVK_ANSI_Quote:
				cef_event.character = '\"';
				break;
			case kVK_ANSI_Comma:
				cef_event.character = '<';
				break;
			case kVK_ANSI_Period:
				cef_event.character = '>';
				break;
			case kVK_ANSI_Slash:
				cef_event.character = '?';
				break;
			default:
				break;
			}
		}
	}

	// Control characters.
	if (cef_event.modifiers & EVENTFLAG_CONTROL_DOWN) {
		if (key_char >= 'A' && key_char <= 'Z')
			cef_event.character = 1 + key_char - 'A';
		else if (cef_event.native_key_code == kVK_ANSI_LeftBracket)
			cef_event.character = 27;
		else if (cef_event.native_key_code == kVK_ANSI_Backslash)
			cef_event.character = 28;
		else if (cef_event.native_key_code == kVK_ANSI_RightBracket)
			cef_event.character = 29;
	}
#endif  // defined(OS_MACOSX)
#endif  // defined(OS_LINUX) || defined(OS_MACOSX)

	if (down) {
#if defined(OS_WIN)
		cef_event.windows_key_code = vkey;
#endif
		cef_event.type = KEYEVENT_RAWKEYDOWN;

		// Pass the first //
		inputreceiver->SendKeyEvent(cef_event);

		// Send char event //
		if(arg.text == 0)
			return;
#if defined(OS_WIN)
		cef_event.windows_key_code = arg.text;
#endif
		cef_event.type = KEYEVENT_CHAR;

	} else {
#if defined(OS_WIN)
		cef_event.windows_key_code = vkey;
		// bits 30 and 31 should always be 1 for WM_KEYUP
		cef_event.native_key_code |= 0xC0000000;
#endif
		cef_event.type = KEYEVENT_KEYUP;
	}

	// Pass it //
	inputreceiver->SendKeyEvent(cef_event);
}

bool Leviathan::Window::keyPressed(const OIS::KeyEvent &arg){
	CheckInputState();

	bool SentToController = false;

	// Try to pass to CEF //
	DoCEFInputPass(arg, true);


	// Check is it now handled or not and continue //
	if(!InputProcessedByCEF){

		// Finally try sending it to GUI //
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
	
	// Send to CEF if GUI is active //
	DoCEFInputPass(arg, false);


	// This should always be passed here //
	OwningWindow->GetInputController()->OnInputGet(arg.key, SpecialKeyModifiers, false);



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
		CefMouseEvent mevent;
		mevent.modifiers = SpecialKeyModifiers;
		mevent.x = mstate.X.abs;
		mevent.y = mstate.Y.abs;

		inputreceiver->SendMouseMoveEvent(mevent, IsMouseOutsideWindowClientArea());

		CefMouseEvent second;
		mevent.modifiers = SpecialKeyModifiers;
		second.x = 0;
		second.y = 0;

		inputreceiver->SendMouseWheelEvent(second, 0, mstate.Z.rel); 
	}
	_CheckMouseVisibilityStates();

	// don't really know what to return
	return true;
}

bool Leviathan::Window::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
	CheckInputState();

	const OIS::MouseState& mstate = arg.state;

	// pass event to active Rocket context //
	int differences = mstate.buttons^LastFrameDownMouseButtons;

	// find differences //
#ifdef _WIN32
	unsigned long index = 0;

	_BitScanForward(&index, differences);

#else
	int index = __builtin_ffs(differences)-1;
#endif

	// update old state //
	LastFrameDownMouseButtons |= 1 << index;


	int Keynumber = index;
	if(!MouseCaptured){

		CefMouseEvent mevent;
		mevent.modifiers = SpecialKeyModifiers;
		mevent.x = mstate.X.abs;
		mevent.y = mstate.Y.abs;

		cef_mouse_button_type_t btype;

		if(Keynumber == 0){
			btype = MBT_LEFT;

		} else if(Keynumber == 1){
			btype = MBT_RIGHT;

		} else if(Keynumber == 2){
			btype = MBT_MIDDLE;

		} else {
			// We actually don't want to pass this //
			return true;
		}

		inputreceiver->SendMouseClickEvent(mevent, btype, false, 1);
	}
		

	_CheckMouseVisibilityStates();

	// don't really know what to return
	return true;
}

bool Leviathan::Window::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
	CheckInputState();

	const OIS::MouseState& mstate = arg.state;

	// pass event to active Rocket context //
	int differences = arg.state.buttons^LastFrameDownMouseButtons;

	// find differences //
#ifdef _WIN32
	unsigned long index = 0;

	_BitScanForward(&index, differences);

#else
	int index = __builtin_ffs(differences)-1;
#endif

	// update old state //
	LastFrameDownMouseButtons ^= 1 << index;

	int Keynumber = index;

	CefMouseEvent mevent;
	mevent.modifiers = SpecialKeyModifiers;
	mevent.x = mstate.X.abs;
	mevent.y = mstate.Y.abs;

	cef_mouse_button_type_t btype;

	if(Keynumber == 0){
		btype = MBT_LEFT;

	} else if(Keynumber == 1){
		btype = MBT_RIGHT;

	} else if(Keynumber == 2){
		btype = MBT_MIDDLE;

	} else {
		// We actually don't want to pass this //
		return true;
	}

	inputreceiver->SendMouseClickEvent(mevent, btype, true, 1);

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
	OverlayScene = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_INTERIOR, "Overlay_forWindow_");

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
	// Update focused here //
	VerifyRenderWindowHandle();
	Focused = m_hwnd == GetForegroundWindow() ? true: false;

	// force cursor visible check (if outside client area or mouse is unfocused on the window) //
	if(IsMouseOutsideWindowClientArea() || !Focused){

		ForceMouseVisible = true;
	} else {

		ForceMouseVisible = false;
	}
	// update cursor state //
	SetHideCursor(ApplicationWantCursorState);
}

void Leviathan::Window::ReportKeyEventAsUsed(){
	InputProcessedByCEF = true;
}
// ------------------------------------ //
DLLEXPORT Int4 Leviathan::Window::GetScreenPixelRect() const THROWS{
#ifdef _WIN32
	RECT rect;
	// Call windows api to get this //
	if(GetWindowRect(m_hwnd, &rect)){
		// We need to translate the end coordinates to width and height //
		return Int4(rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top);
	}
	// Failed //
	throw ExceptionNotFound(L"could not get window rect, call failed", GetLastError(), __WFUNCTION__, L"invalid handle", L"m_hwnd");

#else
	return Int4(0, 0, GetWidth(), GetHeight());
#endif // _WIN32
}


DLLEXPORT Int2 Leviathan::Window::TranslateClientPointToScreenPoint(const Int2 &point) const THROWS{
#ifdef _WIN32

	POINT pt = {point.X, point.Y};
	// Windows api to the rescue //
	if(ClientToScreen(m_hwnd, &pt)){
		// Succeeded //
		return Int2(pt.x, pt.y);
	}

	// Failed //
	throw ExceptionNotFound(L"could not translate point to screen space", GetLastError(), __WFUNCTION__, L"invalid handle", L"m_hwnd");
#else
	return point;
#endif // _WIN32
}



// ------------------ KeyCode conversion map ------------------ //
#define QUICKKEYPAIR(x, y) OIS::x, WebCore::y
#define SIMPLEPAIR(x, y)	L##x, OIS::y
#define QUICKONETOONEPAIR(x) OIS::KC_##x, WebCore::VKEY_##x


#define SIMPLEONETOONE(x)	WSTRINGIFY(x), OIS::KC_##x

boost::bimap<wstring, OIS::KeyCode> Leviathan::Window::CharacterToOISConvert = boost::assign::list_of<boost::bimap<wstring, OIS::KeyCode>::relation>
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

	// In a bidirectional map keys and values need to be unique //
	//(SIMPLEONETOONE(ESCAPE))
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

	auto iter = CharacterToOISConvert.left.find(str);

	if(iter == CharacterToOISConvert.left.end()){

		throw ExceptionNotFound(L"value not found", 0, __WFUNCTION__, L"str", str);
	}

	return iter->second;
}

DLLEXPORT wstring Leviathan::Window::ConvertOISKeyCodeToWstring(const OIS::KeyCode &code){
	
	auto iter = CharacterToOISConvert.right.find(code);

	if(iter == CharacterToOISConvert.right.end()){


		throw ExceptionNotFound(L"value not found", 0, __WFUNCTION__, L"code", Convert::ToWstring(code));
	}

	return iter->second;
}



map<OIS::KeyCode, int> Leviathan::Window::OISVKeyConvert = boost::assign::map_list_of
	(QUICKKEYPAIR(KC_UNASSIGNED, VKEY_UNKNOWN))
	(QUICKONETOONEPAIR(ESCAPE))
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
	(QUICKKEYPAIR(KC_MINUS, VKEY_OEM_MINUS))
	(QUICKKEYPAIR(KC_EQUALS, VKEY_OEM_PLUS)) //
	(QUICKKEYPAIR(KC_BACK, VKEY_BACK))
	(QUICKKEYPAIR(KC_TAB, VKEY_TAB))
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
	(QUICKKEYPAIR(KC_LBRACKET, VKEY_OEM_4))
	(QUICKKEYPAIR(KC_RBRACKET, VKEY_OEM_6))
	(QUICKKEYPAIR(KC_RETURN, VKEY_RETURN))
	(QUICKKEYPAIR(KC_LCONTROL, VKEY_LCONTROL))
	(QUICKONETOONEPAIR(A))
	(QUICKONETOONEPAIR(S))
	(QUICKONETOONEPAIR(D))
	(QUICKONETOONEPAIR(F))
	(QUICKONETOONEPAIR(G))
	(QUICKONETOONEPAIR(H))
	(QUICKONETOONEPAIR(J))
	(QUICKONETOONEPAIR(K))
	(QUICKONETOONEPAIR(L))
	(QUICKKEYPAIR(KC_SEMICOLON, VKEY_OEM_1))
	(QUICKKEYPAIR(KC_APOSTROPHE, VKEY_OEM_7)) //
	(QUICKKEYPAIR(KC_GRAVE, VKEY_OEM_3))
	(QUICKKEYPAIR(KC_LSHIFT, VKEY_LSHIFT))
	(QUICKKEYPAIR(KC_BACKSLASH, VKEY_OEM_5))
	(QUICKONETOONEPAIR(Z))
	(QUICKONETOONEPAIR(X))
	(QUICKONETOONEPAIR(C))
	(QUICKONETOONEPAIR(V))
	(QUICKONETOONEPAIR(B))
	(QUICKONETOONEPAIR(N))
	(QUICKONETOONEPAIR(M))
	(QUICKKEYPAIR(KC_COMMA, VKEY_OEM_COMMA))
	(QUICKKEYPAIR(KC_PERIOD, VKEY_OEM_PERIOD))
	(QUICKKEYPAIR(KC_SLASH, VKEY_OEM_2))
	(QUICKKEYPAIR(KC_RSHIFT, VKEY_RSHIFT))
	(QUICKKEYPAIR(KC_MULTIPLY, VKEY_MULTIPLY))
	(QUICKKEYPAIR(KC_LMENU, VKEY_LMENU))
	(QUICKKEYPAIR(KC_SPACE, VKEY_SPACE))
	(QUICKKEYPAIR(KC_CAPITAL, VKEY_CAPITAL))
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
	(QUICKKEYPAIR(KC_NUMLOCK, VKEY_NUMLOCK))
	(QUICKKEYPAIR(KC_SCROLL, VKEY_SCROLL))
	(QUICKONETOONEPAIR(NUMPAD7))
	(QUICKONETOONEPAIR(NUMPAD8))
	(QUICKONETOONEPAIR(NUMPAD9))
	(QUICKKEYPAIR(KC_SUBTRACT, VKEY_SUBTRACT))
	(QUICKONETOONEPAIR(NUMPAD4))
	(QUICKONETOONEPAIR(NUMPAD5))
	(QUICKONETOONEPAIR(NUMPAD6))
	(QUICKKEYPAIR(KC_ADD, VKEY_ADD))
	(QUICKONETOONEPAIR(NUMPAD1))
	(QUICKONETOONEPAIR(NUMPAD2))
	(QUICKONETOONEPAIR(NUMPAD3))
	(QUICKONETOONEPAIR(NUMPAD9))
	(QUICKKEYPAIR(KC_DECIMAL, VKEY_DECIMAL))
	(QUICKKEYPAIR(KC_OEM_102, VKEY_OEM_102))
	(QUICKONETOONEPAIR(F11))
	(QUICKONETOONEPAIR(F12))
	(QUICKONETOONEPAIR(F13))
	(QUICKONETOONEPAIR(F14))
	(QUICKONETOONEPAIR(F15))
	(QUICKKEYPAIR(KC_OEM_102, VKEY_OEM_102))
	(QUICKKEYPAIR(KC_KANA, VKEY_KANA))
	(QUICKKEYPAIR(KC_ABNT_C1, VKEY_OEM_2)) //
	(QUICKKEYPAIR(KC_CONVERT, VKEY_CONVERT))
	(QUICKKEYPAIR(KC_NOCONVERT, VKEY_NONCONVERT))
	(QUICKKEYPAIR(KC_YEN, VKEY_UNKNOWN)) //
	(QUICKKEYPAIR(KC_ABNT_C2, VKEY_DECIMAL))
	(QUICKKEYPAIR(KC_NUMPADEQUALS, VKEY_RETURN))
	(QUICKKEYPAIR(KC_PREVTRACK, VKEY_MEDIA_PREV_TRACK))
	(QUICKKEYPAIR(KC_AT, VKEY_OEM_8))
	(QUICKKEYPAIR(KC_COLON, VKEY_OEM_1)) //
	(QUICKKEYPAIR(KC_UNDERLINE, VKEY_OEM_MINUS)) //
	(QUICKKEYPAIR(KC_KANJI, VKEY_KANJI))
	(QUICKKEYPAIR(KC_STOP, VKEY_BROWSER_STOP)) //
	(QUICKKEYPAIR(KC_AX, VKEY_UNKNOWN))
	(QUICKKEYPAIR(KC_UNLABELED, VKEY_UNKNOWN))
	(QUICKKEYPAIR(KC_NEXTTRACK, VKEY_MEDIA_NEXT_TRACK))
	(QUICKKEYPAIR(KC_NUMPADENTER, VKEY_RETURN))
	(QUICKKEYPAIR(KC_RCONTROL, VKEY_RCONTROL))
	(QUICKKEYPAIR(KC_MUTE, VKEY_VOLUME_MUTE))
	(QUICKKEYPAIR(KC_CALCULATOR, VKEY_UNKNOWN))
	(QUICKKEYPAIR(KC_PLAYPAUSE, VKEY_MEDIA_PLAY_PAUSE))
	(QUICKKEYPAIR(KC_MEDIASTOP, VKEY_MEDIA_STOP))
	(QUICKKEYPAIR(KC_VOLUMEDOWN, VKEY_VOLUME_DOWN))
	(QUICKKEYPAIR(KC_VOLUMEUP, VKEY_VOLUME_UP))
	(QUICKKEYPAIR(KC_WEBHOME, VKEY_BROWSER_HOME))
	(QUICKKEYPAIR(KC_NUMPADCOMMA, VKEY_DECIMAL)) //
	(QUICKKEYPAIR(KC_DIVIDE, VKEY_DIVIDE))
	(QUICKKEYPAIR(KC_SYSRQ, VKEY_UNKNOWN)) //
	(QUICKKEYPAIR(KC_RMENU, VKEY_RMENU))
	(QUICKKEYPAIR(KC_PAUSE, VKEY_PAUSE))
	(QUICKKEYPAIR(KC_HOME, VKEY_HOME))
	(QUICKKEYPAIR(KC_UP, VKEY_UP))
	(QUICKKEYPAIR(KC_PGUP, VKEY_PRIOR))
	(QUICKKEYPAIR(KC_LEFT, VKEY_LEFT))
	(QUICKKEYPAIR(KC_RIGHT, VKEY_RIGHT))
	(QUICKKEYPAIR(KC_END, VKEY_END))
	(QUICKKEYPAIR(KC_DOWN, VKEY_DOWN))
	(QUICKKEYPAIR(KC_PGDOWN, VKEY_NEXT))
	(QUICKKEYPAIR(KC_INSERT, VKEY_INSERT))
	(QUICKKEYPAIR(KC_DELETE, VKEY_DELETE))
	(QUICKKEYPAIR(KC_LWIN, VKEY_LWIN))
	(QUICKKEYPAIR(KC_RWIN, VKEY_RWIN))
	(QUICKKEYPAIR(KC_APPS, VKEY_APPS))
	(QUICKKEYPAIR(KC_POWER, VKEY_UNKNOWN))
	(QUICKKEYPAIR(KC_SLEEP, VKEY_SLEEP))
	(QUICKKEYPAIR(KC_WAKE, VKEY_UNKNOWN))
	(QUICKKEYPAIR(KC_WEBSEARCH, VKEY_BROWSER_SEARCH))
	(QUICKKEYPAIR(KC_WEBFAVORITES, VKEY_BROWSER_FAVORITES))
	(QUICKKEYPAIR(KC_WEBREFRESH, VKEY_BROWSER_REFRESH))
	(QUICKKEYPAIR(KC_WEBSTOP, VKEY_BROWSER_STOP))
	(QUICKKEYPAIR(KC_WEBFORWARD, VKEY_BROWSER_FORWARD))
	(QUICKKEYPAIR(KC_WEBBACK, VKEY_BROWSER_BACK))
	(QUICKKEYPAIR(KC_MYCOMPUTER, VKEY_UNKNOWN)) //
	(QUICKKEYPAIR(KC_MAIL, VKEY_MEDIA_LAUNCH_MAIL))
	(QUICKKEYPAIR(KC_MEDIASELECT, VKEY_MEDIA_LAUNCH_MEDIA_SELECT))
;
