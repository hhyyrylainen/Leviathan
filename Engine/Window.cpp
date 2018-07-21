// ------------------------------------ //
#include "Window.h"

#include "Engine.h"
#include "Entities/GameWorld.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "GUI/GuiManager.h"
#include "GUI/GuiView.h"
#include "GUI/KeyMapping.h"
#include "Handlers/IDFactory.h"
#include "Input/InputController.h"
#include "Input/Key.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Rendering/Graphics.h"
#include "Threading/ThreadingManager.h"
#include "TimeIncludes.h"
#include "Utility/Convert.h"

#include "utf8.h"

#ifdef __linux
#include "XLibInclude.h"
#endif

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
    CurrentCursor(nullptr, nullptr), CurrentCursorImage(nullptr, nullptr),
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
    // But that is quite hard to detect
    SDL_StartTextInput();

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
    if(LinkedWorld == world)
        return;

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
DLLEXPORT void Window::SetX11Cursor(int cursor, int retrycount /*= 10*/)
{
    if(retrycount <= 0) {

        LOG_ERROR("Window: SetX11Cursor: retrycount reached");
        return;
    }

    Engine::Get()->AssertIfNotMainThread();

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if(!SDL_GetWindowWMInfo(SDLWindow, &wmInfo)) {

        LOG_ERROR("Window: SetX11Cursor: failed to retrieve wm info");
        return;
    }

    // Retrieve the X11 display shared with Chromium.
    ::Display* xdisplay = cef_get_xdisplay();
    if(!xdisplay) {

        LOG_ERROR("Window: SetX11Cursor: cef_get_xdisplay failed");
        return;
    }

    WantedX11Cursor = cursor;
    XDefineCursor(xdisplay, wmInfo.info.x11.window, cursor);
    if(Graphics::HasX11ErrorOccured()) {
        LOG_ERROR("Window: SetX11Cursor: failed due to x11 error (on define cursor), retrying "
                  "in 50ms");

        ThreadingManager::Get()->QueueTask(std::make_shared<DelayedTask>(
            [=]() {
                Engine::Get()->Invoke([=]() { this->SetX11Cursor(cursor, retrycount - 1); });
            },
            MillisecondDuration(50)));
        return;
    }

    // And we need to make it permanent so that the cursor isn't returned to default after it
    // leaves and re-enters our window

    // // Todo make this more robust somehow
    // ThreadingManager::Get()->QueueTask(std::make_shared<DelayedTask>(
    //     [=]() { Engine::Get()->Invoke([=]() { this->MakeX11CursorPermanent(); }); },
    //     MillisecondDuration(10)));

    MakeX11CursorPermanent();
}

