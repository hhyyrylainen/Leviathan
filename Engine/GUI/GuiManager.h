// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Application/AppDefine.h"
#include "Common/ThreadSafe.h"

#include "bsfCore/BsCorePrerequisites.h"

#include <boost/intrusive_ptr.hpp>

namespace Leviathan { namespace GUI {

class Layer;

enum class INPUT_EVENT_TYPE : int { Keypress, Scroll, Other };

//! \brief GUI controller for a Window
class GuiManager {
    struct CutscenePlayStatus;

public:
    DLLEXPORT GuiManager();
    DLLEXPORT ~GuiManager();

    DLLEXPORT bool Init(Graphics* graph, Window* window);
    DLLEXPORT void Release();

    DLLEXPORT void GuiTick(int mspassed);
    DLLEXPORT void Render();

    //! \brief Notifies internal browsers
    DLLEXPORT void OnResize();

    //! \brief Notifies contexts about the change to appropriately lose focus on fields
    DLLEXPORT void OnFocusChanged(bool focused);

    // file loading //
    //! \brief Loads a GUI file
    //! \todo Make sure that loading a path with spaces in it works
    //! \note This is the CEF version of a GUI layer
    DLLEXPORT bool LoadGUIFile(const std::string& urlorpath, bool nochangelistener = false);

    //! \brief Creates a new GUI layer that uses a widget layout
    //!
    //! \param layoutname The name of the layout file to load. If "" then creates an empty
    //! layer
    //! \param


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
    //! \note The pointer is only safe to hang onto until UnLoadGUIFile is called
    DLLEXPORT Layer* GetLayerByIndex(size_t index);

    // called when mouse cannot be captured (should force at least one collection on) //
    DLLEXPORT void OnForceGUIOn();

    //! \brief If true the GUI will be inactive and things like mouse movement will be passed
    //! to the player movement (but key presses goe through the GUI so that it can react to GUI
    //! activation buttons)
    DLLEXPORT void SetDisableMouseCapture(bool newvalue);

    //! This is a temporary thing to just render a few image layers with BSF
    DLLEXPORT void NotifyAboutLayer(int layernumber, const bs::HTexture& texture);

protected:
    //! Is called by folder listeners to notify of Gui file changes
    void _FileChanged(const std::string& file, ResourceFolderListener& caller);

    void _SendChangedLayers() const;

private:
    //! Set when containing window of the GUI shouldn't be allowed to capture mouse
    bool GuiDisallowMouseCapture = true;

    //! True when GuiDisallowMouseCapture has been changed and the change needs to be applied
    bool GuiMouseUseUpdated = true;

    Window* ThisWindow = nullptr;

    //! The main file of the GUI from which it is loaded from
    std::string MainGUIFile;

    int ID;

    //! Used to stop listening for file changes
    int FileChangeID = 0;

    //! When set to true will reload files on next tick
    bool ReloadQueued = false;

    //! These are all the different Layers (web browsers and Leviathan Widgets) that are
    //! rendered on the Window of this GuiManager
    //! \todo Layer pushing and popping needs to have logic for focus changes
    std::vector<boost::intrusive_ptr<GUI::Layer>> ManagedLayers;

    int GuiViewCounter;

    //! Disables the GUI trying to capture the mouse when no collection is active
    bool DisableGuiMouseCapture = false;

    std::unique_ptr<CutscenePlayStatus> CurrentlyPlayingCutscene;

    //! Temporary layers for sending to BSF overlay renderer
    std::map<int, bs::HTexture> TempRenderedLayers;
};

}} // namespace Leviathan::GUI
