// ------------------------------------ //
#include "Window.h"

#include "Engine.h"
#include <boost/assign/list_of.hpp>
#include "Exceptions.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "Rendering/GraphicalInputEntity.h"
#include "CEGUIInclude.h"
#include "Handlers/IDFactory.h"
#include "Utility/Convert.h"
#include "Input/Key.h"
#include "GUI/GuiManager.h"
#include "Input/InputController.h"
using namespace std;
// ------------------------------------ //

#include "XLibInclude.h"

#ifdef _WIN32
// we must have an int of size 32 bits //
#pragma intrinsic(_BitScanForward)

static_assert(sizeof(int) == 4, "int must be 4 bytes long for bit scan function");
#else
// We must use GCC built ins int __builtin_ffs (unsigned int x)
// Returns one plus the index of the least significant 1-bit of x, or
// if x is zero, returns zero.  So using __builtin_ffs(val)-1 should
// work

// X11 window focus find function //
XID Leviathan::Window::GetForegroundWindow(){
    // Method posted on stack overflow (split to two lines to not be too long
    //http://stackoverflow.com/questions/1014822/
    //how-to-know-which-window-has-focus-and-how-to-change-it

    VerifyRenderWindowHandle();

    XID win;

    int revert_to;
    XGetInputFocus(XDisplay, &win, &revert_to); // see man

    return win;
}


#endif