DLLEXPORT void Window::MakeX11CursorPermanent()
{
    Engine::Get()->AssertIfNotMainThread();

    // This needs to be retried if the cursor isn't over our window currently
    if(IsMouseOutsideWindowClientArea()) {

        LOG_ERROR("Window: MakeX11CursorPermanent: failed because mouse is outside area, "
                  "retrying SetX11Cursor in 50ms and after mouse is inside");

        ThreadingManager::Get()->QueueTask(std::make_shared<ConditionalDelayedTask>(
            [=]() { Engine::Get()->Invoke([=]() { this->SetX11Cursor(WantedX11Cursor); }); },
            [=]() { return !this->IsMouseOutsideWindowClientArea(); },
            MillisecondDuration(50)));
        return;
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if(!SDL_GetWindowWMInfo(SDLWindow, &wmInfo)) {

        LOG_ERROR("Window: MakeX11CursorPermanent: failed to retrieve wm info");
        return;
    }

    ::Display* xdisplay = cef_get_xdisplay();
    if(!xdisplay) {

        LOG_ERROR("Window: SetX11Cursor: cef_get_xdisplay failed");
        return;
    }

    ::Display* usedDisplay = xdisplay;
    // ::Display* usedDisplay = wmInfo.info.x11.display;

    // Verify that we can use XFixes
    int event_basep;
    int error_basep;
    if(!XFixesQueryExtension(usedDisplay, &event_basep, &error_basep)) {

        LOG_ERROR("Window: MakeX11CursorPermanent: failed due to XFixes extension not being "
                  "available / failing to load, retrying in 50ms");
    queueretrylabel:
        ThreadingManager::Get()->QueueTask(std::make_shared<DelayedTask>(
            [=]() { Engine::Get()->Invoke([=]() { this->MakeX11CursorPermanent(); }); },
            MillisecondDuration(50)));
        return;
    }

    // Now we can grab the image and feed it to SDL
    // auto* tmpImg = XFixesGetCursorImage(xdisplay);
    auto* tmpImg = XFixesGetCursorImage(usedDisplay);
    if(Graphics::HasX11ErrorOccured()) {
        LOG_ERROR("Window: MakeX11CursorPermanent: failed due to x11 error "
                  "(XFixesGetCursorImage), retrying in 50ms");

        goto queueretrylabel;
    }

    std::unique_ptr<XFixesCursorImage, int (*)(void*)> xCursor(tmpImg, XFree);


    // The pixels format is A8R8G8B8
    std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> image(
        SDL_CreateRGBSurfaceWithFormat(
            0, xCursor->width, xCursor->height, 32, SDL_PIXELFORMAT_ARGB8888),
        SDL_FreeSurface);

    if(!image) {

        LOG_ERROR("Window: SetX11Cursor: failed to create SDL surface");
        return;
    }

    // We need to copy the data to the SDL pixels and undo alpha premultiply
    const size_t dataSize = xCursor->width * xCursor->height;

    // uint8_t* data = reinterpret_cast<uint8_t*>(xCursor->pixels);
    uint8_t* pixels = static_cast<uint8_t*>(image->pixels);

    for(size_t i = 0, j = 0; i < dataSize; ++i, j += 4) {

        // I have no clue why this bit shifting and or works to get the right colour
        // approach taken from here
        // https://github.com/landryb/xfce4-screenshooter/blob/master/lib/screenshooter-capture.c
        const auto fixedColour = (static_cast<uint32_t>(xCursor->pixels[i]) << 8) |
                                 (static_cast<uint32_t>(xCursor->pixels[i]) >> 24);
        // Alpha
        pixels[j + 0] = fixedColour >> 24;
        // Red
        pixels[j + 1] = (fixedColour >> 16) & 0xff;
        // Green
        pixels[j + 2] = (fixedColour >> 8) & 0xff;
        // Blue
        pixels[j + 3] = fixedColour & 0xff;
    }

    std::unique_ptr<SDL_Cursor, void (*)(SDL_Cursor*)> newCursor(
        SDL_CreateColorCursor(image.get(), xCursor->xhot, xCursor->yhot), SDL_FreeCursor);

    if(!newCursor) {

        LOG_ERROR("Window: SetX11Cursor: failed to create sdl cursor");
        return;
    }

    SDL_SetCursor(newCursor.get());

    // Overwrite the old ones releasing the resources after we are using the new cursor
    CurrentCursor = std::move(newCursor);
    CurrentCursorImage = std::move(image);
}
#endif //__linux

#ifdef _WIN32
DLLEXPORT void Window::SetWinCursor(HCURSOR cursor)
{
    std::unique_ptr<ICONINFO, void (*)(ICONINFO*)> info(new ICONINFO({0}), [](ICONINFO* icon) {
        if(icon->hbmColor)
            DeleteObject(icon->hbmColor);

        if(icon->hbmMask)
            DeleteObject(icon->hbmMask);
    });

    LEVIATHAN_ASSERT(info.get(), "Memory alloc failed");

    bool success = GetIconInfo(cursor, info.get());

    if(!success) {

        LOG_ERROR("Window: SetWinCursor: failed to get icon info from HCURSOR");
        return;
    }

    BITMAP colourBitmap;
    BITMAP maskBitmap;

    int colourBitmapSize = 0;
    int maskBitmapSize = 0;
    bool monochrome;

    // This can be none if the cursor is monochrome
    if(info->hbmColor) {
        colourBitmapSize = GetObjectA(info->hbmColor, sizeof(BITMAP), &colourBitmap);

        if(colourBitmap.bmBitsPixel != 32) {

            LOG_ERROR("Window: SetWinCursor: got some weird colour bitmapsthat have "
                      "unexpected pixel depth");
            return;
        }

        if(colourBitmap.bmBits != nullptr)
            LOG_WARNING(
                "Window: SetWinCursor: bitmap retrieve got pixel data, this will be leaked");

        monochrome = false;

    } else {
        // And the colour cursors don't need the mask so this is only retrieved fro monochrome
        maskBitmapSize = GetObjectA(info->hbmMask, sizeof(BITMAP), &maskBitmap);

        if(maskBitmap.bmBitsPixel != 1) {

            LOG_ERROR("Window: SetWinCursor: got some weird monochrome bitmapsthat have "
                      "unexpected pixel depth");
            return;
        }

        if(maskBitmap.bmBits != nullptr)
            LOG_WARNING(
                "Window: SetWinCursor: bitmap retrieve got pixel data, this will be leaked");

        monochrome = true;
    }

    if((monochrome && maskBitmapSize == 0) || (!monochrome && colourBitmapSize == 0)) {

        LOG_ERROR("Window: SetWinCursor: failed to get bitmap objects");
        return;
    }

    const auto width = monochrome ? maskBitmap.bmWidth : colourBitmap.bmWidth;
    const auto height = monochrome ? maskBitmap.bmHeight / 2 : colourBitmap.bmHeight;

    std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> image(
        SDL_CreateRGBSurfaceWithFormat(0, width, height, 32,
            // There's some weird stuff going on with this format. This should not be changed
            // This format is accurate for monochrome but colour cursors have the elements in a
            // different order, for some reason
            SDL_PIXELFORMAT_RGBA32),
        SDL_FreeSurface);

    if(!image) {

        LOG_ERROR("Window: SetWinCursor: failed to create SDL surface");
        return;
    }

    // Retrieve pixel data //
    std::unique_ptr<uint8_t[]> maskPixels;
    std::unique_ptr<uint8_t[]> colourPixels;

    HDC hdcScreen = GetDC(nullptr);

    // BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biPlanes = 1;
    bi.biCompression = BI_RGB; // Apparently this means also uncompressed
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    bool fail = false;

    if(monochrome) {
        // For some reason the colour cursors don't need this so save time retrieving

        bi.biWidth = maskBitmap.bmWidth;
        bi.biHeight = -maskBitmap.bmHeight;
        bi.biBitCount = 1;
        const auto maskByteCount = (maskBitmap.bmWidth / 8) * maskBitmap.bmHeight;
        bi.biSizeImage = maskByteCount;

        maskPixels = std::unique_ptr<uint8_t[]>(new uint8_t[maskByteCount]);

        int readBytes = GetDIBits(hdcScreen, info->hbmMask, 0, maskBitmap.bmHeight,
                            maskPixels.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS) *
                        maskBitmap.bmWidthBytes;
        if(readBytes != maskByteCount) {
            LOG_ERROR("Window: SetWinCursor: pixel read count is different (read) " +
                      std::to_string(readBytes) + " != " + std::to_string(maskByteCount));
            fail = true;
        }
    } else {

        bi.biWidth = colourBitmap.bmWidth;
        bi.biHeight = -colourBitmap.bmHeight;
        bi.biBitCount = 32;
        const auto colourByteCount = (colourBitmap.bmWidth * 4) * colourBitmap.bmHeight;
        bi.biSizeImage = colourByteCount;

        colourPixels = std::unique_ptr<uint8_t[]>(new uint8_t[colourByteCount]);

        int readBytes = GetDIBits(hdcScreen, info->hbmColor, 0, colourBitmap.bmHeight,
                            colourPixels.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS) *
                        colourBitmap.bmWidthBytes;
        if(readBytes != colourByteCount) {
            LOG_ERROR("Window: SetWinCursor: pixel read count is different (read) " +
                      std::to_string(readBytes) + " != " + std::to_string(colourByteCount));
            fail = true;
        }
    }

    ReleaseDC(nullptr, hdcScreen);

    if(fail) {
        LOG_ERROR("Window: SetWinCursor: failed to read bitmap pixels");
        return;
    }

    uint8_t* pixels = static_cast<uint8_t*>(image->pixels);
    uint8_t* directMask = maskPixels.get();
    uint8_t* directColour = colourPixels.get();

    // This if is outside the loop to not constantly hit it
    if(monochrome) {
        for(size_t y = 0; y < height; ++y) {
            for(size_t x = 0; x < width; ++x) {

                const auto pixelStart = y * width * 4 + (x * 4);

                const auto maskBit = (maskBitmap.bmWidthBytes * 8 * y) + x;
                const auto maskIndex = maskBit / 8;
                const auto indexInsideByte = maskBit % 8;
                const auto insideBitAnd = (0x80 >> indexInsideByte);

                const uint8_t maskByte = directMask[maskIndex];
                const bool visible = (maskByte & insideBitAnd) > 0;
                const auto mask = visible ? 255 : 0;

                const auto colourIndex = maskIndex + (maskBitmap.bmWidthBytes * height);
                const auto sourceByte = directMask[colourIndex];
                const bool sourcePixel = (sourceByte & insideBitAnd) > 0;
                const auto pixel = sourcePixel ? 255 : 0;

                // Red
                pixels[pixelStart + 0] = 0;
                // Green
                pixels[pixelStart + 1] = 0;
                // Blue
                pixels[pixelStart + 2] = 0;
                // Alpha
                pixels[pixelStart + 3] = pixel & mask;
            }
        }
    } else {
        for(size_t y = 0; y < height; ++y) {
            for(size_t x = 0; x < width; ++x) {

                const auto pixelStart = y * width * 4 + (x * 4);

                const auto sourcePixelStart = y * colourBitmap.bmWidthBytes + (x * 4);

                // Weird pixel order. This doesn't match the format for some reason
                // So we just copy the data from the HBITMAP and hope it is right
                // Also apparently applying the mask breaks everything
                // Red
                pixels[pixelStart + 0] = directColour[sourcePixelStart + 0] /*& mask*/;
                // Green
                pixels[pixelStart + 1] = directColour[sourcePixelStart + 1] /*& mask*/;
                // Blue
                pixels[pixelStart + 2] = directColour[sourcePixelStart + 2] /*& mask*/;
                // Alpha
                pixels[pixelStart + 3] = directColour[sourcePixelStart + 3] /*& mask*/;
            }
        }
    }

    std::unique_ptr<SDL_Cursor, void (*)(SDL_Cursor*)> newCursor(
        SDL_CreateColorCursor(image.get(), info->xHotspot, info->yHotspot), SDL_FreeCursor);

    if(!newCursor) {
        LOG_ERROR("Window: SetWinCursor: failed to create sdl cursor");
        return;
    }

    SDL_SetCursor(newCursor.get());

    // Overwrite the old ones releasing the resources after we are using the new cursor
    CurrentCursor = std::move(newCursor);
    CurrentCursorImage = std::move(image);
}
#endif //_WIN32
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
        // reset pointer to indicate that this object no longer captures mouse to this
        // window
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

DLLEXPORT void Window::GetRelativeMouse(int& x, int& y) const
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

DLLEXPORT void Window::GetUnclampedRelativeMouse(int& x, int& y) const
{
    if(!SDLWindow)
        return;

    int globalX, globalY;
    SDL_GetGlobalMouseState(&globalX, &globalY);

    int windowX, windowY;
    SDL_GetWindowPosition(SDLWindow, &windowX, &windowY);

    x = globalX - windowX;
    y = globalY - windowY;
}


DLLEXPORT void Window::GetNormalizedRelativeMouse(float& x, float& y) const
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

DLLEXPORT bool Window::IsMouseOutsideWindowClientArea() const
{
    int X, Y;
    GetUnclampedRelativeMouse(X, Y);

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
GUI::View* Window::GetGUIEventReceiver(GUI::INPUT_EVENT_TYPE type, int mousex, int mousey)
{
    // Don't pass to GUI if mouse capture is enabled
    if(MouseCaptured)
        return nullptr;

    GUI::View* view = WindowsGui->GetTargetViewForInput(type, mousex, mousey);
    return view;
}

DLLEXPORT void Window::_StartGatherInput()
{
    // Quit if window closed //
    if(!SDLWindow) {

        LOG_WARNING("Window: GatherInput: window is closed");
        return;
    }

    InputGatherStarted = true;

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
    // Initial mouse position
    if(!InitialMousePositionSent) {
        if(!IsMouseOutsideWindowClientArea()) {

            InitialMousePositionSent = true;

            // Build a mouse move from the current position and send it
            int x;
            int y;
            GetRelativeMouse(x, y);

            SDL_Event event;

            event.motion.x = x;
            event.motion.y = y;
            InjectMouseMove(event);
        }
    }

    InputGatherStarted = false;
}

void Window::_CheckMouseVisibilityStates()
{
    const bool outsideArea = IsMouseOutsideWindowClientArea();

    // force cursor visible check (if outside client area or mouse is unfocused on the
    // window)
    if(outsideArea || !Focused) {

        ForceMouseVisible = true;

    } else {

        ForceMouseVisible = false;
    }

    // update cursor state //
    SetHideCursor(ApplicationWantCursorState);
}
// ------------------ Input listener functions ------------------ //
bool Window::DoCEFInputPass(
    const SDL_Event& sdlevent, bool down, bool textinput, int mousex, int mousey)
{
    // Find active gui view that wants the event
    GUI::View* receiver = GetGUIEventReceiver(GUI::INPUT_EVENT_TYPE::Keypress, mousex, mousey);

    // Don't pass to GUI
    if(!receiver)
        return false;

    CefKeyEvent key_event;
    key_event.modifiers = CEFSpecialKeyModifiers;

#if defined(OS_WIN)
    if(!textinput) {
        // GDK compatible key code
        int asX11KeyCode = KeyMapping::SDLKeyToX11Key(sdlevent.key.keysym);

        // Modified to work here
        // Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
        // reserved. Use of this source code is governed by a BSD-style license that
        // can be found in the LICENSE file.
        // Based on WebKeyboardEventBuilder::Build from
        // content/browser/renderer_host/input/web_input_event_builders_gtk.cc.

        KeyboardCode windows_key_code = KeyMapping::GdkEventToWindowsKeyCode(asX11KeyCode);
        key_event.windows_key_code =
            KeyMapping::GetWindowsKeyCodeWithoutLocation(windows_key_code);

        // List is the lParam of WM_KEYDOWN or WM_SYSKEYDOWN
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms646280(v=vs.85).aspx
        // Also set the repeat count to 1 here
        key_event.native_key_code = 1;

        // WM_SYSKEYDOWN detection (hopefully...)
        if(key_event.modifiers & EVENTFLAG_ALT_DOWN)
            key_event.is_system_key = true;

    } else {
        // text input
        try {
            auto begin = std::begin(sdlevent.text.text);
            key_event.windows_key_code = utf8::next(begin, begin + strlen(sdlevent.text.text));
        } catch(const utf8::exception& e) {
            LOG_ERROR(
                "Window: CEF input handling failed to convert utf8 to utf32 codepoint: " +
                std::string(e.what()));
        }

        // This is the lParam of WM_CHAR
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms646276(v=vs.85).aspx
        // Just set the repeat count to 1 here
        key_event.native_key_code = 1;
    }

#elif defined(OS_LINUX)
    if(!textinput) {

        // GDK compatible key code
        int asX11KeyCode = KeyMapping::SDLKeyToX11Key(sdlevent.key.keysym);

        // Modified to work here
        // Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
        // reserved. Use of this source code is governed by a BSD-style license that
        // can be found in the LICENSE file.
        // Based on WebKeyboardEventBuilder::Build from
        // content/browser/renderer_host/input/web_input_event_builders_gtk.cc.

        KeyboardCode windows_key_code = KeyMapping::GdkEventToWindowsKeyCode(asX11KeyCode);
        key_event.windows_key_code =
            KeyMapping::GetWindowsKeyCodeWithoutLocation(windows_key_code);

        key_event.native_key_code = windows_key_code;
        // key_event.native_key_code = asX11KeyCode;


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
            key_event.unmodified_character = sdlevent.key.keysym.sym;
        }

        // If ctrl key is pressed down, then control character shall be input.
        if(key_event.modifiers & EVENTFLAG_CONTROL_DOWN) {
            key_event.character = KeyMapping::GetControlCharacter(
                windows_key_code, key_event.modifiers & EVENTFLAG_SHIFT_DOWN);
        } else {

            // key_event.character = key_event.unmodified_character;
        }
    } else {
        // text input
        try {
            auto begin = std::begin(sdlevent.text.text);
            key_event.character = utf8::next(begin, begin + strlen(sdlevent.text.text));
            key_event.unmodified_character = key_event.character;
            // VKEY_A 0x41 (this isn't used so this is just to quiet warnings)
            key_event.windows_key_code = 0x41;
            // Not needed
            // key_event.native_key_code = ;
        } catch(const utf8::exception& e) {
            LOG_ERROR(
                "Window: CEF input handling failed to convert utf8 to utf32 codepoint: " +
                std::string(e.what()));
        }
    }
#elif defined(OS_MACOSX)
#error TODO: mac version. See: cef/tests/cefclient/browser/browser_window_osr_mac.mm
// for how the event should be structured
#else
#error Unknown platform
#endif // _WIN32

    // Skip input if unkown key
    if(key_event.windows_key_code == 0)
        return false;

    if(down) {

        if(!textinput) {
            key_event.type = KEYEVENT_RAWKEYDOWN;
            receiver->GetBrowserHost()->SendKeyEvent(key_event);
        } else {
            // Send char event //
            key_event.type = KEYEVENT_CHAR;
            receiver->GetBrowserHost()->SendKeyEvent(key_event);
        }
    } else {
        if(!textinput) {
            key_event.type = KEYEVENT_KEYUP;
            receiver->GetBrowserHost()->SendKeyEvent(key_event);
        }
    }


    // Detecting if the key press was actually used is pretty difficult and might cause lag
    // so we don't do that
    return true;
}

DLLEXPORT void Window::InjectMouseMove(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    // Only pass this data if we aren't going to pass our own captured mouse //
    if(!MouseCaptured) {

        GUI::View* receiver =
            GetGUIEventReceiver(GUI::INPUT_EVENT_TYPE::Other, event.motion.x, event.motion.y);

        if(receiver) {

            CefMouseEvent cevent;
            cevent.x = event.motion.x;
            cevent.y = event.motion.y;

            // TODO: IsMouseOutsideWindowClientArea needs to be updated once we support
            // multiple browsers in a single window
            receiver->GetBrowserHost()->SendMouseMoveEvent(
                cevent, IsMouseOutsideWindowClientArea());
        }
    }

    _CheckMouseVisibilityStates();
}

DLLEXPORT void Window::InjectMouseWheel(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    if(!MouseCaptured) {

        int mouseX;
        int mouseY;
        GetRelativeMouse(mouseX, mouseY);
        // TODO: allow configuring if mouse wheel is considered a key
        GUI::View* receiver =
            GetGUIEventReceiver(GUI::INPUT_EVENT_TYPE::Scroll, mouseX, mouseY);

        if(receiver) {

            int x = static_cast<int>(event.wheel.x * MOUSE_SCROLL_MULTIPLIER);
            int y = static_cast<int>(event.wheel.y * MOUSE_SCROLL_MULTIPLIER);

            if(SDL_MOUSEWHEEL_FLIPPED == event.wheel.direction) {
                y *= -1;
            } else {
                x *= -1;
            }

            // LOG_INFO("Mouse scroll to CEF: " + std::to_string(y) + " " + std::to_string(x));
            CefMouseEvent cevent;
            receiver->GetBrowserHost()->SendMouseWheelEvent(cevent, x, y);
        } else {

            int x = event.wheel.x;
            int y = event.wheel.y;

            if(SDL_MOUSEWHEEL_FLIPPED == event.wheel.direction) {
                y *= -1;
            } else {
                x *= -1;
            }

            GetInputController()->OnScroll(x, y, SpecialKeyModifiers);
        }
    }
}

DLLEXPORT void Window::InjectMouseButtonDown(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    if(!MouseCaptured) {

        GUI::View* receiver =
            GetGUIEventReceiver(GUI::INPUT_EVENT_TYPE::Other, event.button.x, event.button.y);

        if(receiver) {
            CefMouseEvent cevent;
            cevent.x = event.button.x;
            cevent.y = event.button.y;

            int type = KeyMapping::GetCEFButtonFromSdlMouseButton(event.button.button);
            if(type == -1)
                return;

            cef_mouse_button_type_t btype = static_cast<cef_mouse_button_type_t>(type);

            receiver->GetBrowserHost()->SendMouseClickEvent(cevent, btype, false, 1);
        }
    }
}

DLLEXPORT void Window::InjectMouseButtonUp(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    if(!MouseCaptured) {

        GUI::View* receiver =
            GetGUIEventReceiver(GUI::INPUT_EVENT_TYPE::Other, event.button.x, event.button.y);

        if(receiver) {
            CefMouseEvent cevent;
            cevent.x = event.button.x;
            cevent.y = event.button.y;

            int type = KeyMapping::GetCEFButtonFromSdlMouseButton(event.button.button);
            if(type == -1)
                return;

            cef_mouse_button_type_t btype = static_cast<cef_mouse_button_type_t>(type);

            receiver->GetBrowserHost()->SendMouseClickEvent(cevent, btype, true, 1);
        }
    }
}

DLLEXPORT void Window::InjectCodePoint(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    // Try to pass to CEF //
    int mouseX;
    int mouseY;
    GetRelativeMouse(mouseX, mouseY);

    if(!DoCEFInputPass(event, true, true, mouseX, mouseY)) {

        // CEF didn't want it
    }
}

DLLEXPORT void Window::InjectKeyDown(const SDL_Event& event)
{
    if(!InputGatherStarted)
        _StartGatherInput();

    bool SentToController = false;

    // Try to pass to CEF //
    int mouseX;
    int mouseY;
    GetRelativeMouse(mouseX, mouseY);

    const auto handled = DoCEFInputPass(event, true, false, mouseX, mouseY);

    if(!handled) {

        // CEF didn't want it
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
    int mouseX;
    int mouseY;
    GetRelativeMouse(mouseX, mouseY);
    DoCEFInputPass(event, false, false, mouseX, mouseY);

    // This should always be passed here //
    GetInputController()->OnInputGet(event.key.keysym.sym, SpecialKeyModifiers, false);
}

} // namespace Leviathan
