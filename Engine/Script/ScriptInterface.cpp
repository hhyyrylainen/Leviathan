#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_INTERFACE
#include "ScriptInterface.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"

Leviathan::ScriptInterface::ScriptInterface() : ScriptRunner(NULL){
	StaticAccess = this;
}
Leviathan::ScriptInterface::~ScriptInterface(){
	StaticAccess = NULL;
}

ScriptInterface* Leviathan::ScriptInterface::StaticAccess = NULL;
ScriptInterface* Leviathan::ScriptInterface::Get(){ return StaticAccess; };
// ------------------------------------ //
bool Leviathan::ScriptInterface::Init(){
	// create script executor //
	ScriptRunner = new ScriptExecutor();
	if(!ScriptRunner->Init()){

		Logger::Get()->Error(L"ScriptInterface: Init: ScriptExecutor init failed",true);
		return false;
	}

	return true;
}
void Leviathan::ScriptInterface::Release(){
	// send event for scripts shutting down //
	// TODO: this

	// script runner needs to be released //
	SAFE_RELEASEDEL(ScriptRunner);
}
// ------------------------------------ //



