#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_VARIABLEHOLDER
#include "ScriptVariableHolder.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ScriptVariableHolder::ScriptVariableHolder(){

}
ScriptVariableHolder::~ScriptVariableHolder(){
	while(Vars.size() != 0){
		SAFE_DELETE(Vars[0]);
		Vars.erase(Vars.begin());
	}
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //