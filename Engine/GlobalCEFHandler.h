// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GUI/GuiCEFApplication.h"
#include "GUI/LeviathanJavaScriptAsync.h"

#include "include/cef_task.h"
#include "include/internal/cef_ptr.h"

// Forward declare some things //
class CefScopedSandboxInfo;

#define REQUIRE_UI_THREAD() assert(CefCurrentlyOn(TID_UI));
#define REQUIRE_IO_THREAD() assert(CefCurrentlyOn(TID_IO));
#define REQUIRE_FILE_THREAD() assert(CefCurrentlyOn(TID_FILE));

namespace Leviathan {

//! \brief Keeps certain CEF objects allocated for long enough
class CEFSandboxInfoKeeper {
    friend GlobalCEFHandler;

public:
    CEFSandboxInfoKeeper();
    DLLEXPORT ~CEFSandboxInfoKeeper();

    //! \brief Gets the corresponding Gui::CefApplication
    DLLEXPORT CefRefPtr<GUI::CefApplication> GetCEFApp() const;

    void* GetPtr();

protected:
    std::shared_ptr<CefScopedSandboxInfo> ScopedInfo;
    CefRefPtr<GUI::CefApplication> CEFApp;
    void* SandBoxAccess;
};

//! \brief Singleton class for handling CEF initialization that needs to be done right away
class GlobalCEFHandler {
public:
    //! \brief This is the first function called in the Engine to handle CEF child processes
    //!
    //! This function will check command line arguments and possibly just run a subprocess or
    //! continue with the main application Passing command line argument of "--nogui" will skip
    //! this step and CEF initialization
    DLLEXPORT static bool CEFFirstCheckChildProcess(
        int argcount, char* args[], int& returnvalue,
        std::shared_ptr<CEFSandboxInfoKeeper>& keeper, const std::string& logname
#ifdef _WIN32
        ,
        HINSTANCE hInstance
#endif // _WIN32
    );

    DLLEXPORT static void CEFLastThingInProgram();
    DLLEXPORT static CEFSandboxInfoKeeper* GetCEFObjects();

    DLLEXPORT static void DoCEFMessageLoopWork();

    //! \brief Registers a new JavaScript query handler
    //! \param newdptr Pass a newed object or NULL
    //! \todo Add support for removing existing ones
    DLLEXPORT static void RegisterCustomJavaScriptQueryHandler(
        std::shared_ptr<GUI::JSAsyncCustom> ptr);
    //! \brief Unregisters a registered query handler
    //! \see RegisterCustomJavaScriptQueryHandler
    DLLEXPORT static void UnRegisterCustomJavaScriptQueryHandler(GUI::JSAsyncCustom* toremove);

    //! \brief Mainly allows LeviathanJavaScriptAsync to access the list of handlers
    DLLEXPORT static const std::vector<std::shared_ptr<GUI::JSAsyncCustom>>&
        GetRegisteredCustomHandlers();

    //! \brief Registers a LeviathanJavaScriptAsync to receive notifications about
    //! JSAsyncCustom changes
    DLLEXPORT static void RegisterJSAsync(GUI::LeviathanJavaScriptAsync* ptr);
    //! \brief Unregisters a registered LeviathanJavaScriptAsync
    //! \see RegisterJSAsync
    DLLEXPORT static void UnRegisterJSAsync(GUI::LeviathanJavaScriptAsync* ptr);


private:
    GlobalCEFHandler();
    ~GlobalCEFHandler();

protected:
    //! A flag for making sure that functions are only ran if CEF is actually used
    static bool CEFInitialized;
    static CEFSandboxInfoKeeper* AccessToThese;

    //! Stores all the custom handlers
    static std::vector<std::shared_ptr<GUI::JSAsyncCustom>> CustomJSHandlers;

    //! Stored to be able to notify all LeviathanJavaScriptAsync objects
    static std::vector<GUI::LeviathanJavaScriptAsync*> JSAsynToNotify;

    static std::recursive_mutex JSCustomMutex;
};

} // namespace Leviathan