namespace Leviathan{

DLLEXPORT Leviathan::Window::Window(Ogre::RenderWindow* owindow,
    GraphicalInputEntity* owner) :
    OWindow(owindow),
    ID(IDFactory::GetID())
{

    OwningWindow = owner;
    // update focused this way //
    VerifyRenderWindowHandle();
    Focused = m_hwnd == GetForegroundWindow() ? true: false;

    // register as listener to get update notifications //
    Ogre::WindowEventUtilities::addWindowEventListener(OWindow, this);

    // cursor on top of window's windows isn't hidden //
    ApplicationWantCursorState = false;

#ifdef __linux
    // Create an invisible cursor //
    XColor black;
    static char noData[] = { 0,0,0,0,0,0,0,0 };
    black.red = black.green = black.blue = 0;

    Pixmap bitmapNoData = XCreateBitmapFromData(XDisplay, m_hwnd, noData, 8, 8);
    XInvCursor = XCreatePixmapCursor(XDisplay, bitmapNoData, bitmapNoData,
        &black, &black, 0, 0);

#endif

    SetupOISForThisWindow();
    _CreateOverlayScene();
}

DLLEXPORT Leviathan::Window::~Window(){
    // Unregister, just in case //
    Ogre::WindowEventUtilities::removeWindowEventListener(OWindow, this);
    
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::SetHideCursor(bool toset){
    ApplicationWantCursorState = toset;

    if(!ApplicationWantCursorState || ForceMouseVisible){
        // show cursor //
        if(!CursorState){
            CursorState = true;
            Logger::Get()->Info("Showing cursor");
        #ifdef _WIN32
            ShowCursor(TRUE);
        #else

            // Don't do anything if window is not valid anymore //
            if(!VerifyRenderWindowHandle())
                return;

            // Restore default cursor //
            XUndefineCursor(XDisplay, m_hwnd);
        #endif
        }
    } else {
        // hide cursor //
        if(CursorState){
            CursorState = false;
            Logger::Get()->Info("Hiding cursor");
        #ifdef _WIN32
            ShowCursor(FALSE);
        #else
            // Don't do anything if window is not valid anymore //
            if(!VerifyRenderWindowHandle())
                return;

            // Set the invisible map as our cursor //
            XDefineCursor(XDisplay, m_hwnd, XInvCursor);
        #endif
        }
    }
}

#ifdef _WIN32
DLLEXPORT void Leviathan::Window::SetMouseToCenter(){
    // update window handle before using //
    if(!VerifyRenderWindowHandle())
        return;

    if(m_hwnd == NULL){
        // window has closed //
        return;
    }

    POINT p;
    p.x = GetWidth()/2;
    p.y = GetHeight()/2;

    ClientToScreen(m_hwnd, &p);

    SetCursorPos(p.x, p.y);
}
#else
DLLEXPORT void Leviathan::Window::SetMouseToCenter(){

    if(!VerifyRenderWindowHandle())
        return;

    if(m_hwnd == 0){
        // window has closed //
        return;
    }

    // Use the X11 function to warp the cursor //
    XWarpPointer(XDisplay, 0, m_hwnd, 0, 0, 0, 0, GetWidth()/2, GetHeight()/2);
}
#endif

#ifdef _WIN32
DLLEXPORT void Leviathan::Window::GetRelativeMouse(int& x, int& y){
    // update window handle before using //
    if(!VerifyRenderWindowHandle())
        return;

    if(m_hwnd == NULL){
        // window has closed //
        x = y = 0;
        return;
    }

    POINT p;
    GetCursorPos(&p);

    ScreenToClient(m_hwnd, &p);

    x = p.x;
    y = p.y;
}
#else
DLLEXPORT void Leviathan::Window::GetRelativeMouse(int& x, int& y){

    if(!VerifyRenderWindowHandle())
        return;

    // To not segment fault on this, we need to pass in dummy pointers to unused values //
    XID spammyme;
    int spamme;
    unsigned int spammetoo;
    // Should return the right position //
    XQueryPointer(XDisplay, m_hwnd, &spammyme, &spammyme, &spamme, &spamme, &x, &y,
        &spammetoo);
}
#endif


DLLEXPORT bool Leviathan::Window::IsMouseOutsideWindowClientArea(){
    int X, Y;
    GetRelativeMouse(X, Y);
    // check the coordinates //
    if(X < 0 || Y < 0 || X > GetWidth() || Y > GetHeight()){
        return true;
    }

    return false;
}
    
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::CloseDown(){

    // Sometimes the window might be closed already so check is it safe to use //
    bool canusewindow = VerifyRenderWindowHandle();

    // First release our X11 stuff //
#ifdef __linux__

    if(canusewindow && XDisplay >= 0){
        // Undefine the created cursor //
        XFreeCursor(XDisplay, XInvCursor);
        XInvCursor = 0;
    }

#endif

    // Release Ogre resources //
    // Release the scene //
    Ogre::Root::getSingleton().destroySceneManager(OverlayScene);
    OverlayScene = NULL;

    // This has to be called before closing the window to avoid problems //
    ReleaseOIS();

    // Close the window //
    OWindow->destroy();

    // Report that the window is now closed //
    Logger::Get()->Info("Window: closing window("+
        Convert::ToString(OwningWindow->GetWindowNumber())+")");
}

DLLEXPORT void Leviathan::Window::ResizeWindow(const int &width, const int &height){
    // make ogre resize window //
    OWindow->resize(width, height);
}
// ------------------------------------ //
void Leviathan::Window::windowResized(Ogre::RenderWindow* rw){
    // \todo add callback notification
    OwningWindow->OnResize(GetWidth(), GetHeight());

    UpdateOISMouseWindowSize();
}

void Leviathan::Window::windowFocusChange(Ogre::RenderWindow* rw){
    // update handle to have it up to date //
    if(!VerifyRenderWindowHandle())
        return;

    //Focused = m_hwnd == GetFocus() ? true: false;
    Focused = m_hwnd == GetForegroundWindow() ? true: false;

    // update mouse //
    _CheckMouseVisibilityStates();

    OwningWindow->OnFocusChange(Focused);
}
// ------------------------------------ //
#ifdef _WIN32
bool Leviathan::Window::VerifyRenderWindowHandle(){

    void* WindowHwnd(0);

    OWindow->getCustomAttribute(Ogre::String("WINDOW"), &WindowHwnd);

    m_hwnd = reinterpret_cast<HWND>(WindowHwnd);

    if(IsInvalidated || !IsWindow(m_hwnd)){
        // not a window! //
        return false;
    }

    return true;
}
#else
bool Leviathan::Window::VerifyRenderWindowHandle(){

    if(IsInvalidated || !OWindow || OWindow->isClosed()){
        
        m_hwnd = 0;
        XDisplay = 0;
        return false;
    }

    void* xidval(0);

    OWindow->getCustomAttribute(Ogre::String("WINDOW"), &xidval);

    m_hwnd = reinterpret_cast<XID>(xidval);
    // We need the display too //
    void* xdisplay(0);

    OWindow->getCustomAttribute(Ogre::String("DISPLAY"), &xdisplay);

    XDisplay = reinterpret_cast<Display*>(xdisplay);

    return true;
}
#endif
// ------------------------------------ //
DLLEXPORT void Leviathan::Window::InvalidateWindow(){

    IsInvalidated = true;
}
// ------------------------------------ //
bool Leviathan::Window::SetupOISForThisWindow(){
    // creation parameters //
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    OWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
#if defined OIS_WIN32_PLATFORM
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
    pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
    pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
    pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
    pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif


    // create system with parameters //
    WindowsInputManager = OIS::InputManager::createInputSystem(pl);

    // link wanted devices //
    WindowMouse = static_cast<OIS::Mouse*>(WindowsInputManager->createInputObject(
            OIS::OISMouse, true));
    
    WindowKeyboard = static_cast<OIS::Keyboard*>(WindowsInputManager->createInputObject(
            OIS::OISKeyboard, true));

    // there can be multiple joysticks //
    WindowJoysticks.resize(WindowsInputManager->getNumberOfDevices(OIS::OISJoyStick));

    for(size_t i = 0; i < WindowJoysticks.size(); i++){

        WindowJoysticks[i] = static_cast<OIS::JoyStick*>(
            WindowsInputManager->createInputObject(OIS::OISJoyStick, true));

        WindowJoysticks[i]->setEventCallback(this);
    }

    // no need to check all keys (we can just get events for keys that are used) //
    WindowKeyboard->setEventCallback(this);
    WindowMouse->setEventCallback(this);

    WindowKeyboard->setTextTranslation(OIS::Keyboard::Unicode);

    UpdateOISMouseWindowSize();

    return true;
}

void Leviathan::Window::ReleaseOIS(){

    WindowKeyboard->setEventCallback(NULL);
    WindowMouse->setEventCallback(NULL);

    // Destroy objects and manager //
    WindowsInputManager->destroyInputObject(WindowMouse);
    WindowsInputManager->destroyInputObject(WindowKeyboard);

    while(WindowJoysticks.size() != 0){
        WindowsInputManager->destroyInputObject(WindowJoysticks[0]);
        WindowJoysticks.erase(WindowJoysticks.begin());
    }

    MouseCaptured = false;

    OIS::InputManager::destroyInputSystem(WindowsInputManager);
    // clear pointers //
    WindowKeyboard = NULL;
    WindowMouse = NULL;
}

void Leviathan::Window::UpdateOISMouseWindowSize(){
    // get dimensions //
    unsigned int width, height, depth;
    int top, left;
    OWindow->getMetrics(width, height, depth, left, top);
    // get mouse state and set it's dimensions //

    // Yes, this is a const object that is modified. But this is the intended method
    // for this, see: OISMouse.h
    const OIS::MouseState& ms = WindowMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

DLLEXPORT void Leviathan::Window::GatherInput(CEGUI::InputAggregator* receiver){
    // Quit if window closed //
    if(OWindow->isClosed() || !WindowKeyboard || !WindowMouse){

        Logger::Get()->Warning("Window: GatherInput: skipping due to closed input window");
        return;
    }

    inputreceiver = receiver;

    // On first frame we want to manually force mouse position send //
    if(FirstInput){
        FirstInput = false;

        int x, y;
        GetRelativeMouse(x, y);

        // Pass the initial position //
        inputreceiver->injectMousePosition((float)x, (float)y);

        _CheckMouseVisibilityStates();
    }


    // Set parameters that listener functions need //
    ThisFrameHandledCreate = false;

    // We should automatically be the input handler for this browser //

    // Capture the browser variable //
    OwningWindow->GetInputController()->StartInputGather();

    // Capture inputs and send events //
    WindowMouse->capture();


    // This causes window to receive events so we need to be ready to dispatch them to OIS //
    WindowKeyboard->capture();

    // Joysticks //
    for(size_t i = 0; i < WindowJoysticks.size(); i++){

        WindowJoysticks[i]->capture();
    }


    // Listener functions should have handled everything //
    ThisFrameHandledCreate = false;
    // Everything is now processed //
    inputreceiver = NULL;

    // We need to just handle our own mouse capture function //
    if(MouseCaptured && Focused){

        // get mouse relative to window center //
        int xmoved = 0, ymoved = 0;

        GetRelativeMouse(xmoved, ymoved);

        // subtract center of window //
        xmoved -= GetWidth()/2;
        ymoved -= GetHeight()/2;

        // Reset to the center of the window //
        SetMouseToCenter();

        // Pass input //
        OwningWindow->GetInputController()->SendMouseMovement(xmoved, ymoved);
    }
}

void Leviathan::Window::CheckInputState(){
    if(ThisFrameHandledCreate)
        return;

    // create keyboard special key states here //
    SpecialKeyModifiers = 0;

    if(WindowKeyboard->isModifierDown(OIS::Keyboard::Ctrl))
        SpecialKeyModifiers |= KEYSPECIAL_CTRL;
    if(WindowKeyboard->isModifierDown(OIS::Keyboard::Alt))
        SpecialKeyModifiers |= KEYSPECIAL_ALT;
    if(WindowKeyboard->isModifierDown(OIS::Keyboard::Shift))
        SpecialKeyModifiers |= KEYSPECIAL_SHIFT;
    if(WindowKeyboard->isKeyDown(OIS::KC_CAPITAL))
        SpecialKeyModifiers |= KEYSPECIAL_CAPS;
    if(WindowKeyboard->isKeyDown(OIS::KC_LWIN))
        SpecialKeyModifiers |= KEYSPECIAL_WIN;
    if(WindowKeyboard->isKeyDown(OIS::KC_SCROLL))
        SpecialKeyModifiers |= KEYSPECIAL_SCROLL;

    // Set the modifier keys to the input receiver //
    if(inputreceiver)
        inputreceiver->setModifierKeys(SpecialKeyModifiers & KEYSPECIAL_SHIFT,
            SpecialKeyModifiers & KEYSPECIAL_ALT,
            SpecialKeyModifiers & KEYSPECIAL_CTRL);

    ThisFrameHandledCreate = true;
}
// ------------------ Input listener functions ------------------ //
bool Leviathan::Window::keyPressed(const OIS::KeyEvent &arg){
    CheckInputState();

    bool SentToController = false;

    // First pass to CEGUI //
    bool usedkeydown = false;
    bool usedtext = false;

    if(arg.text){

        usedtext = inputreceiver->injectChar(arg.text);
    }

    // See if it does something cool like paste or select all //
    usedkeydown = inputreceiver->injectKeyDown(static_cast<CEGUI::Key::Scan>(arg.key));
        
    if(!usedkeydown && !usedtext){

        // Then try disabling collections //
        if(!OwningWindow->GetGUI()->ProcessKeyDown(arg.key, SpecialKeyModifiers)){

            // Finally send to a controller //
            SentToController = true;
            OwningWindow->GetInputController()->OnInputGet(arg.key, SpecialKeyModifiers, true);
        }
    }

    if(!SentToController){
        OwningWindow->GetInputController()->OnBlockedInput(arg.key, SpecialKeyModifiers, true);
    }


    return true;
}

bool Leviathan::Window::keyReleased(const OIS::KeyEvent &arg){
    CheckInputState();

    // This should always be passed here //
    OwningWindow->GetInputController()->OnInputGet(arg.key, SpecialKeyModifiers, false);
        
    return true;
}

bool Leviathan::Window::mouseMoved(const OIS::MouseEvent &arg){
    CheckInputState();
    // pass event to active Rocket context //
    // send all mouse related things (except buttons) //
    const OIS::MouseState& mstate = arg.state;

    // only pass this data if we aren't going to pass our own captured mouse //
    if(!MouseCaptured){

        // Pass both scroll and movement //
        inputreceiver->injectMousePosition((float)mstate.X.abs, (float)mstate.Y.abs);
        inputreceiver->injectMouseWheelChange((float)mstate.Z.rel);
    }

    _CheckMouseVisibilityStates();
    
    return true;
}

bool Leviathan::Window::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
    CheckInputState();

    const OIS::MouseState& mstate = arg.state;

    // pass event to active Rocket context //
    int differences = mstate.buttons^LastFrameDownMouseButtons;

    // find differences //
#ifdef _WIN32
    unsigned long index = 0;

    _BitScanForward(&index, differences);

#else
    int index = __builtin_ffs(differences)-1;
#endif

    // update old state //
    LastFrameDownMouseButtons |= 1 << index;


    int Keynumber = index;
    if(!MouseCaptured){

        CEGUI::MouseButton pressed = CEGUI::NoButton;

        if(Keynumber == 0){
            pressed = CEGUI::LeftButton;

        } else if(Keynumber == 1){
            pressed = CEGUI::RightButton;

        } else if(Keynumber == 2){
            pressed = CEGUI::MiddleButton;

        } else if (Keynumber == 3){
            pressed = CEGUI::X1Button;

        } else if (Keynumber == 4){
            pressed = CEGUI::X2Button;

        } else {
            // We actually don't want to pass this //
            return true;
        }

        inputreceiver->injectMouseButtonDown(pressed);
    }
    
    return true;
}

bool Leviathan::Window::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id){
    CheckInputState();

    // pass event to active Rocket context //
    int differences = arg.state.buttons^LastFrameDownMouseButtons;

    // find differences //
#ifdef _WIN32
    unsigned long index = 0;

    _BitScanForward(&index, differences);

#else
    int index = __builtin_ffs(differences)-1;
#endif

    // update old state //
    LastFrameDownMouseButtons ^= 1 << index;

    int Keynumber = index;

    CEGUI::MouseButton pressed = CEGUI::NoButton;

    if(Keynumber == 0){
        pressed = CEGUI::LeftButton;

    } else if(Keynumber == 1){
        pressed = CEGUI::RightButton;

    } else if(Keynumber == 2){
        pressed = CEGUI::MiddleButton;

    } else if (Keynumber == 3){
        pressed = CEGUI::X1Button;

    } else if (Keynumber == 4){
        pressed = CEGUI::X2Button;

    } else {
        // We actually don't want to pass this //
        return true;
    }

    inputreceiver->injectMouseButtonUp(pressed);

    // don't really know what to return
    return true;
}

bool Leviathan::Window::buttonPressed(const OIS::JoyStickEvent &arg, int button){
    CheckInputState();

    return true;
}

bool Leviathan::Window::buttonReleased(const OIS::JoyStickEvent &arg, int button){
    CheckInputState();

    return true;
}

bool Leviathan::Window::axisMoved(const OIS::JoyStickEvent &arg, int axis){
    CheckInputState();

    return true;
}

DLLEXPORT string Leviathan::Window::GetOISCharacterAsText(const OIS::KeyCode &code){
    return WindowKeyboard->getAsString(code);
}

void Leviathan::Window::_CreateOverlayScene(){
    // create scene manager //
    OverlayScene = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_INTERIOR, 1,
        Ogre::INSTANCING_CULLING_SINGLETHREAD, "Overlay_forWindow_"+Convert::ToString(ID));

