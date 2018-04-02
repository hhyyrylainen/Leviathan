// ------------------------------------ //
#include "ScriptNotifiers.h"

#include "Bindings/BindHelpers.h"

#include "ScriptExecutor.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

// ------------------ ScriptNotifier ------------------ //
DLLEXPORT Leviathan::ScriptNotifier::ScriptNotifier(asIScriptFunction* functiontocall) :
    CallbackFunction(functiontocall)
{
    // TODO: increase ref count? or does the caller do that
}

DLLEXPORT Leviathan::ScriptNotifier::~ScriptNotifier(){

    GUARD_LOCK();
    
	ReleaseChildHooks(guard);
    
	if(CallbackFunction)
		CallbackFunction->Release();
    
	CallbackFunction = NULL;
}
// ------------------------------------ //
void Leviathan::ScriptNotifier::OnNotified(Lock &ownlock, BaseNotifiableAll* child,
    Lock &childlock)
{
	ScriptRunningSetup params;
	params.ErrorOnNonExistingFunction = true;
    
	auto result = ScriptExecutor::Get()->RunScript<void>(CallbackFunction, nullptr, params);

    if(result.Result != SCRIPT_RUN_RESULT::Success)
        LOG_ERROR("ScriptNotifier: OnNotified: failed to call script callback");
}
// ------------------ ScriptNotifiable ------------------ //
DLLEXPORT Leviathan::ScriptNotifiable::ScriptNotifiable(asIScriptFunction* functiontocall) :
    CallbackFunction(functiontocall)
{
    // TODO: increase ref count? or does the caller do that
}

DLLEXPORT Leviathan::ScriptNotifiable::~ScriptNotifiable(){

    GUARD_LOCK();

	ReleaseParentHooks(guard);
    
	if(CallbackFunction)
		CallbackFunction->Release();
    
	CallbackFunction = NULL;
}
// ------------------------------------ //
void Leviathan::ScriptNotifiable::OnNotified(Lock &ownlock, BaseNotifierAll* parent,
    Lock &parentlock)
{
	ScriptRunningSetup params;
	params.ErrorOnNonExistingFunction = true;
    
	auto result = ScriptExecutor::Get()->RunScript<void>(CallbackFunction, nullptr, params);

    if(result.Result != SCRIPT_RUN_RESULT::Success)
        LOG_ERROR("ScriptNotifiable: OnNotified: failed to call script callback");
}
// ------------------------------------ //
ScriptNotifier* ScriptNotifierFactory(asIScriptFunction* cb){

	return new ScriptNotifier(cb);
}

ScriptNotifiable* ScriptNotifiableFactory(asIScriptFunction* cb){

	return new ScriptNotifiable(cb);
}

void NotifiableConnectProxy(BaseNotifiableAll* obj, ScriptNotifier* target){
	obj->ConnectToNotifier(target);

	// Release the reference given to us //
	target->Release();
}


void NotifierConnectProxy(BaseNotifierAll* obj, ScriptNotifiable* target){
	obj->ConnectToNotifiable(target);

	// Release the reference given to us //
	target->Release();
}


bool Leviathan::RegisterNotifiersWithAngelScript(asIScriptEngine* engine){

	if(engine->RegisterFuncdef("void NotifierCallback()") < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Register the base notifiers //


	if(engine->RegisterObjectType("BaseNotifiableAll", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectType("BaseNotifierAll", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}


	// ScriptNotifier //
    ANGELSCRIPT_REGISTER_REF_TYPE("ScriptNotifier", ScriptNotifier);
    
	if(engine->RegisterObjectBehaviour("ScriptNotifier", asBEHAVE_FACTORY, "ScriptNotifier@ f(NotifierCallback @cb)", asFUNCTION(ScriptNotifierFactory), asCALL_CDECL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}


	// ScriptNotifiable //
    ANGELSCRIPT_REGISTER_REF_TYPE("ScriptNotifiable", ScriptNotifiable);
	if(engine->RegisterObjectBehaviour("ScriptNotifiable", asBEHAVE_FACTORY, "ScriptNotifiable@ f(NotifierCallback @cb)", asFUNCTION(ScriptNotifiableFactory), asCALL_CDECL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}


	// Notifier connecting //
	if(engine->RegisterObjectMethod("BaseNotifiableAll", "void ConnectToNotifier(ScriptNotifier@ notifier)", asFUNCTION(NotifiableConnectProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseNotifierAll", "void ConnectToNotifiable(ScriptNotifiable@ notifiable)", asFUNCTION(NotifierConnectProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	



	return true;
}


