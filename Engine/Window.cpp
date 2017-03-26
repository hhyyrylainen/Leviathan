// ------------------------------------ //
#include "Window.h"

#include "CEGUIInclude.h"
#include "Engine.h"
#include "Exceptions.h"
#include "GUI/GuiManager.h"
#include "Input/InputController.h"
#include "Input/Key.h"
#include "Rendering/GraphicalInputEntity.h"
#include "Utility/Convert.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include "Math/CommonMath.h"
#include <algorithm>

using namespace std;
// ------------------------------------ //

#ifdef _WIN32
// we must have an int of size 32 bits //
#pragma intrinsic(_BitScanForward)

static_assert(sizeof(int) == 4, "int must be 4 bytes long for bit scan function");
#else
// We must use GCC built ins int __builtin_ffs (unsigned int x)
// Returns one plus the index of the least significant 1-bit of x, or
// if x is zero, returns zero.  So using __builtin_ffs(val)-1 should
// work

// #include "XLibInclude.h"

// // X11 window focus find function //
// XID Leviathan::Window::GetForegroundWindow(){
//     // Method posted on stack overflow (split to two lines to not be too long
//     //http://stackoverflow.com/questions/1014822/
//     //how-to-know-which-window-has-focus-and-how-to-change-it

//     XID win;

//     int revert_to;
//     XGetInputFocus(XDisplay, &win, &revert_to); // see man

//     return win;
// }


#endif

