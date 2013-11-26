#pragma once

#include "angelscript.h"
#include "Events\Event.h"
#include "ScriptExecutor.h"
#include "Events/EventHandler.h"
#include "Utility/DataHandling/SimpleDataBase.h"

int WrapperForDataBlockToInt(VariableBlock* obj){

	return (int)obj->GetBlock();
}

GenericEvent* WrapperGenericEventFactory(string name){

	return new GenericEvent(Convert::StringToWstring(name), NamedVars());
}


ScriptSafeVariableBlock* SimpleDataBaseGetValueOnRowWhereSpecificValueProxy(SimpleDatabase* database, string tablename, string valuekeyname, ScriptSafeVariableBlock* wantedvalue, string wantedvaluekey){

	shared_ptr<VariableBlock> tmp = database->GetValueOnRow(Convert::StringToWstring(tablename), Convert::StringToWstring(valuekeyname), 
		VariableBlock(wantedvalue->GetBlockConst()->AllocateNewFromThis()), Convert::StringToWstring(wantedvaluekey));

	if(!tmp){
		// If not found return empty handle //
		return NULL;
	}

	// Release our reference //
	wantedvalue->Release();

	// Create result //
	return new ScriptSafeVariableBlock(tmp.get(), L"Result");
}

ScriptSafeVariableBlock* ScriptSafeVariableBlockFactoryString(string blockname, string valuestr){

	return new ScriptSafeVariableBlock(new StringBlock(valuestr), Convert::StringToWstring(blockname));
}


bool BindEngineCommonScriptIterface(asIScriptEngine* engine){

	// Bind NamedVars //

	// TODO: make this safe to be passed to the script //
	if(engine->RegisterObjectType("NamedVars", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("NamedVars", asBEHAVE_ADDREF, "void f()", asMETHOD(NamedVars, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("NamedVars", asBEHAVE_RELEASE, "void f()", asMETHOD(NamedVars, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	


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

	// bind generic event //
	if(engine->RegisterObjectType("GenericEvent", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GenericEvent", asBEHAVE_ADDREF, "void f()", asMETHOD(GenericEvent, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GenericEvent", asBEHAVE_RELEASE, "void f()", asMETHOD(GenericEvent, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Factory //
	if(engine->RegisterObjectBehaviour("GenericEvent", asBEHAVE_FACTORY, "GenericEvent@ f(string typename)", asFUNCTION(WrapperGenericEventFactory), asCALL_CDECL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Data get function //
	if(engine->RegisterObjectMethod("GenericEvent", "NamedVars@ GetNamedVars()", asMETHOD(GenericEvent, GetNamedVarsRefCounted), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Event handler which cannot be instantiated or copied around //
	if(engine->RegisterObjectType("EventHandler", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Global get function //
	if(engine->RegisterGlobalFunction("EventHandler& GetEventHandler()", asFUNCTION(EventHandler::Get), asCALL_CDECL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Script event firing //
	if(engine->RegisterObjectMethod("EventHandler", "void CallEvent(GenericEvent@ event)", asMETHOD(EventHandler, CallEventGenericProxy), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// bind datablock //
	if(engine->RegisterObjectType("ScriptSafeVariableBlock", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptSafeVariableBlock, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptSafeVariableBlock, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Some factories //
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY, "ScriptSafeVariableBlock@ f(string blockname, string value)", asFUNCTION(ScriptSafeVariableBlockFactoryString), asCALL_CDECL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	

	// conversion functions //
	if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "int ConvertAndReturnInt()", asMETHOD(ScriptSafeVariableBlock, ConvertAndReturnProxyInt), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "string ConvertAndReturnString()", asMETHOD(ScriptSafeVariableBlock, ConvertAndReturnProxyString), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Internal type conversions //
	if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "ScriptSafeVariableBlock@ ConvertToWstringBlock()", asMETHOD(ScriptSafeVariableBlock, CreateNewWstringProxy), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// type check //
	if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "bool IsValidType()", asMETHOD(ScriptSafeVariableBlock, IsValidType), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterObjectMethod("NamedVars", "ScriptSafeVariableBlock@ GetSingleValueByName(string name)", asMETHOD(NamedVars, GetScriptCompatibleValue), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	// Bind SimpleDataBase //
	if(engine->RegisterObjectType("SimpleDatabase", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Getting functions //
	if(engine->RegisterObjectMethod("SimpleDatabase", 
		"ScriptSafeVariableBlock@ GetValueOnRow(string table, string valuekeyname, ScriptSafeVariableBlock@ wantedvalue, string wantedvaluekey)", 
		asFUNCTION(SimpleDataBaseGetValueOnRowWhereSpecificValueProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	

	return true;
}

void RegisterEngineScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids){

	typeids.insert(make_pair(engine->GetTypeIdByDecl("Event"), L"Event"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("GenericEvent"), L"GenericEvent"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("ScriptSafeVariableBlock"), L"ScriptSafeVariableBlock"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("NamedVars"), L"NamedVars"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("SimpleDatabase"), L"SimpleDatabase"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("string"), L"string"));
}