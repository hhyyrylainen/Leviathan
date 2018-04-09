// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Application/AppDefine.h"
#include "Common/ThreadSafe.h"

#include "include/cef_browser.h"

#include <boost/intrusive_ptr.hpp>

namespace Leviathan { namespace GUI {

class View;

//! \brief Main GUI controller
//! \todo Add GUI window objects to this which are associated with different windows
class GuiManager {
public:
    DLLEXPORT GuiManager();
    DLLEXPORT ~GuiManager();

    //! \param ismain Set to true for first created GuiManager
    DLLEXPORT bool Init(Graphics* graph, Window* window, bool ismain);
    DLLEXPORT void Release();

    DLLEXPORT void GuiTick(int mspassed);
    DLLEXPORT void Render();

    //! \brief Notifies internal browsers
    //! \todo Make CEGUI allow multiple windows
    DLLEXPORT void OnResize();

    //! \brief Notifies contexts about the change to appropriately lose focus on fields
    DLLEXPORT void OnFocusChanged(bool focused);

    // file loading //
    //! \brief Loads a GUI file
    DLLEXPORT bool LoadGUIFile(const std::string& urlorpath, bool nochangelistener = false);

    //! \brief Unloads the currently loaded file
    DLLEXPORT void UnLoadGUIFile();

    // called when mouse cannot be captured (should force at least one collection on) //
    DLLEXPORT void OnForceGUIOn();

    //! \brief If true the GUI will be inactive and things like mouse movement will be passed
    //! to the player movement (but key presses goe through the GUI so that it can react to GUI
    //! activation buttons)
    DLLEXPORT void SetDisableMouseCapture(bool newvalue);

    //! \todo This needs to support multiple views somehow (maybe send to one the mouse is
    //! over?)
    DLLEXPORT CefRefPtr<CefBrowserHost> GetPrimaryInputReceiver();

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

    //! Set when this is the first created gui manager
    //! \detail Used for injecting time pulses into CEGUI
    bool MainGuiManager = false;

    int ID;

    //! Used to stop listening for file changes
    int FileChangeID = 0;

    //! When set to true will reload files on next tick
    bool ReloadQueued = false;

    std::vector<boost::intrusive_ptr<GUI::View>> ThissViews;

    //! Disables the GUI trying to capture the mouse when no collection is active
    bool DisableGuiMouseCapture = false;
};

}} // namespace Leviathan::GUI
