#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GLOBALCEFHANDLER
#include "GlobalCEFHandler.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "include/cef_app.h"
#include "GUI/GuiCEFApplication.h"
#include "include/cef_sandbox_win.h"


DLLEXPORT bool Leviathan::GlobalCEFHandler::CEFFirstCheckChildProcess(const wstring &commandline, int &returnvalue, 
	shared_ptr<CEFSandboxInfoKeeper> &keeper,
#ifdef _WIN32
	HINSTANCE hInstance 
#endif // _WIN32 
	)
{
	// Check for no graphics mode //
	if(commandline.find(L"--nogui") != wstring::npos){
		// No gui commandline specified //
		wcout << L"Not using CEF because --nogui is specified" << endl;
		return false;
	}
	// Run CEF startup //

	// create CEF3 objects //
	void* sandbox_info = NULL;

	keeper = shared_ptr<CEFSandboxInfoKeeper>(new CEFSandboxInfoKeeper());

#if CEF_ENABLE_SANDBOX
	sandbox_info = keeper->GetPtr();
#endif

	// Provide CEF with command-line arguments //
#ifdef _WIN32
	CefMainArgs main_args(hInstance);
#else
	CefMainArgs main_args(commandline);
#endif // _WIN32

	// Callback object //
	keeper->CEFApp = CefRefPtr<Gui::CefApplication>(new Gui::CefApplication());

	// Check are we a sub process //
	int exit_code = CefExecuteProcess(main_args, keeper->CEFApp.get(), sandbox_info);
	if(exit_code >= 0){
		// This was a sub process //
		returnvalue = exit_code;
		return true;
	}

	// Specify CEF global settings here.
	CefSettings settings;

#if !CEF_ENABLE_SANDBOX
	settings.no_sandbox = true;
#endif

	// Initialize CEF.
	CefInitialize(main_args, settings, keeper->CEFApp.get(), sandbox_info);

	CEFInitialized = true;

	AccessToThese = keeper.get();

	// Wasn't a sub process //
	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GlobalCEFHandler::CEFLastThingInProgram(){
	if(!CEFInitialized)
		return;
	// Close it down //
	CefShutdown();
	wcout << L"CEF shutdown called" << endl;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GlobalCEFHandler::DoCEFMessageLoopWork(){
	if(!CEFInitialized)
		return;
	CefDoMessageLoopWork();
}
// ------------------------------------ //
DLLEXPORT CEFSandboxInfoKeeper* Leviathan::GlobalCEFHandler::GetCEFObjects(){
	return AccessToThese;
}

CEFSandboxInfoKeeper* Leviathan::GlobalCEFHandler::AccessToThese = NULL;

bool Leviathan::GlobalCEFHandler::CEFInitialized = false;
// ------------------ CEFSandboxInfoKeeper ------------------ //
Leviathan::CEFSandboxInfoKeeper::CEFSandboxInfoKeeper() : SandBoxAccess(NULL){


#if CEF_ENABLE_SANDBOX
	// Manage the life span of the sandbox information object. This is necessary
	// for sandbox support on Windows. See cef_sandbox_win.h for complete details.
	ScopedInfo = shared_ptr<CefScopedSandboxInfo>(new CefScopedSandboxInfo());
	SandBoxAccess = ScopedInfo->sandbox_info();
#endif
}

DLLEXPORT Leviathan::CEFSandboxInfoKeeper::~CEFSandboxInfoKeeper(){

}
// ------------------------------------ //
void* Leviathan::CEFSandboxInfoKeeper::GetPtr(){
	return SandBoxAccess;
}