    OverLayCamera = OverlayScene->createCamera("empty camera");
}

DLLEXPORT void Leviathan::Window::SendCloseMessage(){
    OWindow->destroy();
}

void Leviathan::Window::_CheckMouseVisibilityStates(){
    // Update focused here //
    VerifyRenderWindowHandle();
    Focused = m_hwnd == GetForegroundWindow() ? true: false;

    // force cursor visible check (if outside client area or mouse is unfocused on the window)
    if(IsMouseOutsideWindowClientArea() || !Focused){

        ForceMouseVisible = true;
    } else {

        ForceMouseVisible = false;
    }
    // update cursor state //
    SetHideCursor(ApplicationWantCursorState);
}
// ------------------------------------ //
DLLEXPORT Int4 Leviathan::Window::GetScreenPixelRect() const{

    Int4 result(0);
    unsigned int unused;
    unsigned int width;
    unsigned int height;
    // We can use Ogre to get the metrics
    OWindow->getMetrics(width, height, unused, result.X, result.Y);
    
    result.Z = width;
    result.W = height;
    return result;
}


DLLEXPORT Int2 Leviathan::Window::TranslateClientPointToScreenPoint(const Int2 &point) const{
#ifdef _WIN32

    POINT pt = {point.X, point.Y};
    // Windows api to the rescue //
    if(ClientToScreen(m_hwnd, &pt)){
        // Succeeded //
        return Int2(pt.x, pt.y);
    }

    // Failed //
    throw NotFound("could not translate point to screen space, invalid handle");
#else
    return point;
#endif // _WIN32
}



// ------------------ KeyCode conversion map ------------------ //
#define SIMPLEPAIR(x, y)	#x, OIS::y
#define SIMPLEONETOONE(x)	#x, OIS::KC_##x


boost::bimap<std::string, OIS::KeyCode> Leviathan::Window::CharacterToOISConvert =
    boost::assign::list_of<
        boost::bimap<string, OIS::KeyCode>::relation>
        (SIMPLEONETOONE(A))
        (SIMPLEONETOONE(B))
        (SIMPLEONETOONE(C))
        (SIMPLEONETOONE(D))
        (SIMPLEONETOONE(E))
        (SIMPLEONETOONE(F))
        (SIMPLEONETOONE(G))
        (SIMPLEONETOONE(H))
        (SIMPLEONETOONE(I))
        (SIMPLEONETOONE(J))
        (SIMPLEONETOONE(K))
        (SIMPLEONETOONE(L))
        (SIMPLEONETOONE(M))
        (SIMPLEONETOONE(N))
        (SIMPLEONETOONE(O))
        (SIMPLEONETOONE(P))
        (SIMPLEONETOONE(Q))
        (SIMPLEONETOONE(R))
        (SIMPLEONETOONE(S))
        (SIMPLEONETOONE(T))
        (SIMPLEONETOONE(U))
        (SIMPLEONETOONE(V))
        (SIMPLEONETOONE(W))
        (SIMPLEONETOONE(X))
        (SIMPLEONETOONE(Y))
        (SIMPLEONETOONE(Z))

        (SIMPLEONETOONE(F1))
        (SIMPLEONETOONE(F2))
        (SIMPLEONETOONE(F3))
        (SIMPLEONETOONE(F4))
        (SIMPLEONETOONE(F5))
        (SIMPLEONETOONE(F6))
        (SIMPLEONETOONE(F7))
        (SIMPLEONETOONE(F8))
        (SIMPLEONETOONE(F9))
        (SIMPLEONETOONE(F10))
        (SIMPLEONETOONE(F11))
        (SIMPLEONETOONE(F12))
        (SIMPLEONETOONE(F13))
        (SIMPLEONETOONE(F14))
        (SIMPLEONETOONE(F15))
        // OIS doesn't support these advanced keys //
        /*(SIMPLEONETOONE(F16))
          (SIMPLEONETOONE(F17))
          (SIMPLEONETOONE(F18))
          (SIMPLEONETOONE(F19))
          (SIMPLEONETOONE(F20))
          (SIMPLEONETOONE(F21))
          (SIMPLEONETOONE(F22))
          (SIMPLEONETOONE(F23))
          (SIMPLEONETOONE(F24))*/
    
        (SIMPLEONETOONE(HOME))

        (SIMPLEONETOONE(NUMPAD0))
        (SIMPLEONETOONE(NUMPAD1))
        (SIMPLEONETOONE(NUMPAD2))
        (SIMPLEONETOONE(NUMPAD3))
        (SIMPLEONETOONE(NUMPAD4))
        (SIMPLEONETOONE(NUMPAD5))
        (SIMPLEONETOONE(NUMPAD6))
        (SIMPLEONETOONE(NUMPAD7))
        (SIMPLEONETOONE(NUMPAD8))
        (SIMPLEONETOONE(NUMPAD9))

        // some more "special" keys //
        ("LEFTARROW", OIS::KC_LEFT)
        ("RIGHTARROW", OIS::KC_RIGHT)
        ("UPARROW", OIS::KC_UP)
        ("DOWNARROW", OIS::KC_DOWN)
        ("ESC", OIS::KC_ESCAPE)
        ;


DLLEXPORT OIS::KeyCode Leviathan::Window::ConvertStringToOISKeyCode(const string &str){

    auto iter = CharacterToOISConvert.left.find(str);

    if(iter == CharacterToOISConvert.left.end()){

        throw NotFound("keyvalue not found in map");
    }

    return iter->second;
}

DLLEXPORT string Leviathan::Window::ConvertOISKeyCodeToString(const OIS::KeyCode &code){
    
    auto iter = CharacterToOISConvert.right.find(code);

    if(iter == CharacterToOISConvert.right.end()){

        throw NotFound("keyvalue not found in map");
    }

    return iter->second;
}

DLLEXPORT bool Leviathan::Window::IsWindowFocused() const{
    return Focused;
}
}
