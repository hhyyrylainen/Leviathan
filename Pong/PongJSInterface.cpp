#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_JSINTERFACE
#include "PongJSInterface.h"
#endif
#include "PongGame.h"
#include "jsoncpp\json.h"
using namespace Pong;
using namespace Leviathan;
using namespace Gui;
// ------------------------------------ //
// ------------------ Local helper functions ------------------ //
void PutPlayerDataToJSon(PlayerSlot* ply, Json::Value &root){
	// Set all the data //
	root["Type"] = ply->GetPlayerType();
	root["Identifier"] = ply->GetPlayerIdentifier();
	root["ControlType"] = ply->GetControlType();
	root["ControlID"] = ply->GetControlIdentifier();
	root["CControlID"] = ply->GetPlayerControllerID();
	root["Slot"] = ply->GetSlotNumber();
	root["Score"] = ply->GetScore();
	root["IsSplit"] = ply->GetSplitCount();
	root["Colour"] = ply->GetColourAsRML();
	root["IsActive"] = ply->IsSlotActive();
}
// ------------------ CustomJSInterface ------------------ //
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
		} else if(funcname == L"PongSlotInfo"){
			// Get info about a game slot //
			JS_ACCESSCHECKPTR(VIEW_SECURITYLEVEL_NORMAL, caller);

			// Get the slot number //
			auto numberstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_NONE);

			if(!numberstr){

				callback->Failure(1, "Did not find the team number");
				return true;
			}

			int team = Convert::WstringToInt(*numberstr);

			if(team < 0 || team >= 4){

				callback->Failure(1, "Invalid team number");
				return true;
			}

			// Serialize the info to json //
			PlayerSlot* ply = PongGame::Get()->GetPlayerSlot(team);


			// Get the data and serialize it //
			Json::Value root;
			{
				GUARD_LOCK_OTHER_OBJECT(ply);
				PutPlayerDataToJSon(ply, root);
				

				PlayerSlot* split = ply->GetSplit();
				if(split){
					// We need to put it's data //
					Json::Value childdata;

					GUARD_LOCK_OTHER_OBJECT_NAME(split, guard2);

					PutPlayerDataToJSon(split, childdata);

					// Add it //
					root["SplitSlot"] = childdata;
				}
			}

			// Send the json //
			Json::FastWriter writer;

			const string text = writer.write(root);

			callback->Success(text);
			return true;
		} else if(funcname == L"PongGameDBTable"){
			JS_ACCESSCHECKPTR(VIEW_SECURITYLEVEL_NORMAL, caller);

			// Try to get the table from the game database //
			const wstring table = *itr.GetStringInQuotes(QUOTETYPE_BOTH).get();

			string tabledata;
			
			if(!PongGame::Get()->GetGameDatabase()->WriteTableToJson(table, tabledata)){

				callback->Failure(404, "Table with that name not found");
				return true;
			}


			callback->Success(tabledata);
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
