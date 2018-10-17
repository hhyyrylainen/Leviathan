// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "GuiInputSettings.h"

#include "OgreMaterial.h"
#include "OgreTexture.h"


#include <atomic>

union SDL_Event;

namespace Leviathan { namespace GUI {

//! \brief Base class for WidgetContainer and View (browser containers / CEF) to add to a
//! GuiManager
class Layer : public ReferenceCounted {
public:
    DLLEXPORT Layer(GuiManager* owner, Window* window);
    DLLEXPORT virtual ~Layer();

    //! \brief Must be called before destroying to release allocated Ogre and other resources
    DLLEXPORT virtual void ReleaseResources();

    //! \brief Notifies the internal browser that the window has resized
    //!
    //! Called by GuiManager
    DLLEXPORT virtual void NotifyWindowResized();

    //! \brief Notifies the internal browser that focus has been updated
    //!
    //! Called by GuiManager
    DLLEXPORT virtual void NotifyFocusUpdate(bool focused);

    //! \brief Sets the order Layers are drawn in, higher value is draw under other Layers
    //! \param zcoord The z-coordinate, should be between -1 and 1, higher lower values mean
    //! that it will be drawn earlier
    //! \note Actually it most likely won't be drawn earlier, but it will overwrite everything
    //! below it (if it isn't transparent)
    //! \todo This is unimplemented
    DLLEXPORT virtual void SetZVal(float zcoord);

    DLLEXPORT inline auto GetGuiManager() const
    {
        return Owner;
    }

    DLLEXPORT inline auto GetWindow() const
    {
        return Wind;
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


    // Rendering resources
};

}} // namespace Leviathan::GUI
