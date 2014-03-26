#include "Include.h"
// ------------------------------------ //
#ifndef 
#include ".h"
#endif
using namespace ;
using namespace Leviathan;
using namespace Gui;
// ------------------------------------ //
CustomJSInterface::CustomJSInterface(){


}

CustomJSInterface::~CustomJSInterface(){

}
// ------------------------------------ //
#define JS_ACCESSCHECKPTR(x, y) if(y->_VerifyJSAccess(x, callback)){return true;}
// ------------------------------------ //
bool CustomJSInterface::ProcessQuery(Leviathan::Gui::LeviathanJavaScriptAsync* caller, const CefString &request, 
    int64 queryid, bool persists, CefRefPtr<Callback> &callback)
{
    // Do whatever to handle this //
    if(request == "MyCustomRequest"){
        // Check rights //
        JS_ACCESSCHECKPTR(VIEW_SECURITYLEVEL_ACCESS_ALL, caller);
        // Return the result //
        callback->Success("1");
        return true;
    }
    // Not handled //
    return false;
}
        

void CustomJSInterface::CancelQuery(Leviathan::Gui::LeviathanJavaScriptAsync* caller, int64 queryid){
    // Remove the query matching caller and queryid //
}
// ------------------------------------ //
void CustomJSInterface::CancelAllMine(Leviathan::Gui::LeviathanJavaScriptAsync* me){
    // Remove all stored queries matching me and any id //
    
}
// ------------------------------------ //
