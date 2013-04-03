#ifndef LEVIATHAN_INPUT
#define LEVIATHAN_INPUT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace Leviathan{

	class Input : public EngineComponent{
	public:
		DLLEXPORT Input::Input();
        DLLEXPORT Input::~Input();

		DLLEXPORT bool Init(HINSTANCE thisprocess, int screenwidth, int screenheight);
		DLLEXPORT void Release();

		DLLEXPORT bool Update();

		DLLEXPORT void SetMouseCapture(bool toset);
		DLLEXPORT void GetMouseMoveAmount(int& xmove,int& ymove);


		DLLEXPORT void GetMouseRelativeLocation(int& xpos, int& ypos);
		DLLEXPORT bool GetKeyPressed(int charindex);

		DLLEXPORT bool IsMouseCaptured(){ return CaptureMouse; };
	private:
		bool ReadKeyboard();

		// ------------------ //

		//IDirectInput8* pDirectInput;
		//IDirectInputDevice8* pKeyboard;
		//IDirectInputDevice8* pMouse;

		//unsigned char KeyStates[256];
		char KeyStates[256];

		//DIMOUSESTATE MouseState;

		int ScreenWidth, ScreenHeight;

		bool CaptureMouse;
		int MouseMoveX, MouseMoveY;
		int MouseX, MouseY;


	};

}
#endif