#pragma once

#include "angelscript.h"
#include "Events\Event.h"
#include "ScriptExecutor.h"


int WrapperForDataBlockToInt(VariableBlock* obj){

	return (int)obj->GetBlock();
}


bool BindEngineCommonScriptIterface(asIScriptEngine* engine){

	// bind event type enum //
	if(engine->RegisterEnum("EVENT_TYPE") < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
#define REGISTEREVENTTYPE(x)	{if(engine->RegisterEnumValue("EVENT_TYPE", #x, x) < 0){ANGELSCRIPT_REGISTERFAIL;}}

	REGISTEREVENTTYPE(EVENT_TYPE_SHOW);
	REGISTEREVENTTYPE(EVENT_TYPE_HIDE);
	REGISTEREVENTTYPE(EVENT_TYPE_TICK);
	REGISTEREVENTTYPE(EVENT_TYPE_LISTENERVALUEUPDATED);
	

	// bind event //
	if(engine->RegisterObjectType("Event", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Event", asBEHAVE_ADDREF, "void f()", asMETHOD(Event, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Event", asBEHAVE_RELEASE, "void f()", asMETHOD(Event, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}


	// bind datablock //
	if(engine->RegisterObjectType("VariableBlock", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->SetDefaultNamespace("VariableBlock") < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterGlobalFunction("int GetValueAsInt(VariableBlock@ block)", asFUNCTION(WrapperForDataBlockToInt), asCALL_CDECL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// restore namespace //
	if(engine->SetDefaultNamespace("") < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}



	return true;
}

void RegisterEngineScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids){

	typeids.insert(make_pair(engine->GetTypeIdByDecl("Event"), L"Event"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("DataBlock"), L"DataBlock"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("DataStore"), L"DataStore"));
}