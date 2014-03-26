#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_JSINTERFACE
#include "PongJSInterface.h"
#endif
using namespace Pong;
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
    if(request == "PongVersion"){
        // Check rights //
        JS_ACCESSCHECKPTR(VIEW_SECURITYLEVEL_MINIMAL, caller);
        // Return the result //
        callback->Success(GAME_VERSIONS);
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
