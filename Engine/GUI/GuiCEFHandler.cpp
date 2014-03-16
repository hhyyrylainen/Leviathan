#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUICEFHANDLER
#include "GuiCEFHandler.h"
#endif
#include "Common\Window.h"
using namespace Leviathan;
// ------------------------------------ //
Leviathan::Gui::CefHandler::CefHandler() : InputHandlingWindow(NULL){

}

DLLEXPORT Leviathan::Gui::CefHandler::~CefHandler(){

}

CefRefPtr<CefRequestHandler> Leviathan::Gui::CefHandler::GetRequestHandler(){
	return this;
}

CefRefPtr<CefLoadHandler> Leviathan::Gui::CefHandler::GetLoadHandler(){
	return this;
}

CefRefPtr<CefLifeSpanHandler> Leviathan::Gui::CefHandler::GetLifeSpanHandler(){
	return this;
}

CefRefPtr<CefKeyboardHandler> Leviathan::Gui::CefHandler::GetKeyboardHandler(){
	return this;
}

CefRefPtr<CefGeolocationHandler> Leviathan::Gui::CefHandler::GetGeolocationHandler(){
	return this;
}

CefRefPtr<CefDragHandler> Leviathan::Gui::CefHandler::GetDragHandler(){
	return this;
}

CefRefPtr<CefDownloadHandler> Leviathan::Gui::CefHandler::GetDownloadHandler(){
	return this;
}

CefRefPtr<CefDisplayHandler> Leviathan::Gui::CefHandler::GetDisplayHandler(){
	return this;
}

CefRefPtr<CefContextMenuHandler> Leviathan::Gui::CefHandler::GetContextMenuHandler(){
	return this;
}

void Leviathan::Gui::CefHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution){
	// Handle execution of external protocols...
}

void Leviathan::Gui::CefHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status){
	// A render process has crashed...
}

void Leviathan::Gui::CefHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl){
	// A frame has failed to load content...
}

void Leviathan::Gui::CefHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode){
	// A frame has finished loading content...
}

void Leviathan::Gui::CefHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame){
	// A frame has started loading content...
}

void Leviathan::Gui::CefHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser){
	// Browser window is closed, perform cleanup...
}

void Leviathan::Gui::CefHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser){
	// Browser window created successfully...
}

bool Leviathan::Gui::CefHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access){
	// Allow or block popup windows, customize popup window creation...
	return false;
}

void Leviathan::Gui::CefHandler::OnRequestGeolocationPermission(CefRefPtr<CefBrowser> browser, const CefString& requesting_url, int request_id, CefRefPtr<CefGeolocationCallback> callback){
	// Allow or deny geolocation API access...
}

void Leviathan::Gui::CefHandler::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback){
	// Update the download status...
}

void Leviathan::Gui::CefHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback){
	// Specify a file path or cancel the download...
}

bool Leviathan::Gui::CefHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line){
	// Log a console message...
	// Allow passing to default console //
	return false;
}

void Leviathan::Gui::CefHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title){
	// Update the browser window title...
}

void Leviathan::Gui::CefHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url){
	// Update the URL in the address bar...
}

void Leviathan::Gui::CefHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward){
	// Update UI for browser state...
}
// ------------------------------------ //
bool Leviathan::Gui::CefHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event){
	// Notify the window about this //
	if(InputHandlingWindow)
		InputHandlingWindow->ReportKeyEventAsUsed();
	return false;
}


DLLEXPORT void Leviathan::Gui::CefHandler::SetCurrentInputHandlingWindow(Window* wind){
	InputHandlingWindow = wind;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //