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
#include "OIS.h"
#include "boost/bimap.hpp"

#ifdef __GNUC__
// Preprocessor magic to not conflict with Window class //
namespace X11{
#include <X11/Xlib.h>

// X11 additional includes
#include <X11/Xutil.h>
#include "X11/Xlibint.h"
#include <X11/Xos.h>
#include <X11/Xatom.h>
// Don't want to define our window with the same name //
}
// Need some magic to not confuse with GenericEvent macro with GenericEvent class //
#define X11GenericEvent GenericEvent
#undef GenericEvent
#define X11Status Status
#undef Status
#define X11None None
#undef None
#define X11index index
#undef index
#define X11CurrentTime CurrentTime
#undef CurrentTime

#endif

#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OISJoyStick.h>
#include <OISInputManager.h>
#include "include/cef_browser.h"

namespace Leviathan{

	// for storing in pass to window //
	class LeviathanApplication;
	class Window;
	class GraphicalInputEntity;

	//! window class
	//! \todo Implement global lock for input handling
	class Window : public Ogre::WindowEventListener, OIS::KeyListener, OIS::MouseListener, OIS::JoyStickListener{
	public:
		DLLEXPORT Window(Ogre::RenderWindow* owindow, GraphicalInputEntity* owner);
		DLLEXPORT ~Window();

		DLLEXPORT void CloseDown();
		// tells the Ogre window to close //
		DLLEXPORT void SendCloseMessage();

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
		DLLEXPORT bool IsMouseOutsideWindowClientArea();

		//! \brief Gets the window's rectangle in screen coordinates
		//! \return The screen begin x and y, and z and w as the width and height all in screen pixels
		//! \exception ExceptionNotFound If the window is not found (the internal get rect fails)
		//! \note This doesn't "work" on linux, same as calling GetWidth and GetHeight
		DLLEXPORT Int4 GetScreenPixelRect() const THROWS;

		//! \brief Translates a client space coordinate to screen coordinate
		//! \exception ExceptionNotFound If the window is not found (the internal translate fails)
		//! \note Doesn't work on linux, returns the input point
		DLLEXPORT Int2 TranslateClientPointToScreenPoint(const Int2 &point) const THROWS;
				
		//! \brief Captures input for this window and passes it on
		//! \bug Only the first browser is used, when the browser should be determined by getting the active one from the GuiManager
		DLLEXPORT void GatherInput(CefRefPtr<CefBrowserHost> browserinput);


		DLLEXPORT inline bool IsWindowed() const{ return !OWindow->isFullScreen();};
#ifdef _WIN32
		DLLEXPORT inline HWND GetHandle(){ VerifyRenderWindowHandle(); return m_hwnd; };
#else
		// X11 compatible handle //
		DLLEXPORT inline X11::XID GetX11Window(){ VerifyRenderWindowHandle(); return m_hwnd; }
#endif
		DLLEXPORT inline int GetWidth() const{ return OWindow->getWidth(); };
		DLLEXPORT inline int GetHeight() const{ return OWindow->getHeight(); };
		DLLEXPORT inline bool GetVsync() const{ return OWindow->isVSyncEnabled();};
		DLLEXPORT inline Ogre::RenderWindow* GetOgreWindow() const{ return OWindow; };

		DLLEXPORT inline bool IsOpen() const{

			return !OWindow->isClosed();
		}

		// \todo add a way to force only one window to have mouse captured //
		DLLEXPORT inline void SetCaptureMouse(bool state){
			MouseCaptured = state;
		}

		DLLEXPORT bool VerifyRenderWindowHandle();


		virtual bool keyPressed(const OIS::KeyEvent &arg);
		virtual bool keyReleased(const OIS::KeyEvent &arg);
		virtual bool mouseMoved(const OIS::MouseEvent &arg);
		virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
		virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
		virtual bool buttonPressed(const OIS::JoyStickEvent &arg, int button);
		virtual bool buttonReleased(const OIS::JoyStickEvent &arg, int button);
		virtual bool axisMoved(const OIS::JoyStickEvent &arg, int axis);

		DLLEXPORT string GetOISCharacterAsText(const OIS::KeyCode &code);

		DLLEXPORT inline Ogre::SceneManager* GetOverlayScene(){
			return OverlayScene;
		}
		DLLEXPORT inline Ogre::Viewport* GetOverlayViewport(){
			return OverlayViewport;
		}
		// map that converts OIS::KeyCode to Rocket key codes //
		static std::map<OIS::KeyCode, int> OISVKeyConvert;
		static boost::bimap<wstring, OIS::KeyCode> CharacterToOISConvert;

		// method for other DLLs to call the maps //
		DLLEXPORT static OIS::KeyCode ConvertWstringToOISKeyCode(const wstring &str);
		DLLEXPORT static wstring ConvertOISKeyCodeToWstring(const OIS::KeyCode &code);

		void ReportKeyEventAsUsed();

	private:

		bool SetupOISForThisWindow();
		void ReleaseOIS();
		void UpdateOISMouseWindowSize();
#ifdef __GNUC__
		// X11 window focus find function //
		X11::XID GetForegroundWindow();
#endif
		void CheckInputState();
		void _CreateOverlayScene();
		void _CheckMouseVisibilityStates();
		// ------------------------------------ //
#ifdef _WIN32
		HWND m_hwnd;
#else
		X11::XID m_hwnd;
		X11::Display* XDisplay;
#endif
		Ogre::RenderWindow* OWindow;
		Ogre::SceneManager* OverlayScene;
		Ogre::Viewport* OverlayViewport;

		GraphicalInputEntity* OwningWindow;

		OIS::InputManager* WindowsInputManager;
		OIS::Mouse* WindowMouse;
		OIS::Keyboard* WindowKeyboard;
		std::vector<OIS::JoyStick*> WindowJoysticks;

		// this is temporarily stored during input gathering //
		CefRefPtr<CefBrowserHost> inputreceiver;
		bool ThisFrameHandledCreate;
		int LastFrameDownMouseButtons;

		//! Allows CEF to report whether a key input was handled
		//! Set by ReportKeyEventAsUsed
		bool InputProcessedByCEF;

		// this is updated every time input is gathered //
		int SpecialKeyModifiers;
		bool Focused;
		bool ApplicationWantCursorState;
		bool ForceMouseVisible;
		bool CursorState;

		bool FirstInput;

		bool MouseCaptured;
	};


}
#endif
