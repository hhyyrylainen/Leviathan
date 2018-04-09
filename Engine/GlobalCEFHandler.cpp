// ------------------------------------ //
#include "GlobalCEFHandler.h"

#include "GUI/GuiCEFApplication.h"

#include "include/cef_app.h"

#ifdef _WIN32
#include "include/cef_sandbox_win.h"
#endif

#include <boost/filesystem.hpp>
using namespace Leviathan;
// ------------------------------------ //


DLLEXPORT bool Leviathan::GlobalCEFHandler::CEFFirstCheckChildProcess(
    int argcount, char* args[], int& returnvalue,
    std::shared_ptr<CEFSandboxInfoKeeper>& keeper, const std::string& logname
#ifdef _WIN32
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

    // create CEF3 objects //
    void* sandbox_info = NULL;

    keeper = std::shared_ptr<CEFSandboxInfoKeeper>(new CEFSandboxInfoKeeper());

#ifdef CEF_ENABLE_SANDBOX
    sandbox_info = keeper->GetPtr();
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
#if defined(CEF_ENABLE_SANDBOX) && defined(_WIN32)
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

#ifdef _WIN32
    // Let's try to speed things up //
    // Only works on windows
    settings.multi_threaded_message_loop = true;
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
DLLEXPORT CEFSandboxInfoKeeper* Leviathan::GlobalCEFHandler::GetCEFObjects()
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
        JSAsynToNotify[i]->RegisterNewCustom(ptr);
    }
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

CEFSandboxInfoKeeper* Leviathan::GlobalCEFHandler::AccessToThese = NULL;

bool Leviathan::GlobalCEFHandler::CEFInitialized = false;
// ------------------ CEFSandboxInfoKeeper ------------------ //
Leviathan::CEFSandboxInfoKeeper::CEFSandboxInfoKeeper() : SandBoxAccess(NULL)
{
#ifdef CEF_ENABLE_SANDBOX
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    ScopedInfo = std::shared_ptr<CefScopedSandboxInfo>(new CefScopedSandboxInfo());
    SandBoxAccess = ScopedInfo->sandbox_info();
#endif
}

DLLEXPORT Leviathan::CEFSandboxInfoKeeper::~CEFSandboxInfoKeeper() {}
// ------------------------------------ //
void* Leviathan::CEFSandboxInfoKeeper::GetPtr()
{
    return SandBoxAccess;
}

CefRefPtr<GUI::CefApplication> Leviathan::CEFSandboxInfoKeeper::GetCEFApp() const
{
    return CEFApp;
}
