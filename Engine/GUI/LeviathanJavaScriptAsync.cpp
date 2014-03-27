#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LEVIATHANJAVASCRIPTASYNC
#include "LeviathanJavaScriptAsync.h"
#endif
#include "GuiView.h"
#include "jsoncpp/json.h"
#include "Application/Application.h"
#include "GlobalCEFHandler.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
Leviathan::Gui::LeviathanJavaScriptAsync::LeviathanJavaScriptAsync(View* owner) : Owner(owner){
	GlobalCEFHandler::RegisterJSAsync(this);



	// We need to register all current ones //
	auto vec = GlobalCEFHandler::GetRegisteredCustomHandlers();

	ObjectLock guard(*this);

	for(auto iter = vec.begin(); iter != vec.end(); ++iter){

		RegisterNewCustom((*iter).get());
	}
}

DLLEXPORT Leviathan::Gui::LeviathanJavaScriptAsync::~LeviathanJavaScriptAsync(){
	ObjectLock guard(*this);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::LeviathanJavaScriptAsync::BeforeRelease(){
	GlobalCEFHandler::UnRegisterJSAsync(this);

	ObjectLock guard(*this);

	// Remove all //
	while(RegisteredCustomHandlers.size()){
		
		RegisteredCustomHandlers[0]->CancelAllMine(this);
		RegisteredCustomHandlers.erase(RegisteredCustomHandlers.begin());
	}
}
// ------------------------------------ //
#define JS_ACCESSCHECK(x) if(_VerifyJSAccess(x, callback)){return true;}

bool Leviathan::Gui::LeviathanJavaScriptAsync::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id, const CefString& request, bool persistent, CefRefPtr<Callback> callback){
	// First handle with default handlers //
	if(request == "LeviathanVersion"){
		// Security check //
		JS_ACCESSCHECK(VIEW_SECURITYLEVEL_MINIMAL);

		// Return the wanted value //
		callback->Success(LEVIATHAN_VERSIONS);

		return true;
	} else if(request == "Quit"){
		// Security check //
		JS_ACCESSCHECK(VIEW_SECURITYLEVEL_ACCESS_ALL);

		// Post quit message //
		Logger::Get()->Info(L"JS API Quit called from GUI View: "+Convert::ToWstring(Owner->ID));
		LeviathanApplication::GetApp()->StartRelease();

		// Return success indication //
		callback->Success("1");

		return true;
	} else {
		// These requests are multi-part ones //
		const wstring tmpstr(request);
		WstringIterator itr(tmpstr);
		
		// Get the first part of it //
		const wstring funcname = *itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS | UNNORMALCHARACTER_TYPE_WHITESPACE).get();

		// We can now compare the function name //


		if(funcname == L"Not"){
			// Security check //
			JS_ACCESSCHECK(VIEW_SECURITYLEVEL_NORMAL);

			// Get the event name //
			const wstring argcontent = *itr.GetStringInQuotes(QUOTETYPE_BOTH).get();


			// The window will now listen for it //
			callback->Failure(2, "lol");

			return true;
		}
	}

	ObjectLock guard(*this);

	// Try to use any custom handlers //
	for(size_t i = 0; i < RegisteredCustomHandlers.size(); i++){
		if(RegisteredCustomHandlers[i]->ProcessQuery(this, request, query_id, persistent, callback)){
			// It got handled //
			return true;
		}
	}
	
	// Not a valid query //
	return false;
}
// ------------------------------------ //
void Leviathan::Gui::LeviathanJavaScriptAsync::OnQueryCanceled(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id){
	// Remove from the main queue //


	// If not removed here, it must be a custom interface's query //

	// Tell all custom interfaces that this query is canceled //
	for(size_t i = 0; i < RegisteredCustomHandlers.size(); i++){
		RegisteredCustomHandlers[i]->CancelQuery(this, query_id);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::LeviathanJavaScriptAsync::RegisterNewCustom(JSAsyncCustom* newhandler){
	ObjectLock guard(*this);

	RegisteredCustomHandlers.push_back(newhandler);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::LeviathanJavaScriptAsync::UnregisterCustom(JSAsyncCustom* handler){
	// Cancel all matching ones //
	handler->CancelAllMine(this);

	ObjectLock guard(*this);

	for(size_t i = 0; i < RegisteredCustomHandlers.size(); i++){

		if(RegisteredCustomHandlers[i] == handler){
			RegisteredCustomHandlers.erase(RegisteredCustomHandlers.begin()+i);
			return;
		}
	}

	Logger::Get()->Warning(L"LeviathanJavaScriptAsync: UnregisterCustom: trying to remove nonexistent handler");
}
// ------------------ JSAsyncCustom ------------------ //
DLLEXPORT Leviathan::Gui::JSAsyncCustom::JSAsyncCustom(){
	// Register is done by the code that constructs this class //
}

DLLEXPORT Leviathan::Gui::JSAsyncCustom::~JSAsyncCustom(){

}
