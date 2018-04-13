// ------------------------------------ //
#include ".h"
using namespace;
// ------------------------------------ //
CustomJSInterface::CustomJSInterface() {}

CustomJSInterface::~CustomJSInterface() {}
// ------------------------------------ //
#define JS_ACCESSCHECKPTR(x, y)           \
    if(y->_VerifyJSAccess(x, callback)) { \
        return true;                      \
    }
// ------------------------------------ //
bool CustomJSInterface::ProcessQuery(Leviathan::GUI::LeviathanJavaScriptAsync* caller,
    const CefString& request, int64 queryid, bool persists, CefRefPtr<Callback>& callback)
{
    // Do whatever to handle this //
    if(request == "MyCustomRequest") {
        // Check rights //
        JS_ACCESSCHECKPTR(Leviathan::GUI::VIEW_SECURITYLEVEL_ACCESS_ALL, caller);
        
        // Return the result //
        callback->Success("1");
        return true;
    }
    
    // Not handled //
    return false;
}

void CustomJSInterface::CancelQuery(
    Leviathan::GUI::LeviathanJavaScriptAsync* caller, int64 queryid)
{
    // Remove the query matching caller and queryid //
}
// ------------------------------------ //
void CustomJSInterface::CancelAllMine(Leviathan::GUI::LeviathanJavaScriptAsync* me)
{
    // Remove all stored queries matching me and any id //
}
// ------------------------------------ //
