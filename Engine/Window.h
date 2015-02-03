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


#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OISJoyStick.h>
#include <OISInputManager.h>
#include "CEGUI/GUIContext.h"
#include "CEGUI/InputEvent.h"

#ifdef __linux

// Predefine some Xlib stuff to make this header compile //
typedef long unsigned int XID;
typedef XID Cursor;
struct _XDisplay;
typedef _XDisplay Display;

#endif


namespace Leviathan{

	//! window class
	//! \todo Implement global lock for input handling
	class Window : public Ogre::WindowEventListener, OIS::KeyListener, OIS::MouseListener, OIS::JoyStickListener{
	public:
		DLLEXPORT Window(Ogre::RenderWindow* owindow, GraphicalInputEntity* owner);
		DLLEXPORT ~Window();

		DLLEXPORT void CloseDown();

		//! \brief Tells the Ogre window to close
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
		DLLEXPORT void GatherInput(CEGUI::InputAggregator* receiver);


		DLLEXPORT inline bool IsWindowed() const{ return !OWindow->isFullScreen();};
#ifdef _WIN32
		DLLEXPORT inline HWND GetHandle(){ VerifyRenderWindowHandle(); return m_hwnd; };
#else
		// X11 compatible handle //
		DLLEXPORT inline XID GetX11Window(){ VerifyRenderWindowHandle(); return m_hwnd; }
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


		//! \brief Returns whether this window is focused
		//! \return True when the window has focus
		DLLEXPORT bool IsWindowFocused() const;


		//! \brief Causes this window to no longer function
		//! \note This is provided to prevent calling some platform specific methods that could
		//! cause crashes
		DLLEXPORT void InvalidateWindow();

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
		DLLEXPORT inline Ogre::Camera* GetOverlayCamera() const{
			return OverLayCamera;
		}
		// map that converts OIS::KeyCode to CEGUI key codes, not required since the codes are the same! //
		//static std::map<OIS::KeyCode, CEGUI::Key::Scan> OISCEGUIKeyConvert;
		static boost::bimap<wstring, OIS::KeyCode> CharacterToOISConvert;

		// method for other DLLs to call the maps //
		DLLEXPORT static OIS::KeyCode ConvertWstringToOISKeyCode(const wstring &str);
		DLLEXPORT static wstring ConvertOISKeyCodeToWstring(const OIS::KeyCode &code);


	private:

		bool SetupOISForThisWindow();
		void ReleaseOIS();
		void UpdateOISMouseWindowSize();
#ifdef __GNUC__
		// X11 window focus find function //
		XID GetForegroundWindow();
#endif
		void CheckInputState();
		//! \brief Creates an Ogre scene to display GUI on this window
		//! \todo The window requires an ID member to make this unique
		void _CreateOverlayScene();
		void _CheckMouseVisibilityStates();
		// ------------------------------------ //
#ifdef _WIN32
		HWND m_hwnd;
#else
		XID m_hwnd;
		Display* XDisplay;

		Cursor XInvCursor;
#endif
		Ogre::RenderWindow* OWindow;
		Ogre::SceneManager* OverlayScene;
		Ogre::Camera* OverLayCamera;

        //! Like entity ID
        //! Makes sure that created Ogre resources are unique
        int ID;

		GraphicalInputEntity* OwningWindow;

		OIS::InputManager* WindowsInputManager;
		OIS::Mouse* WindowMouse;
		OIS::Keyboard* WindowKeyboard;
		std::vector<OIS::JoyStick*> WindowJoysticks;

		//! This is temporarily stored during input gathering
		CEGUI::InputAggregator* inputreceiver;

		bool ThisFrameHandledCreate;
		int LastFrameDownMouseButtons;
		
		// Set when the native window is no longer valid //
		bool IsInvalidated;

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
