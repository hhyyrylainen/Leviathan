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

namespace Leviathan{
	
	// for storing in pass to window //
	class LeviathanApplication;
	class Window;
	class GraphicalInputEntity;

	// window class //
	class Window : public Ogre::WindowEventListener, OIS::KeyListener, OIS::MouseListener, OIS::JoyStickListener{
	public:
		DLLEXPORT Window(Ogre::RenderWindow* owindow, GraphicalInputEntity* owner, bool vsync);
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

		DLLEXPORT void GatherInput(Rocket::Core::Context* context);

		DLLEXPORT inline bool IsWindowed() const{ return !OWindow->isFullScreen();};
		DLLEXPORT inline HWND GetHandle(){ return (m_hwnd = GetRenderWindowHandle(OWindow)); };
		DLLEXPORT inline int GetWidth() const{ return OWindow->getWidth(); };
		DLLEXPORT inline int GetHeight() const{ return OWindow->getHeight(); };
		DLLEXPORT inline bool GetVsync() const{ return VerticalSync;};
		DLLEXPORT inline Ogre::RenderWindow* GetOgreWindow() const{ return OWindow; };

		DLLEXPORT inline bool IsOpen() const{

			return !OWindow->isClosed();
		}

		// TODO: add a way to force only one window to have mouse captured //
		DLLEXPORT inline void SetCaptureMouse(bool state){
			MouseCaptured = state;
		}

		DLLEXPORT static HWND GetRenderWindowHandle(Ogre::RenderWindow* owindow);

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
		static std::map<OIS::KeyCode, Rocket::Core::Input::KeyIdentifier> OISRocketKeyConvert;
		static std::map<wstring, OIS::KeyCode> CharacterToOISConvert;

	private:

		bool SetupOISForThisWindow();
		void ReleaseOIS();
		void UpdateOISMouseWindowSize();

		void CheckInputState();
		void _CreateOverlayScene();
		// ------------------------------------ //

		HWND m_hwnd;
		Ogre::RenderWindow* OWindow;
		Ogre::SceneManager* OverlayScene;
		Ogre::Viewport* OverlayViewport;

		GraphicalInputEntity* OwningWindow;

		OIS::InputManager* WindowsInputManager;
		OIS::Mouse* WindowMouse;
		OIS::Keyboard* WindowKeyboard;
		std::vector<OIS::JoyStick*> WindowJoysticks;

		// this is temporarily stored during input gathering //
		Rocket::Core::Context* inputreceiver;
		bool ThisFrameHandledCreate;
		int LastFrameDownMouseButtons;
		// this is updated every time input is gathered //
		int SpecialKeyModifiers;

		bool VerticalSync;
		bool Focused;
		bool ApplicationWantCursorState;
		bool ForceMouseVisible;
		bool CursorState;

		bool MouseCaptured;
	};


}
#endif