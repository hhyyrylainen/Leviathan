// ------------------------------------ //
#include "LeviathanJavaScriptAsync.h"

#include "Application/Application.h"
#include "GlobalCEFHandler.h"
#include "GuiView.h"
#include "Iterators/StringIterator.h"

#include "json/json.h"
using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
LeviathanJavaScriptAsync::LeviathanJavaScriptAsync(View* owner) : Owner(owner)
{
    GlobalCEFHandler::RegisterJSAsync(this);

    // We need to register all current ones //
    auto vec = GlobalCEFHandler::GetRegisteredCustomHandlers();

    GUARD_LOCK();

    for(auto iter = vec.begin(); iter != vec.end(); ++iter) {

        RegisterNewCustom(guard, (*iter));
    }
}

DLLEXPORT LeviathanJavaScriptAsync::~LeviathanJavaScriptAsync()
{
    GUARD_LOCK();
}
// ------------------------------------ //
DLLEXPORT void LeviathanJavaScriptAsync::BeforeRelease()
{
    GlobalCEFHandler::UnRegisterJSAsync(this);

    GUARD_LOCK();

    // Remove all //
    while(RegisteredCustomHandlers.size()) {

        RegisteredCustomHandlers[0]->CancelAllMine(this);
        RegisteredCustomHandlers.erase(RegisteredCustomHandlers.begin());
    }
}
// ------------------------------------ //
#define JS_ACCESSCHECK(x)              \
    if(_VerifyJSAccess(x, callback)) { \
        return true;                   \
    }

bool LeviathanJavaScriptAsync::OnQuery(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame, int64 query_id, const CefString& request, bool persistent,
    CefRefPtr<Callback> callback)
{
    // First handle with default handlers //
    if(request == "LeviathanVersion") {
        // Security check //
        JS_ACCESSCHECK(VIEW_SECURITYLEVEL_MINIMAL);

        // Return the wanted value //
        callback->Success(LEVIATHAN_VERSIONS);

        return true;
    } else if(request == "Quit") {
        // Security check //
        JS_ACCESSCHECK(VIEW_SECURITYLEVEL_ACCESS_ALL);

        // Post quit message //
        Logger::Get()->Info("JS API Quit called from GUI View: " + std::to_string(Owner->ID));
        LeviathanApplication::Get()->MarkAsClosing();

        // Return success indication //
        callback->Success("1");

        return true;
    }

    GUARD_LOCK();

    // Try to use any custom handlers //
    for(size_t i = 0; i < RegisteredCustomHandlers.size(); i++) {
        if(RegisteredCustomHandlers[i]->ProcessQuery(
               this, request, query_id, persistent, callback)) {
            // It got handled //
            return true;
        }
    }

    // Not a valid query //
    return false;
}
// ------------------------------------ //
void LeviathanJavaScriptAsync::OnQueryCanceled(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id)
{
    // Remove from the main queue //


    // If not removed here, it must be a custom interface's query //

    // Tell all custom interfaces that this query is canceled //
    for(size_t i = 0; i < RegisteredCustomHandlers.size(); i++) {
        RegisteredCustomHandlers[i]->CancelQuery(this, query_id);
    }
}
// ------------------------------------ //
DLLEXPORT void LeviathanJavaScriptAsync::RegisterNewCustom(
    Lock& guard, std::shared_ptr<JSAsyncCustom> newhandler)
{
    RegisteredCustomHandlers.push_back(newhandler);
}
// ------------------------------------ //
DLLEXPORT void LeviathanJavaScriptAsync::UnregisterCustom(JSAsyncCustom* handler)
{
    // Cancel all matching ones //
    handler->CancelAllMine(this);

    GUARD_LOCK();

    for(size_t i = 0; i < RegisteredCustomHandlers.size(); i++) {

        if(RegisteredCustomHandlers[i].get() == handler) {
            RegisteredCustomHandlers.erase(RegisteredCustomHandlers.begin() + i);
            return;
        }
    }

    LOG_WARNING(
        "LeviathanJavaScriptAsync: UnregisterCustom: trying to remove nonexistent handler");
}
// ------------------ JSAsyncCustom ------------------ //
DLLEXPORT JSAsyncCustom::JSAsyncCustom()
{
    // Register is done by the code that constructs this class //
}

DLLEXPORT JSAsyncCustom::~JSAsyncCustom() {}
