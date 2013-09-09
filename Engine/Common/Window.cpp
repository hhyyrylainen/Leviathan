#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_WINDOW
#include "Window.h"
#endif
#include "Engine.h"
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::Window::Window(Ogre::RenderWindow* owindow, bool vsync) : OWindow(owindow), VerticalSync(vsync), m_hwnd(NULL){

	// update focused this way //
	Focused = (m_hwnd = GetRenderWindowHandle(OWindow)) == GetForegroundWindow() ? true: false;

	// register as listener to get update notifications //
	Ogre::WindowEventUtilities::addWindowEventListener(OWindow, this);

	// cursor on top of window's windows isn't hidden //
	CursorHidden = false;
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
}

DLLEXPORT void Leviathan::Window::ResizeWindow(const int &width, const int &height){
	// make ogre window resize //
	OWindow->resize(width, height);
}
// ------------------------------------ //
void Leviathan::Window::windowResized(Ogre::RenderWindow* rw){
	// notify engine //
	Engine::GetEngine()->OnResize(GetWidth(), GetHeight());
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
// ------------------ WindowPassData ------------------ //
Leviathan::WindowPassData::WindowPassData(Window* wind, LeviathanApplication* appinterface){
	OwningWindow = wind;
	Appinterface = appinterface;
}
