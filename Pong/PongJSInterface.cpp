#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_JSINTERFACE
#include "PongJSInterface.h"
#endif
#include "PongGame.h"
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
	} else if (request == "PongDisconnect"){
		
		JS_ACCESSCHECKPTR(VIEW_SECURITYLEVEL_NORMAL, caller);

		// Disconnect //
		PongGame::Get()->Disconnect("GUI JavaScript disconnect");

		// Return the result //
		callback->Success("1");
		return true;
	
	} else {
		// These requests are multi-part ones //
		const wstring tmpstr(request);
		WstringIterator itr(tmpstr);

		// Get the first part of it //
		const wstring funcname = *itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_WHITESPACE).get();

		// We can now compare the function name //
		if(funcname == L"PongConnect"){
			// Security check //
			JS_ACCESSCHECKPTR(VIEW_SECURITYLEVEL_ACCESS_ALL, caller);

			// Get the connect address //
			const wstring address = *itr.GetStringInQuotes(QUOTETYPE_BOTH).get();

			// Try to connect //
			wstring error;
			if(!PongGame::Get()->Connect(address, error)){
				// Failed //
				callback->Failure(1, error);
				return true;
			}

			// It worked //
			callback->Success("1");

			return true;
		}
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
