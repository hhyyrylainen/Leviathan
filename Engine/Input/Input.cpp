#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_INPUT
#include "Input.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Engine.h"


Input::Input(){

	//pDirectInput = NULL;
	//pKeyboard = NULL;
	//pMouse = NULL;

	//wchar_t KeyStates[256];
	//DIMOUSESTATE MouseState;

	//int ScreenWidth, ScreenHeight;
	MouseX = 0;
	MouseY = 0;
	MouseMoveX = 0;
	MouseMoveY = 0;
}
Input::~Input(){

}
// ------------------------------------ //
bool Input::Init(HINSTANCE thisprocess, int screenwidth, int screenheight){
	//HRESULT hr = S_OK;

	// store size //
	ScreenWidth = screenwidth;
	ScreenHeight = screenheight;
	// mouse should be at 0,0

	//// initialize main interface //
	//hr = DirectInput8Create(thisprocess, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&pDirectInput, NULL);
	//if(FAILED(hr)){

	//	Logger::Get()->Error(L"Failed to init Input object, DirectInput8Create failed", hr);
	//	return false;
	//}

	//// initialize keyboard object //
	//hr = pDirectInput->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL);
	//if(FAILED(hr)){

	//	Logger::Get()->Error(L"Failed to init Input object, failed to create keyboard object", hr);
	//	return false;
	//}
	//// set data format //
	//hr = pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	//if(FAILED(hr)){

	//	Logger::Get()->Error(L"Failed to init Input object, failed to set keyboard data format", hr);
	//	return false;
	//}

	//// set cooperative level //
	////hr = pKeyboard->SetCooperativeLevel(Engine::GetEngine()->GetWindow()->GetHandle(), DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	//hr = pKeyboard->SetCooperativeLevel(Engine::GetEngine()->GetWindow()->GetHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	//if(FAILED(hr)){

	//	Logger::Get()->Error(L"Failed to init Input object, failed to set cooperative level", hr);
	//	return false;
	//}

	//// get access to keyboard //
	//hr = pKeyboard->Acquire();
	//if(FAILED(hr)){

	//	Logger::Get()->Info(L"Failed to acquire keyboard, doesn't mater will be tried again", false);
	//	// don't return it can be acquired later //
	//}

	// initialize mouse
	//hr = pDirectInput->CreateDevice(GUID_SysMouse, &pMouse, NULL);
	//if(FAILED(hr)){

	//	Logger::Get()->Error(L"Failed to init Input object, failed to create mouse device", hr);
	//	return false;
	//}

	//// Set the data format for the mouse using the pre-defined mouse data format.
	//hr = pMouse->SetDataFormat(&c_dfDIMouse);
	//if(FAILED(hr)){

	//	Logger::Get()->Error(L"Failed to init Input object, failed to set mouse data format", hr);
	//	return false;
	//}

	//// Set the cooperative level of the mouse to share with other programs.
	//hr = pMouse->SetCooperativeLevel(Engine::GetEngine()->GetWindow()->GetHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	//if(FAILED(hr)){

	//	Logger::Get()->Error(L"Failed to init Input object, failed to set mouse cooperative level", hr);
	//	return false;
	//}

	//// Acquire the mouse.
	//hr = pMouse->Acquire();
	//if(FAILED(hr)){

	//	Logger::Get()->Info(L"Failed to acquire mouse, doesn't mater will be tried again", false);
	//	// don't return it can be acquired later //
	//}


	return true;
}
void Input::Release(){
	// unacquire controls and release them //
	//if(pMouse){
	//	pMouse->Unacquire();
	//	pMouse->Release();
	//	pMouse = NULL;
	//}

	//if(pKeyboard){
	//	pKeyboard->Unacquire();
	//	pKeyboard->Release();
	//	pKeyboard = NULL;
	//}

	//SAFE_RELEASE(pDirectInput);

}
// ------------------------------------ //

bool Input::Update(){
	// read keyboard state //
	if(!ReadKeyboard()){

		Logger::Get()->Error(L"Input: Update: ReadKeyboard failed");
		return false;
	}
	
	auto tmpptr = Engine::GetEngine()->GetDefinition()->GetWindow();

	Window::GetRelativeMouse(Window::GetRenderWindowHandle(tmpptr), MouseX, MouseY);

	// get mouse move amount //
	if(CaptureMouse){
		// get distance to center of window //
		int width = tmpptr->getWidth();
		int height = tmpptr->getHeight();

		width /= 2;
		height /= 2;

		MouseMoveX = MouseX-width;
		MouseMoveY = MouseY-height;

		// set to center //
		Window::SetMouseToCenter(Window::GetRenderWindowHandle(tmpptr), tmpptr);
	}

	// process data to values //
	//UpdateValues();

	return true;
}
// ------------------------------------ //
void Input::GetMouseRelativeLocation(int& xpos, int& ypos){
	xpos = MouseX;
	ypos = MouseY;
}
void Input::SetMouseCapture(bool toset){
	CaptureMouse = toset;
	if(!CaptureMouse){
		MouseMoveX = 0;
		MouseMoveY = 0;
	}
}
void Input::GetMouseMoveAmount(int& xmove, int& ymove){
	xmove = MouseMoveX;
	ymove = MouseMoveY;
}
bool Input::GetKeyState(int charindex){
	if(KeyStates[charindex])
		return true;
	return false;
}
// ------------------------------------ //
bool Input::ReadKeyboard(){
	//HRESULT hr = S_OK;

	// read keyboard device //
	//hr = pKeyboard->GetDeviceState(sizeof(KeyStates), (LPVOID)&KeyStates);
	//if(FAILED(hr)){
	//	// check for focus lost and try to reacquire //
	//	if((hr == DIERR_INPUTLOST) | (hr == DIERR_NOTACQUIRED)){
	//		pKeyboard->Acquire();
	//	} else {
	//		Logger::Get()->Error(L"Keyboard GetDeviceState failed",hr, false);
	//		return false;
	//	}
	//}
	for (int x = 0; x < 256; x++)
		KeyStates[x] = (char) (GetAsyncKeyState(x) >> 8);

	return true;
}

DLLEXPORT void Leviathan::Input::ResolutionUpdated(int width, int height){
	// change internal values //
	ScreenWidth = width;
	ScreenHeight = height;
}

//bool InputClass::IsEscapePressed()
//{
//	// Do a bitwise and on the keyboard state to check if the escape key is currently being pressed.
//	if(m_keyboardState[DIK_ESCAPE] & 0x80)
//	{
//		return true;
//	}
//
//	return false;
//}