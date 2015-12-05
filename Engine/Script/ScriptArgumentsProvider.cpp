#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPTARGUMENTSPROVIDER
#include "ScriptArgumentsProvider.h"
#endif
#include "ScriptModule.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT void Leviathan::ScriptArgumentsProvider::_LeaveBondBridge(){
	_ArgumentBridge->LeaveProvider();
	_ArgumentBridge.reset();
}

DLLEXPORT void Leviathan::ScriptArgumentsProvider::_BondWithModule(ScriptModule* module){
	_ArgumentBridge = std::shared_ptr<ScriptArgumentsProviderBridge>(new ScriptArgumentsProviderBridge());

	_ArgumentBridge->SetProvider(this);
	_ArgumentBridge->SetModule(module);

	module->OnAddedToBridge(_ArgumentBridge);
}