namespace Leviathan{

CEGUI::Key::Scan SDLKeyToCEGUIKey(int32_t key);

DLLEXPORT Leviathan::Window::Window(SDL_Window* sdlwindow, GraphicalInputEntity* owner) :
    SDLWindow(sdlwindow)
{
    OwningWindow = owner;
    
    Focused = true;

    SDL_StartTextInput();

    // cursor on top of window's windows isn't hidden //
    ApplicationWantCursorState = false;
}

DLLEXPORT Leviathan::Window::~Window(){

    if(MouseCaptured){

        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    LOG_WRITE("TODO: check why calling SDL_DestroyWindow crashes in Ogre "
        "GLX plugin uninstall");
    //SDL_DestroyWindow(SDLWindow);
    SDL_HideWindow(SDLWindow);
    SDLWindow = nullptr;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::SetHideCursor(bool toset){
    ApplicationWantCursorState = toset;

    if(!ApplicationWantCursorState || ForceMouseVisible){
        // show cursor //
        if(!CursorState){
            CursorState = true;
            Logger::Get()->Info("Showing cursor");
            
            // Don't do anything if window is not valid anymore //
            if(!SDLWindow)
                return;
            
            SDL_ShowCursor(SDL_ENABLE);
        }
    } else {
        // hide cursor //
        if(CursorState){
            
            CursorState = false;
            Logger::Get()->Info("Hiding cursor");
            
            // Don't do anything if window is not valid anymore //
            if(!SDLWindow)
                return;
            
            SDL_ShowCursor(SDL_DISABLE);
        }
    }
}

DLLEXPORT void Leviathan::Window::SetMouseToCenter(){

    int32_t width, height;
    GetSize(width, height);
    SDL_WarpMouseInWindow(SDLWindow, width/2, height/2);
}

DLLEXPORT void Leviathan::Window::GetRelativeMouse(int& x, int& y){

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
    
    x = Leviathan::clamp(globalX, 0, width);
    y = Leviathan::clamp(globalY, 0, height);
}

DLLEXPORT bool Leviathan::Window::IsMouseOutsideWindowClientArea(){
    int X, Y;
    GetRelativeMouse(X, Y);
    
    int32_t width, height;
    GetSize(width, height);
    
    // check the coordinates //
    
    if(X < 0 || Y < 0 || X > width || Y > height){
        return true;
    }

    return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::GetSize(int32_t &width, int32_t &height) const{

    SDL_GetWindowSize(SDLWindow, &width, &height);
}
// ------------------------------------ //
DLLEXPORT uint32_t Leviathan::Window::GetSDLID() const{

    if(!SDLWindow)
        return -1;
    
    return SDL_GetWindowID(SDLWindow);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::SetCaptureMouse(bool state){

    if(MouseCaptured == state)
        return;

    MouseCaptured = state;

    if(MouseCaptured){

        SDL_SetRelativeMouseMode(SDL_TRUE);
        
    } else {

        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    
}

DLLEXPORT bool Leviathan::Window::IsWindowFocused() const{
    return Focused;
}

void Leviathan::Window::_FirstInputCheck(){

    if(FirstInput){
        
        FirstInput = false;

        int x, y;
        GetRelativeMouse(x, y);

        // Pass the initial position //
        inputreceiver->injectMousePosition((float)x, (float)y);

        _CheckMouseVisibilityStates();
    }
}

DLLEXPORT void Leviathan::Window::GatherInput(CEGUI::InputAggregator* receiver){
    // Quit if window closed //
    if(!SDLWindow){

        Logger::Get()->Warning("Window: GatherInput: skipping due to closed input window");
        return;
    }

    inputreceiver = receiver;

    // On first frame we want to manually force mouse position send //
    if(FirstInput)
        _FirstInputCheck();


    // Set parameters that listener functions need //
    ThisFrameHandledCreate = false;

    // Capture the browser variable //
    OwningWindow->GetInputController()->StartInputGather();

    SpecialKeyModifiers = 0;

    const auto mods = SDL_GetModState();
    
    if(mods & KMOD_CTRL)
        SpecialKeyModifiers |= KEYSPECIAL_CTRL;
    if(mods & KMOD_ALT)
        SpecialKeyModifiers |= KEYSPECIAL_ALT;
    if(mods & KMOD_SHIFT)
        SpecialKeyModifiers |= KEYSPECIAL_SHIFT;
    if(mods & KMOD_CAPS)
        SpecialKeyModifiers |= KEYSPECIAL_CAPS;
    if(mods & KMOD_GUI)
        SpecialKeyModifiers |= KEYSPECIAL_SUPER;

    // Set the modifier keys to the input receiver //
    if(inputreceiver)
        inputreceiver->setModifierKeys(SpecialKeyModifiers & KEYSPECIAL_SHIFT,
            SpecialKeyModifiers & KEYSPECIAL_ALT,
            SpecialKeyModifiers & KEYSPECIAL_CTRL);

    
    // Handle mouse capture
    if(MouseCaptured && Focused){

        // get mouse relative to window center //
        int xmoved = 0, ymoved = 0;

        SDL_GetRelativeMouseState(&xmoved, &ymoved);

        // Pass input //
        OwningWindow->GetInputController()->SendMouseMovement(xmoved, ymoved);
    }
}

DLLEXPORT void Leviathan::Window::ReadInitialMouse(CEGUI::InputAggregator* receiver){

    auto* old = inputreceiver;
    
    inputreceiver = receiver;
    _FirstInputCheck();

    inputreceiver = old;
}

void Leviathan::Window::_CheckMouseVisibilityStates(){
    
    // force cursor visible check (if outside client area or mouse is unfocused on the window)
    if(IsMouseOutsideWindowClientArea() || !Focused){

        ForceMouseVisible = true;
        
    } else {

        ForceMouseVisible = false;
    }
    
    // update cursor state //
    SetHideCursor(ApplicationWantCursorState);
}
// ------------------ Input listener functions ------------------ //
DLLEXPORT void Leviathan::Window::InjectMouseMove(int xpos, int ypos){
 
    // Only pass this data if we aren't going to pass our own captured mouse //
    if(!MouseCaptured){

        inputreceiver->injectMousePosition((float)xpos, (float)ypos);
    }

    _CheckMouseVisibilityStates();
}

DLLEXPORT void Leviathan::Window::InjectMouseWheel(int xamount, int yamount){

    if(!MouseCaptured){

        inputreceiver->injectMouseWheelChange((float)yamount);
    }
}

DLLEXPORT void Leviathan::Window::InjectMouseButtonDown(int32_t whichbutton){

    if(!MouseCaptured){

        inputreceiver->injectMouseButtonDown(SDLToCEGUIMouseButton(whichbutton));
    }
}

DLLEXPORT void Leviathan::Window::InjectMouseButtonUp(int32_t whichbutton){

    if(!MouseCaptured){

        inputreceiver->injectMouseButtonUp(SDLToCEGUIMouseButton(whichbutton));
    }
}

DLLEXPORT void Leviathan::Window::InjectCodePoint(uint32_t utf32char){

    if(!MouseCaptured){

        inputreceiver->injectChar(utf32char);
    }
}

DLLEXPORT void Leviathan::Window::InjectKeyDown(int32_t sdlkey){

    bool SentToController = false;

    // First pass to CEGUI //
    bool usedkeydown = false;
    bool usedtext = false;

    // See if it does something cool like paste or select all //
    usedkeydown = inputreceiver->injectKeyDown(SDLKeyToCEGUIKey(sdlkey));
    
    if(!usedkeydown){

        // Then try disabling collections //
        LOG_WRITE("TODO: check is a text box active");
        if(!OwningWindow->GetGui()->ProcessKeyDown(sdlkey, SpecialKeyModifiers)){

            // Finally send to a controller //
            SentToController = true;
            OwningWindow->GetInputController()->OnInputGet(sdlkey, SpecialKeyModifiers, true);
        }
    }

    if(!SentToController){
        OwningWindow->GetInputController()->OnBlockedInput(sdlkey, SpecialKeyModifiers, true);
    }
}

DLLEXPORT void Leviathan::Window::InjectKeyUp(int32_t sdlkey){
    
    // CEGUI doesn't need this as it handles things on key down
    // But it actually might so let's pass it
    inputreceiver->injectKeyUp(SDLKeyToCEGUIKey(sdlkey));

    // This should always be passed here //
    OwningWindow->GetInputController()->OnInputGet(sdlkey, SpecialKeyModifiers, false);
}



DLLEXPORT CEGUI::MouseButton Leviathan::Window::SDLToCEGUIMouseButton(int sdlbutton){

    if(sdlbutton & SDL_BUTTON_LEFT)
        return CEGUI::MouseButton::Left;

    if(sdlbutton & SDL_BUTTON_RIGHT)
        return CEGUI::MouseButton::Right;

    if(sdlbutton & SDL_BUTTON_MIDDLE)
        return CEGUI::MouseButton::Middle;

    if(sdlbutton & SDL_BUTTON_X1)
        return CEGUI::MouseButton::X1;

    if(sdlbutton & SDL_BUTTON_X2)
        return CEGUI::MouseButton::X2;

    LOG_WARNING("SDLToCEGUIMouseButton: unknown sdl button: " + Convert::ToString(sdlbutton));
    return CEGUI::MouseButton::Invalid;
}

// bool Leviathan::Window::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
//     CheckInputState();

//     const OIS::MouseState& mstate = arg.state;

//     // pass event to active Rocket context //
//     int differences = mstate.buttons^LastFrameDownMouseButtons;

//     // find differences //
// #ifdef _WIN32
//     unsigned long index = 0;

//     _BitScanForward(&index, differences);

// #else
//     int index = __builtin_ffs(differences)-1;
// #endif

//     // update old state //
//     LastFrameDownMouseButtons |= 1 << index;


//     int Keynumber = index;
//     if(!MouseCaptured){

//         CEGUI::MouseButton pressed = CEGUI::NoButton;

//         if(Keynumber == 0){
//             pressed = CEGUI::LeftButton;

//         } else if(Keynumber == 1){
//             pressed = CEGUI::RightButton;

//         } else if(Keynumber == 2){
//             pressed = CEGUI::MiddleButton;

//         } else if (Keynumber == 3){
//             pressed = CEGUI::X1Button;

//         } else if (Keynumber == 4){
//             pressed = CEGUI::X2Button;

//         } else {
//             // We actually don't want to pass this //
//             return true;
//         }

//         inputreceiver->injectMouseButtonDown(pressed);
//     }
    
//     return true;
// }

// bool Leviathan::Window::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
//     CheckInputState();

//     // pass event to active Rocket context //
//     int differences = arg.state.buttons^LastFrameDownMouseButtons;

//     // find differences //
// #ifdef _WIN32
//     unsigned long index = 0;

//     _BitScanForward(&index, differences);

// #else
//     int index = __builtin_ffs(differences)-1;
// #endif

//     // update old state //
//     LastFrameDownMouseButtons ^= 1 << index;

//     int Keynumber = index;

//     CEGUI::MouseButton pressed = CEGUI::NoButton;

//     if(Keynumber == 0){
//         pressed = CEGUI::LeftButton;

//     } else if(Keynumber == 1){
//         pressed = CEGUI::RightButton;

//     } else if(Keynumber == 2){
//         pressed = CEGUI::MiddleButton;

//     } else if (Keynumber == 3){
//         pressed = CEGUI::X1Button;

//     } else if (Keynumber == 4){
//         pressed = CEGUI::X2Button;

//     } else {
//         // We actually don't want to pass this //
//         return true;
//     }

//     inputreceiver->injectMouseButtonUp(pressed);

//     // don't really know what to return
//     return true;
// }

// ------------------------------------ //

DLLEXPORT int32_t Leviathan::Window::ConvertStringToKeyCode(const string &str){

    auto key = SDL_GetKeyFromName(str.c_str());

    if(key == SDLK_UNKNOWN){

        LOG_ERROR("Invalid SDL key name('" + str + "'): " + std::string(SDL_GetError()));
        return key;
    }

    return key;
}

DLLEXPORT string Leviathan::Window::ConvertKeyCodeToString(const int32_t &code){

    return std::string(SDL_GetKeyName(code));
}

/************************************************************************
Translate a SDLKey to the proper CEGUI::Key
*************************************************************************/
// From: http://cegui.org.uk/wiki/SDL_to_CEGUI_keytable
// Modified for SDL2
CEGUI::Key::Scan SDLKeyToCEGUIKey(int32_t key)
{
    using namespace CEGUI;
    switch (key)
    {
    case SDLK_BACKSPACE:    return CEGUI::Key::Scan::Backspace;
    case SDLK_TAB:          return CEGUI::Key::Scan::Tab;
    case SDLK_RETURN:       return CEGUI::Key::Scan::Return;
    case SDLK_PAUSE:        return CEGUI::Key::Scan::Pause;
    case SDLK_ESCAPE:       return CEGUI::Key::Scan::Esc;
    case SDLK_SPACE:        return CEGUI::Key::Scan::Space;
    case SDLK_COMMA:        return CEGUI::Key::Scan::Comma;
    case SDLK_MINUS:        return CEGUI::Key::Scan::Minus;
    case SDLK_PERIOD:       return CEGUI::Key::Scan::Period;
    case SDLK_SLASH:        return CEGUI::Key::Scan::ForwardSlash;
    case SDLK_0:            return CEGUI::Key::Scan::Zero;
    case SDLK_1:            return CEGUI::Key::Scan::One;
    case SDLK_2:            return CEGUI::Key::Scan::Two;
    case SDLK_3:            return CEGUI::Key::Scan::Three;
    case SDLK_4:            return CEGUI::Key::Scan::Four;
    case SDLK_5:            return CEGUI::Key::Scan::Five;
    case SDLK_6:            return CEGUI::Key::Scan::Six;
    case SDLK_7:            return CEGUI::Key::Scan::Seven;
    case SDLK_8:            return CEGUI::Key::Scan::Eight;
    case SDLK_9:            return CEGUI::Key::Scan::Nine;
    case SDLK_COLON:        return CEGUI::Key::Scan::Colon;
    case SDLK_SEMICOLON:    return CEGUI::Key::Scan::Semicolon;
    case SDLK_EQUALS:       return CEGUI::Key::Scan::Equals;
    case SDLK_LEFTBRACKET:  return CEGUI::Key::Scan::LeftBracket;
    case SDLK_BACKSLASH:    return CEGUI::Key::Scan::Backslash;
    case SDLK_RIGHTBRACKET: return CEGUI::Key::Scan::RightBracket;
    case SDLK_a:            return CEGUI::Key::Scan::A;
    case SDLK_b:            return CEGUI::Key::Scan::B;
    case SDLK_c:            return CEGUI::Key::Scan::C;
    case SDLK_d:            return CEGUI::Key::Scan::D;
    case SDLK_e:            return CEGUI::Key::Scan::E;
    case SDLK_f:            return CEGUI::Key::Scan::F;
    case SDLK_g:            return CEGUI::Key::Scan::G;
    case SDLK_h:            return CEGUI::Key::Scan::H;
    case SDLK_i:            return CEGUI::Key::Scan::I;
    case SDLK_j:            return CEGUI::Key::Scan::J;
    case SDLK_k:            return CEGUI::Key::Scan::K;
    case SDLK_l:            return CEGUI::Key::Scan::L;
    case SDLK_m:            return CEGUI::Key::Scan::M;
    case SDLK_n:            return CEGUI::Key::Scan::N;
    case SDLK_o:            return CEGUI::Key::Scan::O;
    case SDLK_p:            return CEGUI::Key::Scan::P;
    case SDLK_q:            return CEGUI::Key::Scan::Q;
    case SDLK_r:            return CEGUI::Key::Scan::R;
    case SDLK_s:            return CEGUI::Key::Scan::S;
    case SDLK_t:            return CEGUI::Key::Scan::T;
    case SDLK_u:            return CEGUI::Key::Scan::U;
    case SDLK_v:            return CEGUI::Key::Scan::V;
    case SDLK_w:            return CEGUI::Key::Scan::W;
    case SDLK_x:            return CEGUI::Key::Scan::X;
    case SDLK_y:            return CEGUI::Key::Scan::Y;
    case SDLK_z:            return CEGUI::Key::Scan::Z;
    case SDLK_DELETE:       return CEGUI::Key::Scan::DeleteKey;
    case SDLK_KP_0:         return CEGUI::Key::Scan::Numpad_0;
    case SDLK_KP_1:         return CEGUI::Key::Scan::Numpad_1;
    case SDLK_KP_2:         return CEGUI::Key::Scan::Numpad_2;
    case SDLK_KP_3:         return CEGUI::Key::Scan::Numpad_3;
    case SDLK_KP_4:         return CEGUI::Key::Scan::Numpad_4;
    case SDLK_KP_5:         return CEGUI::Key::Scan::Numpad_5;
    case SDLK_KP_6:         return CEGUI::Key::Scan::Numpad_6;
    case SDLK_KP_7:         return CEGUI::Key::Scan::Numpad_7;
    case SDLK_KP_8:         return CEGUI::Key::Scan::Numpad_8;
    case SDLK_KP_9:         return CEGUI::Key::Scan::Numpad_9;
    case SDLK_KP_PERIOD:    return CEGUI::Key::Scan::Decimal;
    case SDLK_KP_DIVIDE:    return CEGUI::Key::Scan::Divide;
    case SDLK_KP_MULTIPLY:  return CEGUI::Key::Scan::Multiply;
    case SDLK_KP_MINUS:     return CEGUI::Key::Scan::Subtract;
    case SDLK_KP_PLUS:      return CEGUI::Key::Scan::Add;
    case SDLK_KP_ENTER:     return CEGUI::Key::Scan::NumpadEnter;
    case SDLK_KP_EQUALS:    return CEGUI::Key::Scan::NumpadEquals;
    case SDLK_UP:           return CEGUI::Key::Scan::ArrowUp;
    case SDLK_DOWN:         return CEGUI::Key::Scan::ArrowDown;
    case SDLK_RIGHT:        return CEGUI::Key::Scan::ArrowRight;
    case SDLK_LEFT:         return CEGUI::Key::Scan::ArrowLeft;
    case SDLK_INSERT:       return CEGUI::Key::Scan::Insert;
    case SDLK_HOME:         return CEGUI::Key::Scan::Home;
    case SDLK_END:          return CEGUI::Key::Scan::End;
    case SDLK_PAGEUP:       return CEGUI::Key::Scan::PageUp;
    case SDLK_PAGEDOWN:     return CEGUI::Key::Scan::PageDown;
    case SDLK_F1:           return CEGUI::Key::Scan::F1;
    case SDLK_F2:           return CEGUI::Key::Scan::F2;
    case SDLK_F3:           return CEGUI::Key::Scan::F3;
    case SDLK_F4:           return CEGUI::Key::Scan::F4;
    case SDLK_F5:           return CEGUI::Key::Scan::F5;
    case SDLK_F6:           return CEGUI::Key::Scan::F6;
    case SDLK_F7:           return CEGUI::Key::Scan::F7;
    case SDLK_F8:           return CEGUI::Key::Scan::F8;
    case SDLK_F9:           return CEGUI::Key::Scan::F9;
    case SDLK_F10:          return CEGUI::Key::Scan::F10;
    case SDLK_F11:          return CEGUI::Key::Scan::F11;
    case SDLK_F12:          return CEGUI::Key::Scan::F12;
    case SDLK_F13:          return CEGUI::Key::Scan::F13;
    case SDLK_F14:          return CEGUI::Key::Scan::F14;
    case SDLK_F15:          return CEGUI::Key::Scan::F15;
    case SDLK_NUMLOCKCLEAR: return CEGUI::Key::Scan::NumLock;
    case SDLK_SCROLLLOCK:   return CEGUI::Key::Scan::ScrollLock;
    case SDLK_RSHIFT:       return CEGUI::Key::Scan::RightShift;
    case SDLK_LSHIFT:       return CEGUI::Key::Scan::LeftShift;
    case SDLK_RCTRL:        return CEGUI::Key::Scan::RightControl;
    case SDLK_LCTRL:        return CEGUI::Key::Scan::LeftControl;
    case SDLK_RALT:         return CEGUI::Key::Scan::RightAlt;
    case SDLK_LALT:         return CEGUI::Key::Scan::LeftAlt;
    case SDLK_LGUI:         return CEGUI::Key::Scan::LeftWindows;
    case SDLK_RGUI:         return CEGUI::Key::Scan::RightWindows;
    case SDLK_SYSREQ:       return CEGUI::Key::Scan::SysRq;
    case SDLK_MENU:         return CEGUI::Key::Scan::AppMenu;
    case SDLK_POWER:        return CEGUI::Key::Scan::Power;
    default:                return CEGUI::Key::Scan::Unknown;
    }
}


}
