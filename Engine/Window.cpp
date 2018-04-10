// ------------------------------------ //
#include "Window.h"

#include "Engine.h"
#include "Entities/GameWorld.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "GUI/GuiManager.h"
#include "GUI/KeyMapping.h"
#include "Handlers/IDFactory.h"
#include "Input/InputController.h"
#include "Input/Key.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Rendering/Graphics.h"
#include "TimeIncludes.h"
#include "Utility/Convert.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "OgreCommon.h"
#include "OgreRenderWindow.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreVector4.h"
#include "OgreWindowEventUtilities.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include "include/cef_browser.h"
#include "include/cef_keyboard_handler.h"
#include "include/internal/cef_types.h"
#include "include/internal/cef_types_wrappers.h"

#include <algorithm>
#include <thread>
// ------------------------------------ //

namespace Leviathan {

Window* Window::InputCapturer = nullptr;

std::atomic<int> Window::OpenWindowCount = 0;
std::atomic<int> Window::TotalCreatedWindows = 0;

// ------------------------------------ //
DLLEXPORT Window::Window(Graphics* windowcreater, AppDef* windowproperties) :
    ID(IDFactory::GetID())
{
    // Create window //

    const WindowDataDetails& WData = windowproperties->GetWindowDetails();

    // set some rendering specific parameters //
    Ogre::NameValuePairList WParams;

    // variables //
    Ogre::String fsaastr = Convert::ToString(WData.FSAA);

    WParams["FSAA"] = fsaastr;
    WParams["vsync"] = WData.VSync ? "true" : "false";
    WParams["gamma"] = WData.UseGamma ? "true" : "false";

    Ogre::String wcaption = WData.Title;

    int extraFlags = 0;

    if(WData.FullScreen == "no" || WData.FullScreen == "0") {
    } else if(WData.FullScreen == "fullscreendesktop") {
        extraFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else if(WData.FullScreen == "fullscreenvideomode") {
        extraFlags |= SDL_WINDOW_FULLSCREEN;
    } else {

        LOG_ERROR("Window: invalid fullscreen value: " + WData.FullScreen);
    }

    // TODO: On Apple's OS X you must set the NSHighResolutionCapable
    // Info.plist property to YES, otherwise you will not receive a
    // High DPI OpenGL
    // canvas. https://wiki.libsdl.org/SDL_CreateWindow

    SDL_Window* sdlWindow = SDL_CreateWindow(WData.Title.c_str(),
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(WData.DisplayNumber),
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(WData.DisplayNumber), WData.Width, WData.Height,
        // This seems to cause issues on Windows
        // SDL_WINDOW_OPENGL |
        SDL_WINDOW_RESIZABLE | extraFlags);

    // SDL_WINDOW_FULLSCREEN_DESKTOP works so much better than
    // SDL_WINDOW_FULLSCREEN so it should be always used

    // SDL_WINDOW_BORDERLESS
    // SDL_WINDOWPOS_UNDEFINED_DISPLAY(x)
    // SDL_WINDOWPOS_CENTERED_DISPLAY(x)

    if(!sdlWindow) {

        LOG_FATAL("SDL Window creation failed, error: " + std::string(SDL_GetError()));
    }

    // SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if(!SDL_GetWindowWMInfo(sdlWindow, &wmInfo)) {

        LOG_FATAL("Window: created sdl window failed to retrieve info");
    }

#ifdef _WIN32
    auto winHandle = reinterpret_cast<size_t>(wmInfo.info.win.window);
    // WParams["parentWindowHandle"] = Ogre::StringConverter::toString(winHandle);
    // This seems to be the right name on windows
    WParams["externalWindowHandle"] = Ogre::StringConverter::toString(winHandle);
    // externalWindowHandle
#else
    WParams["parentWindowHandle"] =
        Ogre::StringConverter::toString((unsigned long)wmInfo.info.x11.display) + ":" +
        Ogre::StringConverter::toString(
            (unsigned int)XDefaultScreen(wmInfo.info.x11.display)) +
        ":" + Ogre::StringConverter::toString((unsigned long)wmInfo.info.x11.window);

#endif

    Ogre::RenderWindow* tmpwindow;

    try {
        tmpwindow = windowcreater->GetOgreRoot()->createRenderWindow(
            wcaption, WData.Width, WData.Height, false, &WParams);
    } catch(const Ogre::RenderingAPIException& e) {

        LOG_ERROR("Failed to create Ogre window, exception: " + std::string(e.what()));
        throw;
    }

    int windowsafter = ++OpenWindowCount;

    // Do some first window initialization //
    if(windowsafter == 1) {

        // Notify engine to register threads to work with Ogre //
        Engine::GetEngine()->_NotifyThreadsRegisterOgre();

        // Hlms is needed to parse scripts etc.
        windowcreater->_LoadOgreHLMS();

        FileSystem::RegisterOGREResourceGroups();
    }

    // Store this window's number
    WindowNumber = ++TotalCreatedWindows;

    OWindow = tmpwindow;

#ifdef _WIN32
    // Fetch the windows handle from SDL //
    HWND ourHWND = wmInfo.info.win.window;

    // apply style settings (mainly ICON) //
    if(ourHWND) {

        WData.ApplyIconToHandle(ourHWND);

    } else {
        LOG_WARNING("Window: failed to get window HWND for styling");
    }
#else
    // \todo linux icon
#endif
    tmpwindow->setDeactivateOnFocusChange(false);

    // set the new window to be active //
    tmpwindow->setActive(true);
    Focused = true;

    // Overlay is needed for the GUI to register its views
    _CreateOverlayScene();

    // create GUI //
    WindowsGui = std::make_unique<GUI::GuiManager>();
    if(!WindowsGui) {
        // TODO: the window must be destroyed here
        DEBUG_BREAK;
        throw NULLPtr("cannot create GUI manager instance");
    }

    if(!WindowsGui->Init(windowcreater, this)) {

        LOG_ERROR("Window: Gui init failed");
        throw NULLPtr("invalid GUI manager");
    }

    // create receiver interface //
    TertiaryReceiver = std::shared_ptr<InputController>(new InputController());

    SDLWindow = sdlWindow;

    // TODO: this needs to be only used when a text box etc. is used
    // SDL_StartTextInput();

    // cursor on top of window isn't hidden //
    ApplicationWantCursorState = false;
}

DLLEXPORT Window::~Window()
{
    // GUI is very picky about delete order
    if(WindowsGui) {
        WindowsGui->Release();
        WindowsGui.reset();
    }

    if(MouseCaptured) {

        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    // Un fullscreen to make sure nothing is screwed up
    if(SDL_GetWindowFlags(SDLWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) {

        LOG_INFO("Window: unfullscreened before quit");
        SDL_SetWindowFullscreen(SDLWindow, 0);
    }

    // Report that the window is now closed //
    Logger::Get()->Info(
        "Window: closing window(" + Convert::ToString(GetWindowNumber()) + ")");

    _DestroyOverlay();

    // Close the window //
    OWindow->destroy();

    TertiaryReceiver.reset();

    int windowsafter = --OpenWindowCount;

    if(windowsafter == 0) {

        Logger::Get()->Info("Window: all windows have been closed, "
                            "should quit soon");
    }

    LOG_WRITE("TODO: check why calling SDL_DestroyWindow crashes in Ogre "
              "GLX plugin uninstall");
    // SDL_DestroyWindow(SDLWindow);
    SDL_HideWindow(SDLWindow);
    SDLWindow = nullptr;
}
// ------------------------------------ //
DLLEXPORT void Window::LinkObjects(std::shared_ptr<GameWorld> world)
{
    if(LinkedWorld) {

        LinkedWorld->OnUnLinkedFromWindow(this, Graphics::Get()->GetOgreRoot());
    }

    LinkedWorld = world;

    if(LinkedWorld) {

        LinkedWorld->OnLinkToWindow(this, Graphics::Get()->GetOgreRoot());
    }
}

DLLEXPORT void Window::UnlinkAll()
{
    LinkObjects(nullptr);
}
// ------------------------------------ //
DLLEXPORT void Window::Tick(int mspassed)
{
    // pass to GUI //
    WindowsGui->GuiTick(mspassed);
}

DLLEXPORT bool Window::Render(int mspassed, int tick, int timeintick)
{
    if(LinkedWorld)
        LinkedWorld->Render(mspassed, tick, timeintick);

    // Update GUI before each frame //
    WindowsGui->Render();

    // update window //
    Ogre::RenderWindow* tmpwindow = GetOgreWindow();

    // finish rendering the window //

    // We don't actually want to swap buffers here because the Engine
    // method that called us will call Ogre::Root::renderOneFrame
    // after Render has been called on all active windows, so if we
    // have this call here we may do double swap depending on the
    // drivers.
    // tmpwindow->swapBuffers();

    return true;
}

DLLEXPORT void Window::OnResize(int width, int height)
{
// Notify Ogre //
// This causes issues on Windows
#ifdef __linux__
    GetOgreWindow()->resize(width, height);
#endif
    GetOgreWindow()->windowMovedOrResized();

    // send to GUI //
    WindowsGui->OnResize();
}

DLLEXPORT void Window::OnFocusChange(bool focused)
{
    if(Focused == focused)
        return;

    LOG_INFO("Focus change in Window");

    // Update mouse //
    Focused = focused;
    _CheckMouseVisibilityStates();

    WindowsGui->OnFocusChanged(focused);

    if(!Focused && MouseCaptured) {

        LOG_WRITE("TODO: We need to force GUI on to stop mouse capture");
        LOG_FATAL("Not implemented unfocus when mouse capture is on");
    }
}
// ------------------------------------ //
#ifdef __linux
DLLEXPORT void Window::SetX11Cursor(int cursor)
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if(!SDL_GetWindowWMInfo(SDLWindow, &wmInfo)) {

        LOG_FATAL("Window: SetX11Cursor: failed to retrieve wm info");
        return;
    }

    // Retrieve the X11 display shared with Chromium.
    ::Display* xdisplay = cef_get_xdisplay();
    LEVIATHAN_ASSERT(xdisplay, "cef_get_xdisplay failed");

    // This is broken because apparently CEF has its own cursor stuff
    // XDefineCursor(wmInfo.info.x11.display, wmInfo.info.x11.window, cursor);
    XDefineCursor(xdisplay, wmInfo.info.x11.window, cursor);
}
#endif //__linux
// ------------------------------------ //
DLLEXPORT bool Window::SetMouseCapture(bool state)
{
    if(MouseCaptureState == state)
        return true;

    MouseCaptureState = state;

    // handle changing state //
    if(!MouseCaptureState) {

        // set mouse visible and disable capturing //
        SDL_SetRelativeMouseMode(SDL_FALSE);
        // reset pointer to indicate that this object no longer captures mouse to this window
        // //
        InputCapturer = nullptr;

    } else {

        if(InputCapturer != this && InputCapturer != nullptr) {
            // another window has input //
            MouseCaptureState = false;
            return false;
        }

        SDL_SetRelativeMouseMode(SDL_TRUE);

        // hide mouse and tell window to capture //
        // DisplayWindow->SetMouseToCenter();

        // set static ptr to this //
        InputCapturer = this;
    }
    return true;
}


DLLEXPORT void Window::SetHideCursor(bool toset)
{
    ApplicationWantCursorState = toset;

    if(!ApplicationWantCursorState || ForceMouseVisible) {
        // show cursor //
        if(!CursorState) {
            CursorState = true;
            Logger::Get()->Info("Showing cursor");

            // Don't do anything if window is not valid anymore //
            if(!SDLWindow)
                return;

            SDL_ShowCursor(SDL_ENABLE);
        }
    } else {
        // hide cursor //
        if(CursorState) {

            CursorState = false;
            Logger::Get()->Info("Hiding cursor");

            // Don't do anything if window is not valid anymore //
            if(!SDLWindow)
                return;

            SDL_ShowCursor(SDL_DISABLE);
        }
    }
}

DLLEXPORT void Window::SetMouseToCenter()
{

    int32_t width, height;
    GetSize(width, height);
    SDL_WarpMouseInWindow(SDLWindow, width / 2, height / 2);
}

DLLEXPORT void Window::GetRelativeMouse(int& x, int& y)
{
    if(!SDLWindow)
        return;

    int globalX, globalY;
    SDL_GetGlobalMouseState(&globalX, &globalY);

    int windowX, windowY;
    SDL_GetWindowPosition(SDLWindow, &windowX, &windowY);


    globalX -= windowX;
    globalY -= windowY;

    int32_t width, height;
    GetSize(width, height);

    x = std::clamp(globalX, 0, width);
    y = std::clamp(globalY, 0, height);
}

DLLEXPORT void Window::GetNormalizedRelativeMouse(float& x, float& y)
{

    int xInt, yInt;
    GetRelativeMouse(xInt, yInt);

    int32_t width, height;
    GetSize(width, height);

    if(width == 0 || height == 0) {
        x = 0.5f;
        y = 0.5f;
        return;
    }

    x = static_cast<float>(xInt) / width;
    y = static_cast<float>(yInt) / height;
}

DLLEXPORT bool Window::IsMouseOutsideWindowClientArea()
{
    int X, Y;
    GetRelativeMouse(X, Y);

    int32_t width, height;
    GetSize(width, height);

    // check the coordinates //

    if(X < 0 || Y < 0 || X > width || Y > height) {
        return true;
    }

    return false;
}
// ------------------------------------ //
DLLEXPORT void Window::GetSize(int32_t& width, int32_t& height) const
{
    SDL_GetWindowSize(SDLWindow, &width, &height);
}

DLLEXPORT void Window::GetPosition(int32_t& x, int32_t& y) const
{
    SDL_GetWindowPosition(SDLWindow, &x, &y);
}
// ------------------------------------ //
DLLEXPORT uint32_t Window::GetSDLID() const
{
    if(!SDLWindow)
        return -1;

    return SDL_GetWindowID(SDLWindow);
}

#ifdef _WIN32
DLLEXPORT HWND Window::GetNativeHandle() const
#else
DLLEXPORT uint32_t Window::GetNativeHandle() const
#endif //_WIN32
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if(!SDL_GetWindowWMInfo(SDLWindow, &wmInfo)) {

        LOG_FATAL("Window: GetNativeHandle: failed to retrieve wm info");
#ifdef _WIN32
		return 0;
#else
        return -1;
#endif //_WIN32
    }
#if defined(_WIN32)
    return wmInfo.info.win.window;
#else
#if defined(__linux)
    return wmInfo.info.x11.window;
#else
#error Fix this for mac
#endif
#endif
}
// ------------------------------------ //
DLLEXPORT void Window::SaveScreenShot(const std::string& filename)
{
    // uses render target's capability to save it's contents //
    GetOgreWindow()->writeContentsToTimestampedFile(filename, "_window1.png");
}

DLLEXPORT bool Window::GetVsync() const
{
    return OWindow->isVSyncEnabled();
}

DLLEXPORT void Window::SetCustomInputController(std::shared_ptr<InputController> controller)
{
    TertiaryReceiver = controller;
}
// ------------------------------------ //
void Window::_CreateOverlayScene()
{
    Ogre::Root& ogre = Ogre::Root::getSingleton();

    // create scene manager //
    OverlayScene = ogre.createSceneManager(Ogre::ST_GENERIC, 1,
        Ogre::INSTANCING_CULLING_SINGLETHREAD, "Overlay_window_" + Convert::ToString(ID));

    // create camera //
    OverlayCamera = OverlayScene->createCamera("overlay camera");

    // Create the workspace for this scene
    // Which will render after the normal scene
    OverlayWorkspace = ogre.getCompositorManager2()->addWorkspace(
        OverlayScene, OWindow, OverlayCamera, "OverlayWorkspace", true, 1000);
}

void Window::_DestroyOverlay()
{
    // Destroy the compositor //
    Ogre::Root& ogre = Ogre::Root::getSingleton();

    // Allow releasing twice
    if(OverlayWorkspace) {
        ogre.getCompositorManager2()->removeWorkspace(OverlayWorkspace);
        OverlayWorkspace = nullptr;
    }

    if(OverlayScene) {
        ogre.destroySceneManager(OverlayScene);
        OverlayScene = nullptr;
        OverlayCamera = nullptr;
    }
}

// ------------------------------------ //
DLLEXPORT void Window::_StartGatherInput()
{
    // Quit if window closed //
    if(!SDLWindow) {

        LOG_WARNING("Window: GatherInput: window is closed");
        return;
    }

    InputGatherStarted = true;

    CefBrowserHost* browserinput;

    inputreceiver = WindowsGui->GetPrimaryInputReceiver();

    if(!inputreceiver)
        LOG_ERROR("Window: _StartGatherInput: gui manager had no browser host");

    GetInputController()->StartInputGather();

    SpecialKeyModifiers = 0;
    CEFSpecialKeyModifiers = EVENTFLAG_NONE;

    const auto mods = SDL_GetModState();

    if(mods & KMOD_CTRL) {
        SpecialKeyModifiers |= KEYSPECIAL_CTRL;
        CEFSpecialKeyModifiers |= EVENTFLAG_CONTROL_DOWN;
    }
    if(mods & KMOD_ALT) {
        SpecialKeyModifiers |= KEYSPECIAL_ALT;
        CEFSpecialKeyModifiers |= EVENTFLAG_ALT_DOWN;
    }
    if(mods & KMOD_SHIFT) {
        SpecialKeyModifiers |= KEYSPECIAL_SHIFT;
        CEFSpecialKeyModifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if(mods & KMOD_CAPS) {
        SpecialKeyModifiers |= KEYSPECIAL_CAPS;
        CEFSpecialKeyModifiers |= EVENTFLAG_CAPS_LOCK_ON;
    }
    if(mods & KMOD_GUI) {
        SpecialKeyModifiers |= KEYSPECIAL_SUPER;
    }

    // TODO: EVENTFLAG_NUM_LOCK_ON;

    // TODO: fix mouse capture
    // // Handle mouse capture
    // if(MouseCaptured && Focused) {

    //     // get mouse relative to window center //
    //     int xmoved = 0, ymoved = 0;

    //     SDL_GetRelativeMouseState(&xmoved, &ymoved);

    //     // Pass input //
    //     GetInputController()->SendMouseMovement(xmoved, ymoved);
    // }
}

DLLEXPORT void Window::InputEnd()
{
    InputGatherStarted = false;
    inputreceiver = nullptr;
}

void Window::_CheckMouseVisibilityStates()
{

    // force cursor visible check (if outside client area or mouse is unfocused on the window)
    if(IsMouseOutsideWindowClientArea() || !Focused) {

        ForceMouseVisible = true;

    } else {

        ForceMouseVisible = false;
    }

    // update cursor state //
    SetHideCursor(ApplicationWantCursorState);
}
// ------------------------------------ //
DLLEXPORT int Window::GetCEFButtonFromSdlMouseButton(uint32_t whichbutton)
{
    if(whichbutton == SDL_BUTTON_LEFT) {
        return MBT_LEFT;

    } else if(whichbutton == SDL_BUTTON_RIGHT) {
        return MBT_RIGHT;

    } else if(whichbutton == SDL_BUTTON_MIDDLE) {
        return MBT_MIDDLE;

    } else {
        return -1;
    }
}
// ------------------ Input listener functions ------------------ //
void Window::DoCEFInputPass(const SDL_Event& sdlevent, bool down)
{
    if(!inputreceiver)
        return;

    InputProcessedByCEF = false;

    // Currently there's no code from CEF here
    // // Heavily modified CEF (chromium embedded framework) code, license:

    // GDK compatible key code
    int asX11KeyCode = KeyMapping::SDLKeyToX11Key(sdlevent.key.keysym);

    // Modified to work here
    // Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
    // reserved. Use of this source code is governed by a BSD-style license that
    // can be found in the LICENSE file.
    // Based on WebKeyboardEventBuilder::Build from
    // content/browser/renderer_host/input/web_input_event_builders_gtk.cc.
    CefKeyEvent key_event;
    KeyboardCode windows_key_code = KeyMapping::GdkEventToWindowsKeyCode(asX11KeyCode);
    key_event.windows_key_code =
        KeyMapping::GetWindowsKeyCodeWithoutLocation(windows_key_code);
    key_event.native_key_code = asX11KeyCode;

    key_event.modifiers = CEFSpecialKeyModifiers;
    // if(event->keyval >= GDK_KP_Space && event->keyval <= GDK_KP_9)
    //     key_event.modifiers |= EVENTFLAG_IS_KEY_PAD;
    if(key_event.modifiers & EVENTFLAG_ALT_DOWN)
        key_event.is_system_key = true;

    // This is VKEY_RETURN = 0x0D
    if(windows_key_code == 0x0D) {
        // We need to treat the enter key as a key press of character \r.  This
        // is apparently just how webkit handles it and what it expects.
        key_event.unmodified_character = '\r';
    } else {
        // FIXME: fix for non BMP chars
        key_event.unmodified_character = sdlevent.key.keysym.sym;
        // static_cast<int>(gdk_keyval_to_unicode(event->keyval));
    }

    // If ctrl key is pressed down, then control character shall be input.
    if(key_event.modifiers & EVENTFLAG_CONTROL_DOWN) {
        key_event.character = KeyMapping::GetControlCharacter(
            windows_key_code, key_event.modifiers & EVENTFLAG_SHIFT_DOWN);
    } else {
        key_event.character = key_event.unmodified_character;
    }

    // This part has been modified
    if(down) {
        key_event.type = KEYEVENT_RAWKEYDOWN;
        inputreceiver->SendKeyEvent(key_event);

        // Pass the first //
        inputreceiver->SendKeyEvent(key_event);

        const auto storedHandled = InputProcessedByCEF;

        // Send char event //
        // TODO: should we pass the sdl text input event here
        key_event.type = KEYEVENT_CHAR;
        inputreceiver->SendKeyEvent(key_event);

        if(storedHandled)
            InputProcessedByCEF = true;
    } else {
        key_event.type = KEYEVENT_KEYUP;
        inputreceiver->SendKeyEvent(key_event);
    }

#if defined(OS_WIN)
#elif defined(OS_LINUX)
#elif defined(OS_MACOSX)
#else
#error Unknown platform
#endif // _WIN32
}

DLLEXPORT void Window::InjectMouseMove(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    // Only pass this data if we aren't going to pass our own captured mouse //
    if(!MouseCaptured) {

        if(!inputreceiver)
            return;

        CefMouseEvent cevent;
        cevent.x = event.motion.x;
        cevent.y = event.motion.y;

        inputreceiver->SendMouseMoveEvent(cevent, IsMouseOutsideWindowClientArea());
    }

    _CheckMouseVisibilityStates();
}

DLLEXPORT void Window::InjectMouseWheel(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    if(!MouseCaptured) {

        if(!inputreceiver)
            return;

        int x = event.wheel.x;
        int y = event.wheel.y;

        if(SDL_MOUSEWHEEL_FLIPPED == event.wheel.direction) {
            y *= -1;
        } else {
            x *= -1;
        }

        CefMouseEvent cevent;
        inputreceiver->SendMouseWheelEvent(cevent, x, y);
    }
}

DLLEXPORT void Window::InjectMouseButtonDown(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    if(!MouseCaptured) {

        if(!inputreceiver)
            return;

        CefMouseEvent cevent;
        cevent.x = event.button.x;
        cevent.y = event.button.y;

        int type = GetCEFButtonFromSdlMouseButton(event.button.button);
        if(type == -1)
            return;

        cef_mouse_button_type_t btype = static_cast<cef_mouse_button_type_t>(type);

        inputreceiver->SendMouseClickEvent(cevent, btype, false, 1);
    }
}

DLLEXPORT void Window::InjectMouseButtonUp(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    if(!MouseCaptured) {

        if(!inputreceiver)
            return;

        CefMouseEvent cevent;
        cevent.x = event.button.x;
        cevent.y = event.button.y;

        int type = GetCEFButtonFromSdlMouseButton(event.button.button);
        if(type == -1)
            return;

        cef_mouse_button_type_t btype = static_cast<cef_mouse_button_type_t>(type);

        inputreceiver->SendMouseClickEvent(cevent, btype, true, 1);
    }
}

DLLEXPORT void Window::InjectCodePoint(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    if(!MouseCaptured) {

        if(!inputreceiver)
            return;

        // DEBUG_BREAK;

        // CefKeyEvent event;
        // // event.character = utf32char;
        // event.type = KEYEVENT_CHAR;

        // inputreceiver->SendKeyEvent(event);
    }
}

DLLEXPORT void Window::InjectKeyDown(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    bool SentToController = false;

    // Try to pass to CEF //
    DoCEFInputPass(event, true);

    // Check is it now handled or not and continue //
    if(!InputProcessedByCEF) {

        // Then try disabling collections //
        // LOG_WRITE("TODO: check is a text box active");
        // if(!OwningWindow->GetGui()->ProcessKeyDown(sdlkey, SpecialKeyModifiers)) {

        // Finally send to a controller //
        SentToController = true;
        GetInputController()->OnInputGet(event.key.keysym.sym, SpecialKeyModifiers, true);
        // }
    }

    if(!SentToController) {
        GetInputController()->OnBlockedInput(event.key.keysym.sym, SpecialKeyModifiers, true);
    }
}

DLLEXPORT void Window::InjectKeyUp(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    // Send to CEF if GUI is active //
    DoCEFInputPass(event, false);

    // This should always be passed here //
    GetInputController()->OnInputGet(event.key.keysym.sym, SpecialKeyModifiers, false);
}
// ------------------------------------ //

DLLEXPORT int32_t Window::ConvertStringToKeyCode(const std::string& str)
{

    auto key = SDL_GetKeyFromName(str.c_str());

    if(key == SDLK_UNKNOWN) {

        LOG_ERROR("Invalid SDL key name('" + str + "'): " + std::string(SDL_GetError()));
        return key;
    }

    return key;
}

DLLEXPORT std::string Window::ConvertKeyCodeToString(const int32_t& code)
{

    return std::string(SDL_GetKeyName(code));
}


} // namespace Leviathan
