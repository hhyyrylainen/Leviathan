// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Application/AppDefine.h"
#include "Common/ThreadSafe.h"

#include <boost/intrusive_ptr.hpp>

namespace Leviathan { namespace GUI {

class Layer;

enum class INPUT_EVENT_TYPE : int { Keypress, Scroll, Other };

//! \brief Main GUI controller
//! \todo Add GUI window objects to this which are associated with different windows
class GuiManager {
public:
    DLLEXPORT GuiManager();
    DLLEXPORT ~GuiManager();

    //! \param ismain Set to true for first created GuiManager
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
    DLLEXPORT bool LoadGUIFile(const std::string& urlorpath, bool nochangelistener = false);

    //! \brief Unloads the currently loaded file
    DLLEXPORT void UnLoadGUIFile();

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

protected:
    //! Is called by folder listeners to notify of Gui file changes
    void _FileChanged(const std::string& file, ResourceFolderListener& caller);

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
    std::vector<boost::intrusive_ptr<GUI::Layer>> ManagedLayers;

    //! Disables the GUI trying to capture the mouse when no collection is active
    bool DisableGuiMouseCapture = false;
};

}} // namespace Leviathan::GUI
