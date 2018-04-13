// ------------------------------------ //
#include "GlobalCEFHandler.h"

#include "GUI/GuiCEFApplication.h"

#include "include/cef_app.h"

#include <boost/filesystem.hpp>

#include <iostream>
using namespace Leviathan;
// ------------------------------------ //


DLLEXPORT bool Leviathan::GlobalCEFHandler::CEFFirstCheckChildProcess(
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
)
{
    // Check for no graphics mode //
    for(int i = 0; i < argcount; ++i) {
        if(std::string_view(args[i]).find("--nogui") != std::string_view::npos) {
            // No gui commandline specified //
            std::cout << "Not using CEF because --nogui is specified" << std::endl;
            return false;
        }
    }

    // Run CEF startup //
    keeper = std::make_shared<CEFApplicationKeeper>();

    void* sandbox_info = nullptr;

#ifdef CEF_ENABLE_SANDBOX
    sandbox_info = &sandbox;
#endif

#ifdef __linux
    // Must force GPU disabled
    char** oldArgs = args;

    argcount += 1;
    args = new char*[argcount];

    std::unique_ptr<char[]> disablegpu(strdup("--disable-gpu"));

    for(int i = 0; i < argcount; ++i) {
        if(i < argcount - 1) {
            args[i] = oldArgs[i];
        } else {
            args[i] = disablegpu.get();
        }
    }

#endif

        // Provide CEF with command-line arguments //
#ifdef _WIN32
    CefMainArgs main_args(hInstance);
#else
    CefMainArgs main_args(argcount, args);
#endif // _WIN32

    // Callback object //
    keeper->CEFApp = CefRefPtr<GUI::CefApplication>(new GUI::CefApplication());

    // Check are we a sub process //
    int exit_code = CefExecuteProcess(main_args, keeper->CEFApp.get(), sandbox_info);
    if(exit_code >= 0) {
        // This was a sub process //
        returnvalue = exit_code;
        return true;
    }

    // Specify CEF global settings here.
    CefSettings settings;

    // Apparently this "just works" on non-windows platforms
#if !defined(CEF_ENABLE_SANDBOX) && defined(_WIN32)
    settings.no_sandbox = true;
#endif

    try {
        CefString(&settings.locales_dir_path) =
            boost::filesystem::canonical("locales").wstring();

        const auto currentCanonical = boost::filesystem::canonical("./").wstring();

        CefString(&settings.resources_dir_path) = currentCanonical;

        CefString(&settings.log_file) =
            currentCanonical + Convert::Utf8ToUtf16("/" + logname + "LogCEF.txt");

        const auto cachePath = currentCanonical + L"/Data/Cache/CEF";
        boost::filesystem::create_directories(cachePath);

        CefString(&settings.cache_path) = cachePath;

    } catch(const boost::filesystem::filesystem_error& e) {

        std::cout << "Error missing file or accessing cache location: " << e.what()
                  << std::endl;
        abort();
    }

    // TODO: log_severity

    // TODO: user agent

    settings.windowless_rendering_enabled = true;

    settings.single_process = false;

#ifdef _WIN32
    // Only works on windows
    // And the OnPaint assumes it is on the main thread so this doesn't work
    settings.multi_threaded_message_loop = false;
#endif

    // Initialize CEF.
    CefInitialize(main_args, settings, keeper->CEFApp.get(), sandbox_info);

    CEFInitialized = true;

    AccessToThese = keeper.get();

    // Wasn't a sub process //
    return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GlobalCEFHandler::CEFLastThingInProgram()
{
    if(!CEFInitialized)
        return;

    // Close it down //
    CefShutdown();
    std::cout << "CEF shutdown called" << std::endl;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GlobalCEFHandler::DoCEFMessageLoopWork()
{
    if(!CEFInitialized) {
        LOG_ERROR("DoCEFMessageLoopWork called before CEF is initialized");
        return;
    }

    CefDoMessageLoopWork();
}
// ------------------------------------ //
DLLEXPORT CEFApplicationKeeper* Leviathan::GlobalCEFHandler::GetCEFObjects()
{
    return AccessToThese;
}

DLLEXPORT void Leviathan::GlobalCEFHandler::RegisterCustomJavaScriptQueryHandler(
    std::shared_ptr<GUI::JSAsyncCustom> ptr)
{
    std::unique_lock<std::recursive_mutex> guard(JSCustomMutex);

    // Add it //
    CustomJSHandlers.push_back(ptr);

    // Notify all //
    for(size_t i = 0; i < JSAsynToNotify.size(); i++) {
        GUARD_LOCK_OTHER_NAME(JSAsynToNotify[i], guard2);
        JSAsynToNotify[i]->RegisterNewCustom(guard2, ptr);
    }

    // Things created after this will automatically retrieve the ones that are registered
    // before it is created
}

DLLEXPORT void Leviathan::GlobalCEFHandler::UnRegisterCustomJavaScriptQueryHandler(
    GUI::JSAsyncCustom* toremove)
{
    boost::unique_lock<std::recursive_mutex> guard(JSCustomMutex);

    // Notify all objects //
    for(size_t i = 0; i < JSAsynToNotify.size(); i++) {
        JSAsynToNotify[i]->UnregisterCustom(toremove);
    }

    // Compare pointers and remove it //
    for(size_t i = 0; i < CustomJSHandlers.size(); i++) {
        if(CustomJSHandlers[i].get() == toremove) {

            CustomJSHandlers.erase(CustomJSHandlers.begin() + i);
            return;
        }
    }
}

DLLEXPORT const std::vector<std::shared_ptr<GUI::JSAsyncCustom>>&
    Leviathan::GlobalCEFHandler::GetRegisteredCustomHandlers()
{
    return CustomJSHandlers;
}

DLLEXPORT void Leviathan::GlobalCEFHandler::RegisterJSAsync(GUI::LeviathanJavaScriptAsync* ptr)
{
    std::unique_lock<std::recursive_mutex> guard(JSCustomMutex);

    JSAsynToNotify.push_back(ptr);
}

DLLEXPORT void Leviathan::GlobalCEFHandler::UnRegisterJSAsync(
    GUI::LeviathanJavaScriptAsync* ptr)
{
    std::unique_lock<std::recursive_mutex> guard(JSCustomMutex);

    for(size_t i = 0; i < JSAsynToNotify.size(); i++) {

        if(JSAsynToNotify[i] == ptr) {

            JSAsynToNotify.erase(JSAsynToNotify.begin() + i);
            return;
        }
    }
}

std::recursive_mutex Leviathan::GlobalCEFHandler::JSCustomMutex;

std::vector<GUI::LeviathanJavaScriptAsync*> Leviathan::GlobalCEFHandler::JSAsynToNotify;

std::vector<std::shared_ptr<GUI::JSAsyncCustom>> Leviathan::GlobalCEFHandler::CustomJSHandlers;

CEFApplicationKeeper* Leviathan::GlobalCEFHandler::AccessToThese = NULL;

bool Leviathan::GlobalCEFHandler::CEFInitialized = false;
// ------------------ CEFSandboxInfoKeeper ------------------ //
DLLEXPORT Leviathan::CEFApplicationKeeper::CEFApplicationKeeper() {}

DLLEXPORT Leviathan::CEFApplicationKeeper::~CEFApplicationKeeper() {}
// ------------------------------------ //
DLLEXPORT CefRefPtr<GUI::CefApplication> Leviathan::CEFApplicationKeeper::GetCEFApp() const
{
    return CEFApp;
}
