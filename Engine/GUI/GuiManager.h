// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Application/AppDefine.h"
#include "Common/ThreadSafe.h"

#include <boost/intrusive_ptr.hpp>

namespace Leviathan { namespace GUI {

class Layer;
class WidgetLayer;
class Widget;
class ImageWidget;

enum class INPUT_EVENT_TYPE : int { Keypress, Scroll, Other };

//! \brief GUI controller for a Window
class GuiManager {
    struct CutscenePlayStatus;

public:
    DLLEXPORT GuiManager();
    DLLEXPORT ~GuiManager();

    DLLEXPORT bool Init(Graphics* graph, Window* window);
    DLLEXPORT void Release();

    //! This is mainly used for detecting mouse states and refreshing
    DLLEXPORT void GuiTick(int mspassed);

    //! \brief Used to let widgets do something before render
    DLLEXPORT void OnRender(float passed);

    //! \brief Notifies internal browsers
    DLLEXPORT void OnResize();

    //! \brief Notifies contexts about the change to appropriately lose focus on fields
    DLLEXPORT void OnFocusChanged(bool focused);

    //! \brief Loads a CEF Layer with the url
    //! \todo Make sure that loading a path with spaces in it works
    DLLEXPORT bool LoadCEFLayer(const std::string& urlorpath, bool nochangelistener = false);

    //! \brief Creates a new GUI layer that uses a widget layout
    //!
    //! \param layoutname The name of the layout file to load. If "" then creates an empty
    //! layer
    //! \param renderorder The Ogre workspace number this uses. 0 Is used by GameWorld. Must be
    //! >= GUI_WORKSPACE_BEGIN_ORDER and less than GUI_WORKSPACE_OVERLAY (as that is reserved
    //! for this GuiManager to use). Leave as -1 to automatically pick the next one
    DLLEXPORT Layer* LoadGUILayer(const std::string& layoutname, int renderorder = -1);

    //! \brief Creates a new Widget Layer and plays a fullscreen video in it
    //!
    //! This is a pretty huge function and the functionality should maybe split more
    //! \param onfinished Callback called when the video finishes or the user has skipped it
    //! \param onerror Callback for when an error occurs and the file can't be played. Only one
    //! of these callbacks is called
    DLLEXPORT void PlayCutscene(const std::string& file, std::function<void()> onfinished,
        std::function<void(const std::string&)> onerror, bool allowskip = true);

    //! \brief Cancels a playing video started by PlayCutscene
    DLLEXPORT void CancelCutscene();

    //! \brief Returns the Layer that should receive the event
    //! \see Window::GetGUIEventReceiver
    //! \todo Add support for multiple views inside a window. And prefer the active input if
    //! this is a keypress
    DLLEXPORT Layer* GetTargetLayerForInput(INPUT_EVENT_TYPE type, int mousex, int mousey);

    //! \brief Returns the Layer count
    DLLEXPORT inline auto GetLayerCount() const
    {
        return ManagedLayers.size();
    }

    //! \brief Gets a Layer by index
    //! \note The pointer is only safe to hang onto until a new gui layer is loaded or removed
    DLLEXPORT Layer* GetLayerByIndex(size_t index);

    //! Called when mouse cannot be captured (should force at least one collection on) //
    DLLEXPORT void OnForceGUIOn();

    //! \brief If true the GUI will be inactive and things like mouse movement will be passed
    //! to the player movement (but key presses go through the GUI so that it can react to GUI
    //! activation buttons)
    //! \todo This needs to be rethought out
    DLLEXPORT void SetDisableMouseCapture(bool newvalue);

    //
    // Software cursor methods
    //
    //! \brief Enables or disables showing a software cursor
    void SetSoftwareCursor(const std::string& cursor);

    //! \brief Updates software cursor position (if enabled)
    void SetCursorPosition(int x, int y);

    //! \brief Sets the visibility of the software cursor
    void SetSoftwareCursorVisible(bool visible);

protected:
    //! Is called by folder listeners to notify of Gui file changes
    void _FileChanged(const std::string& file, ResourceFolderListener& caller);

private:
    //! Set when containing window of the GUI shouldn't be allowed to capture mouse
    bool GuiDisallowMouseCapture = true;

    //! True when GuiDisallowMouseCapture has been changed and the change needs to be applied
    bool GuiMouseUseUpdated = true;

    Window* ThisWindow = nullptr;

    int ID;

    //! Used to stop listening for file changes
    int FileChangeID = 0;

    //! When set to true will reload files on next tick
    bool ReloadQueued = false;

    //! These are all the different Layers (web browsers and Leviathan Widgets) that are
    //! rendered on the Window of this GuiManager
    //! \todo Layer pushing and popping needs to have logic for focus changes
    std::vector<boost::intrusive_ptr<GUI::Layer>> ManagedLayers;

    //! Used to assign consecutive render orders for ManagedLayers. While keeping the Overlay
    //! last
    int GuiViewCounter = GUI_WORKSPACES_BEFORE;

    //! This layer contains the software cursor (if enabled) and the cutscene player
    boost::intrusive_ptr<WidgetLayer> OverlayLayer;

    //! The software cursor widget when software cursor is enabled
    boost::intrusive_ptr<ImageWidget> SoftwareCursorWidget;

    //! When true the cursor should be hidden on this window. Only if software cursor is
    //! enabled this has an effect. This is reported by the Window
    bool HideSoftwareCursor = false;

    //! Disables the GUI trying to capture the mouse when no collection is active
    bool DisableGuiMouseCapture = false;

    int CursorX = -100;
    int CursorY = 0;

    std::unique_ptr<CutscenePlayStatus> CurrentlyPlayingCutscene;
};

}} // namespace Leviathan::GUI
