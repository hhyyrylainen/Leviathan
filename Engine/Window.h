#ifndef LEVIATHAN_WINDOW
#define LEVIATHAN_WINDOW
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{
	//DLLEXPORT LRESULT CALLBACK DefWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			// window class //
	class Window : public EngineComponent{
	public:
		DLLEXPORT Window::Window();

		DLLEXPORT void Init(HWND hwnd, int width, int height);
		DLLEXPORT bool Init(HINSTANCE hInstance, WNDPROC proc, wstring tittle, int width, int height, HICON hIcon, bool windowed);
		DLLEXPORT bool Init(HINSTANCE hInstance);
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

		DLLEXPORT void GetRelativeMouse(int& x, int& y);
		DLLEXPORT void SetMouseToCenter();

		DLLEXPORT int IsWindowed(){ return Windowed;};

		// default stuff //
		//DLLEXPORT static LRESULT CALLBACK DefWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		bool Windowed;
		int Width;
		int Height;
		HWND m_hwnd;

		bool CursorHidden;
	};


}
#endif