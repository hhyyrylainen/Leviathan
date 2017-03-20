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
// bool Leviathan::Window::keyPressed(const OIS::KeyEvent &arg){
//     CheckInputState();

//     bool SentToController = false;

//     // First pass to CEGUI //
//     bool usedkeydown = false;
//     bool usedtext = false;

//     if(arg.text){

//         usedtext = inputreceiver->injectChar(arg.text);
//     }

//     // See if it does something cool like paste or select all //
//     usedkeydown = inputreceiver->injectKeyDown(static_cast<CEGUI::Key::Scan>(arg.key));
        
//     if(!usedkeydown && !usedtext){

//         // Then try disabling collections //
//         if(!OwningWindow->GetGUI()->ProcessKeyDown(arg.key, SpecialKeyModifiers)){

//             // Finally send to a controller //
//             SentToController = true;
//             OwningWindow->GetInputController()->OnInputGet(arg.key, SpecialKeyModifiers, true);
//         }
//     }

//     if(!SentToController){
//         OwningWindow->GetInputController()->OnBlockedInput(arg.key, SpecialKeyModifiers, true);
//     }


//     return true;
// }

// bool Leviathan::Window::keyReleased(const OIS::KeyEvent &arg){
//     CheckInputState();

//     // This should always be passed here //
//     OwningWindow->GetInputController()->OnInputGet(arg.key, SpecialKeyModifiers, false);
        
//     return true;
// }

DLLEXPORT void Leviathan::Window::InjectMouseMove(int xpos, int ypos){
 
    // only pass this data if we aren't going to pass our own captured mouse //
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



DLLEXPORT CEGUI::MouseButton Leviathan::Window::SDLToCEGUIMouseButton(int sdlbutton){

    if(sdlbutton & SDL_BUTTON_LEFT)
        return CEGUI::LeftButton;

    if(sdlbutton & SDL_BUTTON_RIGHT)
        return CEGUI::RightButton;

    if(sdlbutton & SDL_BUTTON_MIDDLE)
        return CEGUI::MiddleButton;

    if(sdlbutton & SDL_BUTTON_X1)
        return CEGUI::X1Button;

    if(sdlbutton & SDL_BUTTON_X2)
        return CEGUI::X2Button;

    LOG_WARNING("SDLToCEGUIMouseButton: unknown sdl button: " + Convert::ToString(sdlbutton));
    return CEGUI::NoButton;
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

        LOG_ERROR("Invalid SDL key name: " + std::string(SDL_GetError()));
        return key;
    }

    return key;
}

DLLEXPORT string Leviathan::Window::ConvertKeyCodeToString(const int32_t &code){

    return std::string(SDL_GetKeyName(code));
}

}
