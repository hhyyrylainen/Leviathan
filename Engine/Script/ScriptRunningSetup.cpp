#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPTRUNNINGSETUP
#include "ScriptRunningSetup.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ScriptRunningSetup::ScriptRunningSetup() : Parameters(), PrintErrors(true), FullDeclaration(false), 
	ErrorOnNonExistingFunction(true), RunType(SCRIPT_RUNTYPE_BREAKONERROR), ScriptExisted(false), Entryfunction("")
{

}

DLLEXPORT Leviathan::ScriptRunningSetup::~ScriptRunningSetup(){

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


