#ifndef LEVIATHAN_WINDOW
#define LEVIATHAN_WINDOW
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "OgreWindowEventUtilities.h"
#include "OgreRenderWindow.h"

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
	class Window : public Ogre::WindowEventListener{
	public:
		DLLEXPORT Window(Ogre::RenderWindow* owindow, bool vsync);
		DLLEXPORT ~Window();

		DLLEXPORT void CloseDown();


		DLLEXPORT void ResizeWindow(const int &width, const int &height);

		DLLEXPORT inline float GetAspectRatio() const{

			return ((float)GetWidth())/GetHeight();
		}

		DLLEXPORT void SetHideCursor(bool toset);

		// callback functions //
		virtual void windowResized(Ogre::RenderWindow* rw);
		virtual void windowFocusChange(Ogre::RenderWindow* rw);

		DLLEXPORT void GetRelativeMouse(int& x, int& y);
		DLLEXPORT void SetMouseToCenter();


		DLLEXPORT inline bool IsWindowed() const{ return !OWindow->isFullScreen();};
		DLLEXPORT inline HWND GetHandle(){ return (m_hwnd = GetRenderWindowHandle(OWindow)); };
		DLLEXPORT inline int GetWidth() const{ return OWindow->getWidth(); };
		DLLEXPORT inline int GetHeight() const{ return OWindow->getHeight(); };
		DLLEXPORT inline bool GetVsync() const{ return VerticalSync;};
		DLLEXPORT inline Ogre::RenderWindow* GetOgreWindow() const{ return OWindow; };

		DLLEXPORT inline bool IsOpen() const{

			return !OWindow->isClosed();
		}

		DLLEXPORT static HWND GetRenderWindowHandle(Ogre::RenderWindow* owindow);
	private:

		HWND m_hwnd;
		Ogre::RenderWindow* OWindow;

		bool VerticalSync;
		bool Focused;
		bool CursorHidden;
	};


}
#endif