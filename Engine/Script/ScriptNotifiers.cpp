#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPTNOTIFIERS
#include "ScriptNotifiers.h"
#endif
#include "ScriptExecutor.h"
using namespace Leviathan;
// ------------------------------------ //

// ------------------ ScriptNotifier ------------------ //
DLLEXPORT Leviathan::ScriptNotifier::ScriptNotifier(asIScriptFunction* functiontocall) : CallbackFunction(functiontocall){

}

DLLEXPORT Leviathan::ScriptNotifier::~ScriptNotifier(){
	ReleaseChildHooks();
	GUARD_LOCK_THIS_OBJECT_CAST(BaseNotifierAll);
	if(CallbackFunction)
		CallbackFunction->Release();
	CallbackFunction = NULL;
}
// ------------------------------------ //
void Leviathan::ScriptNotifier::OnNotified(){

	ScriptRunningSetup params;
	params.ErrorOnNonExistingFunction = true;
	// No parameters used //


	ScriptExecutor::Get()->RunFunctionSetUp(CallbackFunction, &params);
}
// ------------------ ScriptNotifiable ------------------ //
DLLEXPORT Leviathan::ScriptNotifiable::ScriptNotifiable(asIScriptFunction* functiontocall) : CallbackFunction(functiontocall){

}

DLLEXPORT Leviathan::ScriptNotifiable::~ScriptNotifiable(){
	ReleaseParentHooks();
	GUARD_LOCK_THIS_OBJECT_CAST(BaseNotifiableAll);
	if(CallbackFunction)
		CallbackFunction->Release();
	CallbackFunction = NULL;
}
// ------------------------------------ //
void Leviathan::ScriptNotifiable::OnNotified(){

	ScriptRunningSetup params;
	params.ErrorOnNonExistingFunction = true;
	// No parameters used //


	ScriptExecutor::Get()->RunFunctionSetUp(CallbackFunction, &params);
}
// ------------------------------------ //
ScriptNotifier* ScriptNotifierFactory(asIScriptFunction* cb){

	return new ScriptNotifier(cb);
}

ScriptNotifiable* ScriptNotifiableFactory(asIScriptFunction* cb){

	return new ScriptNotifiable(cb);
}

template<class From, class To>
To* DoReferenceCastDynamic(From* ptr){
	// If already invalid just return it //
	if(!ptr)
		return NULL;

	To* newptr = dynamic_cast<To*>(ptr);
	if(newptr){
		// Add reference so script doesn't screw up with the handles //
		newptr->AddRef();
	}

	// Return the ptr (which might be invalid) //
	return newptr;
}

template<class From, class To>
To* DoReferenceCastStaticNoAdd(From* ptr){
	// If already invalid just return it //
	if(!ptr)
		return NULL;

	To* newptr = static_cast<To*>(ptr);

	// Return the ptr (which might be invalid) //
	return newptr;
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
	if(engine->RegisterObjectType("ScriptNotifier", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptNotifier", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptNotifier, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptNotifier", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptNotifier, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptNotifier", asBEHAVE_FACTORY, "ScriptNotifier@ f(NotifierCallback @cb)", asFUNCTION(ScriptNotifierFactory), asCALL_CDECL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}


	// ScriptNotifiable //
	if(engine->RegisterObjectType("ScriptNotifiable", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptNotifiable", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptNotifiable, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptNotifiable", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptNotifiable, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
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

void Leviathan::RegisterNotifierTypesWithAngelScript(asIScriptEngine* engine, std::map<int, wstring> &typeids){
	typeids.insert(make_pair(engine->GetTypeIdByDecl("ScriptNotifiable"), L"ScriptNotifiable"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("ScriptNotifier"), L"ScriptNotifier"));
	//typeids.insert(make_pair(engine->GetTypeIdByDecl("BaseNotifiableAll"), L"BaseNotifiableAll"));
	//typeids.insert(make_pair(engine->GetTypeIdByDecl("BaseNotifierAll"), L"BaseNotifierAll"));
}


