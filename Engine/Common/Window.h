#ifndef LEVIATHAN_WINDOW
#define LEVIATHAN_WINDOW
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{
	
	// for storing in pass to window //
	class LeviathanApplication;
	class Window;

	// data to pass to wndproc //
	struct WindowPassData{
		WindowPassData(Window* wind, LeviathanApplication* appinterface);


		LeviathanApplication* Appinterface;
		Window* OwningWindow;
	};


	// window class //
	class Window : public EngineComponent{
	public:
		DLLEXPORT Window::Window();

		DLLEXPORT bool Init(HINSTANCE hInstance, WNDPROC proc, wstring tittle, int width, int height, HICON hIcon, bool windowed, 
			LeviathanApplication* application);

		DLLEXPORT void CloseDown();
		DLLEXPORT HWND GetHandle(){ return m_hwnd;};
		DLLEXPORT int GetWidth(){ return Width;};
		DLLEXPORT int GetHeight(){ return Height;};

		DLLEXPORT void SetNewSize(int width, int height);
		DLLEXPORT bool ResizeWin32Window(int newwidth, int newheight, bool resizetocenter = false);

		DLLEXPORT float GetAspectRatio() const;

		DLLEXPORT void SetHideCursor(bool toset);
		DLLEXPORT void LoseFocus();
		DLLEXPORT void GainFocus();

		DLLEXPORT static void GetRelativeMouse(HWND hwnd, int& x, int& y);
		

		DLLEXPORT bool IsWindowed(){ return Windowed;};

		// TODO: change this class to cache this //
		DLLEXPORT static HWND GetRenderWindowHandle(Ogre::RenderWindow* owindow);
		DLLEXPORT static void SetMouseToCenter(HWND hwnd, Ogre::RenderWindow* owindow);
	private:
		bool Windowed;
		int Width;
		int Height;
		HWND m_hwnd;

		bool CursorHidden;
	};


}
#endif