#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_SCRIPT
#include "ScriptScript.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ScriptScript::ScriptScript(const int &MID, weak_ptr<ScriptModule> wptr) : ModuleID(MID), ScriptsModule(wptr){

}

DLLEXPORT Leviathan::ScriptScript::ScriptScript(const ScriptScript &other){
	// copy over //
	ModuleID = other.ModuleID;
	ScriptsModule = other.ScriptsModule;
}

DLLEXPORT Leviathan::ScriptScript::~ScriptScript(){

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


