#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_LIST
#include "ScriptList.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ScriptList::ScriptList(){
	Variables = new NamedVars();
}
ScriptList::ScriptList(wstring name){
	Name = name;
	Variables = new NamedVars();
}
ScriptList::~ScriptList(){
	SAFE_DELETE(Variables);
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //