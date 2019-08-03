// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Common/Types.h"

#ifdef _WIN32
#include "WindowsInclude.h"
#endif

#include "bsfCore/RenderAPI/BsRenderWindow.h"

#include <atomic>
#include <memory>

struct SDL_Window;
struct SDL_Keysym;
union SDL_Event;
struct SDL_Cursor;
struct SDL_Surface;

namespace Leviathan {

namespace GUI {
class Layer;
enum class INPUT_EVENT_TYPE : int;
} // namespace GUI

//! window class
//! \todo Implement global lock for input handling
//! \todo This should be handled through shared_ptr
class Window {
    struct BSFResources;

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

    //! \brief Tries to bring this window to front
    DLLEXPORT void BringToFront();

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
    DLLEXPORT void SetX11Cursor(int cursor, int retrycount = 10);
#endif //__linux
#ifdef _WIN32
    DLLEXPORT void SetWinCursor(HCURSOR cursor);
#endif //_WIN32

    //! \note This is clamped to range [0, width / height]
    DLLEXPORT void GetRelativeMouse(int& x, int& y) const;

    //! Unclamped version of GetRelativeMouse can return negative coordinates if the mouse is
    //! left of this window
    DLLEXPORT void GetUnclampedRelativeMouse(int& x, int& y) const;

    //! \returns The normalized mouse x and y positions (in range [0, 1])
    DLLEXPORT void GetNormalizedRelativeMouse(float& x, float& y) const;
    DLLEXPORT void SetMouseToCenter();
    DLLEXPORT bool IsMouseOutsideWindowClientArea() const;

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

    DLLEXPORT uint32_t GetSDLID() const;

#ifdef _WIN32
    DLLEXPORT HWND GetNativeHandle() const;
#else
    DLLEXPORT unsigned long GetNativeHandle() const;
#endif //_WIN32

#if defined(__linux)
    DLLEXPORT unsigned long GetWindowXDisplay() const;
#endif

    //! \brief Returns whether this window is focused
    //! \return True when the window has focus
    DLLEXPORT bool IsWindowFocused() const
    {
        return Focused;
    }


    // Key press callbacks
    DLLEXPORT void InjectMouseMove(const SDL_Event& event);
    //! \todo allow configuring if mouse wheel is considered a key (for Gameplay input mode)
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

    DLLEXPORT inline const auto& GetBSFWindow() const
    {
        return BSFWindow;
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

    //! \todo Move to KeyMapping.cpp


protected:
    //! \brief Detects state of modifier keys. Called whenever input is injected and is stored
    //! until InputEnd is called
    //! \todo Fix mouse capture
    DLLEXPORT void _StartGatherInput();

    void _CheckMouseVisibilityStates();

    //! \returns True if the event was passed to the GUI
    //!
    //! It is difficult to get the events back (from CEF) in a guaranteed time so
    //! we use GUI_INPUT_MODE to only send events to the GUI when they wouldn't be used
    //! by any player character controllers or similar
    bool DoGUILayerInputPass(
        const SDL_Event& sdlevent, bool down, bool textinput, int mousex, int mousey);

    //! \brief Retrieves the active gui object that is going to receive an event
    //! \param type If Keypress then only the currently input receiving window
    //! (controlled by GUI_INPUT_MODE) is returned if there is one.
    //! For mouse events the gui object under the cursor position is returned (unless mouse
    //! capture is on then nothing is returned even if iskeypress is true)
    //! If Scroll then this is a mouse scroll event and a GUI::Layer that has a
    //! scrollable element is returned in every case except when the mouse is captured (or the
    //! Layer has None as the input mode)
    GUI::Layer* GetGUIEventReceiver(GUI::INPUT_EVENT_TYPE type, int mousex, int mousey);

#ifdef __linux
    //! \brief Called in SetX11Cursor (or after a slight delay)
    //!
    //! This grabs the current X11 cursor and makes an SDL_Cursor out of it to make it
    //! permanent
    DLLEXPORT void MakeX11CursorPermanent();
#endif //__linux

private:
    //! Set null when the native window is no longer valid
    SDL_Window* SDLWindow = nullptr;

    bs::SPtr<bs::RenderWindow> BSFWindow;

    //! This is retrieved from GuiManager at the start of a sequence of inputs based
    //! on the mouse position. The property of GUI_INPUT_MODE will determine how the input
    //! is passed
    // boost::intrusive_ptr<GUI::View> inputreceiver = nullptr;

    ////! Allows CEF to report whether a key input was handled
    ////! Set by ReportKeyEventAsUsed
    // bool InputProcessedByCEF;

    // this is updated every time input is gathered //
    int SpecialKeyModifiers = 0;
    //! Contains the modifier keys with the CEF flags
    int CEFSpecialKeyModifiers = 0;
    //! Used to populate SpecialKeyModifiers and CEFSpecialKeyModifiers once per input
    //! gathering
    bool InputGatherStarted = false;

    //! This is used to send the initial mouse position on the first frame to make sure that
    //! mouse visibility and custom cursors are set
    bool InitialMousePositionSent = false;

    bool Focused = true;

    //! The state the GUI::GuiManager on this window wants CursorState to be
    bool ApplicationWantCursorState;
    //! This is true when the mouse needs to be visible, for example when it is outside our
    //! client area)
    bool ForceMouseVisible = false;
    //! Controls whether hardware cursor should be visible
    bool CursorState = true;

    bool MouseCaptured = false;

    //! \todo Cache old cursors to avoid recreations
    std::unique_ptr<SDL_Cursor, void (*)(SDL_Cursor*)> CurrentCursor;
    std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> CurrentCursorImage;

#ifdef __linux
    //! This is the cursor that SetX11Cursor was last called with
    int WantedX11Cursor = 0;
#endif //__linux

    std::shared_ptr<InputController> TertiaryReceiver;
    std::unique_ptr<GUI::GuiManager> WindowsGui;

    //! Unique id of this window.
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

    //! Per-window BSF resources
    std::unique_ptr<BSFResources> _BSFResources;

    //! Workaround for missing BSF resize notify
    //! https://github.com/GameFoundry/bsf/issues/382
    bool DoingResize = false;

    //! \todo This should probably be atomic
    static Window* InputCapturer;
};
} // namespace Leviathan
