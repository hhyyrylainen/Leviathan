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
class CEFApplicationKeeper {
    friend GlobalCEFHandler;

public:
    DLLEXPORT CEFApplicationKeeper();
    DLLEXPORT ~CEFApplicationKeeper();

    //! \brief Gets the corresponding Gui::CefApplication
    DLLEXPORT CefRefPtr<GUI::CefApplication> GetCEFApp() const;

protected:
    CefRefPtr<GUI::CefApplication> CEFApp;
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
        std::shared_ptr<CEFApplicationKeeper>& keeper, const std::string& logname
#ifdef _WIN32
#ifdef CEF_ENABLE_SANDBOX
        ,
        CefScopedSandboxInfo& sandbox
#endif
        ,
        HINSTANCE hInstance
#endif // _WIN32
    );

    DLLEXPORT static void CEFLastThingInProgram();
    DLLEXPORT static CEFApplicationKeeper* GetCEFObjects();

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

    //! \brief Registers a custom extension for all render processes to load as a V8 extension
    //! \note Only one should ever be registered, for performance reasons
    DLLEXPORT static void RegisterCustomExtension(
        std::shared_ptr<GUI::CustomExtension> extension);


    //! \brief Registers a LeviathanJavaScriptAsync to receive notifications about
    //! JSAsyncCustom changes
    DLLEXPORT static void RegisterJSAsync(GUI::LeviathanJavaScriptAsync* ptr);
    //! \brief Unregisters a registered LeviathanJavaScriptAsync
    //! \see RegisterJSAsync
    DLLEXPORT static void UnRegisterJSAsync(GUI::LeviathanJavaScriptAsync* ptr);

    //! \brief Handles passing a custom process message
    DLLEXPORT static bool HandleCustomExtensionProcessMessage(CefRefPtr<CefBrowser> browser,
        CefProcessId source_process, CefRefPtr<CefProcessMessage> message);

private:
    GlobalCEFHandler() = delete;
    ~GlobalCEFHandler() = delete;

protected:
    //! A flag for making sure that functions are only ran if CEF is actually used
    static bool CEFInitialized;
    static CEFApplicationKeeper* AccessToThese;

    //! Stores all the custom handlers
    static std::vector<std::shared_ptr<GUI::JSAsyncCustom>> CustomJSHandlers;

    //! Stored to be able to notify all LeviathanJavaScriptAsync objects
    static std::vector<GUI::LeviathanJavaScriptAsync*> JSAsynToNotify;

    static std::recursive_mutex JSCustomMutex;

    //! Custom extension storage here to allow routing messages to the right one
    static std::vector<std::shared_ptr<GUI::CustomExtension>> CustomExtensions;
};

} // namespace Leviathan
