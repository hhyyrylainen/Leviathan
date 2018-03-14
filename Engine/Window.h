// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/Types.h"
#include "CEGUI/InputEvent.h"

struct SDL_Window;

namespace Leviathan{

class GraphicalInputEntity;

//! window class
//! \todo Implement global lock for input handling
class Window{
    friend GraphicalInputEntity;
public:

    DLLEXPORT Window(SDL_Window* sdlwindow, GraphicalInputEntity* owner);
    DLLEXPORT ~Window();

    DLLEXPORT inline float GetAspectRatio() const{

        int32_t width, height;
        GetSize(width, height);
        
        return (static_cast<float>(width)) / height;
    }

    DLLEXPORT void GetSize(int32_t &width, int32_t &height) const;

    

    DLLEXPORT void SetHideCursor(bool toset);

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
                
    //! \brief Captures input for this window and passes it on
    DLLEXPORT void GatherInput(CEGUI::InputAggregator* receiver);

    //! \brief Passes initial mouse position to CEGUI
    DLLEXPORT void ReadInitialMouse(CEGUI::InputAggregator* receiver);

    DLLEXPORT uint32_t GetSDLID() const;

    // \todo add a way to force only one window to have mouse captured //
    DLLEXPORT void SetCaptureMouse(bool state);

    //! \brief Returns whether this window is focused
    //! \return True when the window has focus
    DLLEXPORT bool IsWindowFocused() const;


    // Key press callbacks
    DLLEXPORT void InjectMouseMove(int xpos, int ypos);

    DLLEXPORT void InjectMouseWheel(int xamount, int yamount);

    DLLEXPORT void InjectMouseButtonDown(int32_t whichbutton);

    DLLEXPORT void InjectMouseButtonUp(int32_t whichbutton);

    DLLEXPORT void InjectCodePoint(uint32_t utf32char);

    DLLEXPORT void InjectKeyDown(int32_t sdlkey);

    DLLEXPORT void InjectKeyUp(int32_t sdlkey);
    

    DLLEXPORT static CEGUI::MouseButton SDLToCEGUIMouseButton(int sdlbutton);
    

    DLLEXPORT static int32_t ConvertStringToKeyCode(const std::string &str);
    DLLEXPORT static std::string ConvertKeyCodeToString(const int32_t &code);

protected:

    void _CheckMouseVisibilityStates();

    void _FirstInputCheck();
    
private:

    //! Set null when the native window is no longer valid
    SDL_Window* SDLWindow = nullptr;
        
    GraphicalInputEntity* OwningWindow = nullptr;

    //! This is temporarily stored during input gathering
    CEGUI::InputAggregator* inputreceiver = nullptr;

    bool ThisFrameHandledCreate = false;
    int LastFrameDownMouseButtons = 0;
    
    // this is updated every time input is gathered //
    int SpecialKeyModifiers = 0;
    bool Focused = true;
    bool ApplicationWantCursorState;
    bool ForceMouseVisible = false;
    bool CursorState = true;

    bool FirstInput = true;

    bool MouseCaptured = false;
};


}

