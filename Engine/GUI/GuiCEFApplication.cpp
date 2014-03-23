#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUICEFAPPLICATION
#include "GuiCEFApplication.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Leviathan::Gui::CefApplication::CefApplication(){

}

DLLEXPORT Leviathan::Gui::CefApplication::~CefApplication(){

}
// ------------------------------------ //
bool Leviathan::Gui::CefApplication::OnBeforeNavigation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request,
	NavigationType navigation_type, bool is_redirect)
{
	// Allow it //
	return false;
}
// ------------------------------------ //
void Leviathan::Gui::CefApplication::OnContextInitialized(){
	// Let's register this thread with everything //



}


// ------------------------------------ //
void Leviathan::Gui::CefApplication::OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line){
	// Check if we want to change something
}

void Leviathan::Gui::CefApplication::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar){

}

void Leviathan::Gui::CefApplication::OnBrowserCreated(CefRefPtr<CefBrowser> browser){
	// Browser created in this render process...
}

void Leviathan::Gui::CefApplication::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser){
	// Browser destroyed in this render process...
}
// ------------------------------------ //
void Leviathan::Gui::CefApplication::OnWebKitInitialized(){
	// WebKit has been initialized, register V8 extensions...
}

void Leviathan::Gui::CefApplication::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context){
	// JavaScript context created, add V8 bindings here...
}

void Leviathan::Gui::CefApplication::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context){
	// JavaScript context released, release V8 references here...
}
// ------------------------------------ //
void Leviathan::Gui::CefApplication::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info){
	// Send startup information to a new render process...
}

void Leviathan::Gui::CefApplication::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info){
	// The render process main thread has been initialized...
	// Receive startup information in the new render process...
}

