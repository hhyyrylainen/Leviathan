// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Exceptions.h"
#include "GuiInputSettings.h"

#include <atomic>

union SDL_Event;

namespace Leviathan { namespace GUI {

//! \brief Base class for WidgetLayer and View (browser containers / CEF) to add to a
//! GuiManager
class Layer : public ReferenceCounted {
public:
    DLLEXPORT Layer(GuiManager* owner, Window* window, int renderorder);
    DLLEXPORT virtual ~Layer();

    //! \brief Must be called before destroying to release allocated resources
    DLLEXPORT void ReleaseResources();

    //! \brief Notifies all the widgets and layout that the size has changed
    //!
    //! Called by GuiManager
    DLLEXPORT void NotifyWindowResized();

    //! \brief Notifies this view whether it is in focus or not
    //!
    //! Called by GuiManager
    DLLEXPORT void NotifyFocusUpdate(bool focused);

    DLLEXPORT inline auto GetGuiManager() const
    {
        return Owner;
    }

    DLLEXPORT inline auto GetWindow() const
    {
        return Wind;
    }

    DLLEXPORT inline auto GetRenderOrder() const
    {
        return RenderOrder;
    }

    //! \brief Sets the input mode. This should be regularly called from game code to update
    //! how the key presses should be sent to this View (or not sent)
    DLLEXPORT inline void SetInputMode(INPUT_MODE newmode)
    {
        InputMode = newmode;
    }

    DLLEXPORT virtual inline INPUT_MODE GetInputMode() const
    {
        return InputMode;
    }

    //! \returns True if this has a focused input element
    DLLEXPORT inline bool HasFocusedInputElement() const
    {
        return InputFocused;
    }

    //! \returns True if there is a scrollable element under the mouse
    DLLEXPORT inline bool HasScrollableElementUnderCursor() const
    {
        return ScrollableElement;
    }

    //! \brief Returns the main scene of this layer that contains all renderables
    //! \exception InvalidState if this view has been released already
    // DLLEXPORT inline Ogre::SceneManager* GetScene()
    DLLEXPORT inline int GetScene()
    {
        // if(!BSFLayerHack)
        //     throw InvalidState("This layer has been released already");
        // return BSFLayerHack;
        DEBUG_BREAK;
        return -1;
    }

    // Input passing from Window
    //! \param specialkeymodifiers This has the Leviathan version of the modifier keys. Not
    //! needed often as the event contains this data, but CEF needs the CEF version of this so
    //! also this is passed
    DLLEXPORT virtual bool OnReceiveKeyInput(const SDL_Event& event, bool down, bool textinput,
        int mousex, int mousey, int specialkeymodifiers, int cefspecialkeymodifiers)
    {
        return false;
    }

    DLLEXPORT virtual void OnMouseMove(const SDL_Event& event, bool outsidewindow) {}

    DLLEXPORT virtual void OnMouseWheel(const SDL_Event& event) {}

    DLLEXPORT virtual void OnMouseButton(const SDL_Event& event, bool down) {}

protected:
    // Callbacks vor derived classes
    DLLEXPORT virtual void _DoReleaseResources() {}
    DLLEXPORT virtual void _OnWindowResized() {}
    DLLEXPORT virtual void _OnFocusChanged() {}


    //! \brief Adjusts the orthographic properties of the camera to match the window size to
    //! make the Scene world coordinates match up with pixels
    DLLEXPORT void AdjustCameraProperties();

protected:
    //! Unique ID
    const int ID;

    //! Stored access to matching window
    Window* const Wind;

    //! Owning GuiManager
    GuiManager* const Owner;

    // Input handling variables

    //! Current focus state, set with NotifyFocusUpdate
    bool OurFocus = false;

    //! The mode of input. used by GuiManager when deciding where to pass input
    std::atomic<INPUT_MODE> InputMode = INPUT_MODE::Menu;

    //! Support for input when text box is focused
    //! \todo This needs to be tracked per frame for CEF browsers. Not sure how that should be
    //! done
    std::atomic<bool> InputFocused = false;

    //! Support for scrolling when the mouse is over a scrollable thing
    //! \todo This needs to be tracked per frame for CEF browsers
    std::atomic<bool> ScrollableElement = false;

    int RenderOrder;


    // Rendering resources
    // Ogre::CompositorWorkspace* Workspace;
    // Ogre::SceneManager* Scene;
    // Ogre::Camera* Camera;
    // bs::HSceneObject CameraSO;
    // bs::HCamera Camera;
};

}} // namespace Leviathan::GUI
