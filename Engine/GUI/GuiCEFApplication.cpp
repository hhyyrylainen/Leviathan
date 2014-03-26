#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUICEFAPPLICATION
#include "GuiCEFApplication.h"
#endif
#include "Common/StringOperations.h"
#include "Generated/LeviathanV8CoreExt.h"
#include "FileSystem.h"
using namespace Leviathan;
// ------------------------------------ //
Leviathan::Gui::CefApplication::CefApplication() : RendererRouter(NULL){

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


	// Register it //
	// TODO: add native handler
	const string levcoreext(LeviathanCoreV8ExtensionStr);


	CefRegisterExtension("Leviathan/API", levcoreext, NULL);

	// Load custom extensions //
	for(size_t i = 0; i < ExtensionContents.size(); i++){
		// Load it //
		CefRegisterExtension(ExtensionContents[i]->FileName, ExtensionContents[i]->Contents, NULL);
	}

	// Clear the memory //
	ExtensionContents.clear();
}

void Leviathan::Gui::CefApplication::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context){
	// JavaScript context created, add V8 bindings here...

	RendererRouter->OnContextCreated(browser, frame, context);
}

void Leviathan::Gui::CefApplication::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context){
	// JavaScript context released, release V8 references here...

	RendererRouter->OnContextReleased(browser, frame, context);
}
// ------------------------------------ //
void Leviathan::Gui::CefApplication::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info){
	// Send startup information to a new render process...


	// Send custom extensions to it //
	extra_info->SetInt(0, (int)CustomExtensionFiles.size());

	for(int i = 0; i < (int)CustomExtensionFiles.size(); i++){
		if(i != 0){

			Logger::Get()->Warning(L"CefApplication: loading multiple extensions, performance might be worse than with one large extension");
		}
		// Report this //
		Logger::Get()->Info(L"CefApplication: loading extension file "+Convert::StringToWstring(CustomExtensionFiles[i]));

		// Load it's contents and pass it //
		extra_info->SetString(1+i*2, "Leviathan/"+StringOperations::RemoveExtensionString(CustomExtensionFiles[i]));
		// Load the file //
		string filecontents;
		try{
			FileSystem::ReadFileEntirely(CustomExtensionFiles[i], filecontents);
		} catch(...){
			filecontents = "";
		}
		extra_info->SetString(1+i*2+1, filecontents);
	}

}

void Leviathan::Gui::CefApplication::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info){
	// The render process main thread has been initialized...
	// Receive startup information in the new render process...

	// Create messaging functionality //
	CefMessageRouterConfig config;
	config.js_query_function = "cefQuery";
	config.js_cancel_function = "cefQueryCancel";

	RendererRouter = CefMessageRouterRendererSide::Create(config);

	// Copy the extensions away //
	int size = extra_info->GetInt(0);

	for(int i = 0; i < size; i++){

		// Copy it to our vector //
		ExtensionContents.push_back(unique_ptr<CustomExtensionFileData>(new CustomExtensionFileData(extra_info->GetString(1+i*2), 
			extra_info->GetString(1+i*2+1))));
	}
}

bool Leviathan::Gui::CefApplication::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message){
	// Handle IPC messages from the browser process...

	if(RendererRouter->OnProcessMessageReceived(browser, source_process, message))
		return true;

	// Not handled //
	return false;
}

DLLEXPORT void Leviathan::Gui::CefApplication::RegisterCustomExtensionFile(const string &file){
	CustomExtensionFiles.push_back(file);
}

