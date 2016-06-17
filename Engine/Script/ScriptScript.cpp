// ------------------------------------ //
#include "ScriptScript.h"

#include "ScriptModule.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::ScriptScript::ScriptScript(const int &MID, weak_ptr<ScriptModule> wptr) :
    ScriptsModule(wptr), ModuleID(MID)
{

}

DLLEXPORT Leviathan::ScriptScript::ScriptScript(const ScriptScript &other){
	// copy over //
	ModuleID = other.ModuleID;
	ScriptsModule = other.ScriptsModule;
}

DLLEXPORT Leviathan::ScriptScript::ScriptScript(weak_ptr<ScriptModule> wptr) :
    ScriptsModule(wptr), ModuleID(wptr.lock()->GetID())
{

}

DLLEXPORT Leviathan::ScriptScript::~ScriptScript(){

}
// ------------------------------------ //


