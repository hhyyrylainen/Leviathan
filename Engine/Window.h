// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Common/Types.h"

#include "include/cef_browser.h"

#include <atomic>
#include <memory>

namespace Ogre {

class RenderWindow;
class Camera;
class CompositorWorkspace;
} // namespace Ogre


struct SDL_Window;
struct SDL_Keysym;
union SDL_Event;
class CefBrowserHost;

namespace Leviathan {

//! window class
//! \todo Implement global lock for input handling
class Window {
public:
    //! \exception InvalidArgument if creation fails
    DLLEXPORT Window(Graphics* windowcreater, AppDef* windowproperties);
    DLLEXPORT ~Window();


    //! Called by Engine
    DLLEXPORT void Tick(int mspassed);

    //! This function uses the LinkObjects function objects
    DLLEXPORT bool Render(int mspassed, int tick, int timeintick);

    //! This function also updates the camera aspect ratio
    DLLEXPORT void LinkObjects(std::shared_ptr<GameWorld> world);

    DLLEXPORT void UnlinkAll();

    //! \returns true if succeeds, false if another window has input
    DLLEXPORT bool SetMouseCapture(bool state);


    // Input function //

    // graphics related //
    // DLLEXPORT float GetViewportAspectRatio();
    DLLEXPORT void SaveScreenShot(const std::string& filename);

    DLLEXPORT void OnResize(int width, int height);

    DLLEXPORT void OnFocusChange(bool focused);


    //! \brief Overwrites the default InputController with a
    //! custom one
    DLLEXPORT void SetCustomInputController(std::shared_ptr<InputController> controller);

    // ------------------------------------ //
    // Size properties and mouse handling
    DLLEXPORT inline float GetAspectRatio() const
    {
        int32_t width, height;
        GetSize(width, height);

        return (static_cast<float>(width)) / height;
    }

    DLLEXPORT void GetSize(int32_t& width, int32_t& height) const;

    DLLEXPORT void GetPosition(int32_t& x, int32_t& y) const;


    DLLEXPORT void SetHideCursor(bool toset);
#ifdef __linux
    DLLEXPORT void SetX11Cursor(int cursor);
#endif //__linux

    //! \note This is clamped to range [0, width / height]
    DLLEXPORT void GetRelativeMouse(int& x, int& y);
    //! \returns The normalized mouse x and y positions (in range [0, 1])
    DLLEXPORT void GetNormalizedRelativeMouse(float& x, float& y);
    DLLEXPORT void SetMouseToCenter();
    DLLEXPORT bool IsMouseOutsideWindowClientArea();

    //! \brief Translates a client space coordinate to screen coordinate
    //! \exception ExceptionNotFound If the window is not found (the internal translate fails)
    //! \note Doesn't work on linux, returns the input point
    // DLLEXPORT Int2 TranslateClientPointToScreenPoint(const Int2 &point) const;

    // ------------------------------------ //
    // Input handling

    //! \brief Called by the engine event loop after key events are sent
    //!
    //! This is used to reset modifier key states
    DLLEXPORT void InputEnd();

    //! \brief Passes initial mouse position to gui
    DLLEXPORT void ReadInitialMouse(CefBrowserHost* browserinput);

    DLLEXPORT uint32_t GetSDLID() const;

    DLLEXPORT uint32_t GetNativeHandle() const;

    //! \brief Returns whether this window is focused
    //! \return True when the window has focus
    DLLEXPORT bool IsWindowFocused() const
    {
        return Focused;
    }


    // Key press callbacks
    DLLEXPORT void InjectMouseMove(const SDL_Event& event);
    DLLEXPORT void InjectMouseWheel(const SDL_Event& event);
    DLLEXPORT void InjectMouseButtonDown(const SDL_Event& event);
    DLLEXPORT void InjectMouseButtonUp(const SDL_Event& event);
    //! Injects text
    DLLEXPORT void InjectCodePoint(const SDL_Event& event);
    DLLEXPORT void InjectKeyDown(const SDL_Event& event);
    DLLEXPORT void InjectKeyUp(const SDL_Event& event);

    // ------------------------------------ //
    // Various random get functions
    DLLEXPORT inline GUI::GuiManager* GetGui()
    {
        return WindowsGui.get();
    }
    DLLEXPORT inline InputController* GetInputController()
    {
        return TertiaryReceiver.get();
    }
    DLLEXPORT inline bool GetVsync() const;
    DLLEXPORT inline Ogre::RenderWindow* GetOgreWindow() const
    {
        return OWindow;
    };

    DLLEXPORT inline Ogre::SceneManager* GetOverlayScene()
    {
        return OverlayScene;
    }

    //! Returns this windows creation number
    //! \note This is quaranteed to be unique among all windows
    DLLEXPORT inline int GetWindowNumber() const
    {
        return WindowNumber;
    }

    //! Returns how many windows are open
    //! \see OpenWindowCount
    DLLEXPORT static int GetGlobalWindowCount()
    {
        return OpenWindowCount;
    }

    // ------------------------------------ //
    // Input helpers

    //! Called from GuiView when it has consumed the event that was passed to it
    DLLEXPORT inline void ReportKeyEventAsUsed()
    {
        InputProcessedByCEF = true;
    }

    DLLEXPORT static int GetCEFButtonFromSdlMouseButton(uint32_t whichbutton);

    DLLEXPORT static int32_t ConvertStringToKeyCode(const std::string& str);
    DLLEXPORT static std::string ConvertKeyCodeToString(const int32_t& code);

protected:
    //! \brief Detects state of modifier keys. Called whenever input is injected and is stored
    //! until InputEnd is called
    DLLEXPORT void _StartGatherInput();

    void _CheckMouseVisibilityStates();

    void DoCEFInputPass(const SDL_Keysym& arg, bool down);

    //! \brief Creates an Ogre scene to display GUI on this window
    void _CreateOverlayScene();

    void _DestroyOverlay();

private:
    //! Set null when the native window is no longer valid
    SDL_Window* SDLWindow = nullptr;

    //! This is temporarily stored during input gathering
    CefRefPtr<CefBrowserHost> inputreceiver = nullptr;

    //! Allows CEF to report whether a key input was handled
    //! Set by ReportKeyEventAsUsed
    bool InputProcessedByCEF;

    // this is updated every time input is gathered //
    int SpecialKeyModifiers = 0;
    //! Contains the modifier keys with the CEF flags
    int CEFSpecialKeyModifiers = 0;
    bool InputGatherStarted = false;

    bool Focused = true;

    bool ApplicationWantCursorState;
    bool ForceMouseVisible = false;
    bool CursorState = true;

    bool MouseCaptured = false;

    Ogre::RenderWindow* OWindow = nullptr;
    std::shared_ptr<InputController> TertiaryReceiver;
    std::unique_ptr<GUI::GuiManager> WindowsGui;

    Ogre::CompositorWorkspace* OverlayWorkspace = nullptr;
    Ogre::SceneManager* OverlayScene = nullptr;
    Ogre::Camera* OverlayCamera = nullptr;

    //! Unique id of this window.
    //! Makes sure that created Ogre resources are unique
    int ID;

    //! Used to do input setup each time some input is received
    // bool InputStarted = false;

    std::shared_ptr<GameWorld> LinkedWorld;

    //! Keeps track of how many windows in total have been created
    static std::atomic<int> TotalCreatedWindows;
    static std::atomic<int> OpenWindowCount;

    //! The number of this window (starts from 1)
    int WindowNumber;

    bool MouseCaptureState = false;
    //! \todo This should probably be atomic
    static Window* InputCapturer;
};
} // namespace Leviathan
