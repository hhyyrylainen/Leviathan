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
    CefScopedSandboxInfo& windowssandbox
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

    void* windows_sandbox_info = nullptr;

#ifdef CEF_ENABLE_SANDBOX
    windows_sandbox_info = &sandbox;
#endif

    // Provide CEF with command-line arguments //
#ifdef _WIN32
    CefMainArgs main_args(hInstance);
#else
    // Make a copy of the args to not have CEF mess with them
    std::vector<char*> cefArgs;
    std::vector<std::string> cefArgData;

    cefArgs.resize(argcount);
    cefArgData.reserve(argcount);

    for(int i = 0; i < argcount; ++i) {

        cefArgData.push_back(args[i]);
        cefArgs[i] = cefArgData.back().data();
    }

    CefMainArgs main_args(cefArgs.size(), cefArgs.data());
    // CefMainArgs main_args(argcount, args);
#endif // _WIN32

    // Callback object //
    keeper->CEFApp = CefRefPtr<GUI::CefApplication>(new GUI::CefApplication());

    // Check are we a sub process //
    int exit_code = CefExecuteProcess(main_args, keeper->CEFApp.get(), windows_sandbox_info);
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
            boost::filesystem::canonical("Data/locales").wstring();

        const auto currentCanonical = boost::filesystem::canonical("./").wstring();

        // CefString(&settings.resources_dir_path) = currentCanonical;

        CefString(&settings.log_file) =
            currentCanonical + Convert::Utf8ToUtf16("/" + logname + "LogCEF.txt");

        const auto cachePath = currentCanonical + L"/Data/Cache/CEF";
        boost::filesystem::create_directories(cachePath);

        CefString(&settings.cache_path) = cachePath;

    } catch(const boost::filesystem::filesystem_error& e) {

        std::stringstream msg;
        msg << "Error missing file or accessing cache location: " << e.what() << "\n";


        std::ofstream write(std::string("Leviathan_start_failure_") +
#ifdef __linux
                            std::to_string(::getpid()) +
#endif //__linux
                            ".txt");
        write << msg.str();
        write << "Args are (" << argcount << ")" << std::endl;
        for(int i = 0; i < argcount; ++i)
            write << args[i] << std::endl;
        write << std::endl;
        write.close();

        std::cout << msg.str();
        abort();
    }

    // TODO: log_severity
    // settings.log_severity = cef_log_severity_t::LOGSEVERITY_DEBUG;

    // TODO: user agent

    settings.windowless_rendering_enabled = true;

    settings.external_message_pump = true;

    // Apparently this is missing from the windows version but not the linux version. For some
    // reason?
    // settings.single_process = false;

    // // Enable remote debugging
    // settings.remote_debugging_port = 9090;

    // Only works on windows
    // And the OnPaint assumes it is on the main thread so this doesn't work at all
    settings.multi_threaded_message_loop = false;

    // Initialize CEF.
    CefInitialize(main_args, settings, keeper->CEFApp.get(), windows_sandbox_info);

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

    // The automatic templates remove the need for this message, which won't be logged anyway
    // std::cout << "CEF shutdown called" << std::endl;
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
    std::unique_lock<std::recursive_mutex> guard(JSCustomMutex);

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

// ------------------------------------ //
DLLEXPORT void GlobalCEFHandler::RegisterCustomExtension(
    std::shared_ptr<GUI::CustomExtension> extension)
{
    CustomExtensions.push_back(extension);
    AccessToThese->CEFApp->RegisterCustomExtension(extension);
}

DLLEXPORT bool GlobalCEFHandler::HandleCustomExtensionProcessMessage(
    CefRefPtr<CefBrowser> browser, CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    // Pass to the extensions until it is handled //
    for(const auto& ext : CustomExtensions) {

        if(ext->MessageHandler) {

            if(ext->MessageHandler->OnProcessMessageReceived(browser, source_process, message))
                return true;
        }
    }

    return false;
}

std::vector<std::shared_ptr<GUI::CustomExtension>> GlobalCEFHandler::CustomExtensions;

// ------------------ CEFSandboxInfoKeeper ------------------ //
DLLEXPORT Leviathan::CEFApplicationKeeper::CEFApplicationKeeper() {}

DLLEXPORT Leviathan::CEFApplicationKeeper::~CEFApplicationKeeper() {}
// ------------------------------------ //
DLLEXPORT CefRefPtr<GUI::CefApplication> Leviathan::CEFApplicationKeeper::GetCEFApp() const
{
    return CEFApp;
}
