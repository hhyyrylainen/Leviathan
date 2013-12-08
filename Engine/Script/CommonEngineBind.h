#pragma once

#include "angelscript.h"
#include "Events/Event.h"
#include "ScriptExecutor.h"
#include "Events/EventHandler.h"
#include "Utility/DataHandling/SimpleDataBase.h"
#include "Addons/GameModule.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/Brush.h"
#include "Entities/Objects/TrackEntityController.h"

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

void Float3ConstructorProxy(void* memory){
	new(memory) Float3();
}

void Float3ConstructorProxyAll(void* memory, float x, float y, float z){
	new(memory) Float3(x, y, z);
}

void Float3ConstructorProxySingle(void* memory, float all){
	new(memory) Float3(all);
}

void Float3ConstructorProxyCopy(void* memory, const Float3* other){
	new(memory) Float3(*other);
}

void Float3DestructorProxy(void* memory){
	reinterpret_cast<Float3*>(memory)->~Float3();
}

NewtonBody* GetPropPhysicalBodyProxy(Entity::Prop* obj){
	return obj->GetPhysicsBody();
}

NewtonBody* GetBrushPhysicalBodyProxy(Entity::Brush* obj){
	return obj->GetPhysicsBody();
}

Float3 GetPropPosition(Entity::Prop* obj){
	return obj->GetPos();
}

Float3 GetPropVelocity(Entity::Prop* obj){
	return obj->GetBodyVelocity();
}


NewtonBody* BaseObjectCustomMessageGetNewtonBody(BaseObject* obj){
	// Use the SendCustomMessage function to request this data //
	ObjectDataRequest request(ENTITYDATA_REQUESTTYPE_NEWTONBODY);

	obj->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_DATAREQUEST, &request);

	return reinterpret_cast<NewtonBody*>(request.RequestResult);
}

BaseObject* CastPropToBaseObjectProxy(Entity::Prop* object){
	return dynamic_cast<BaseObject*>(object);
}

BaseObject* CastBrushToBaseObjectProxy(Entity::Brush* object){
	return dynamic_cast<BaseObject*>(object);
}

Float3* Float3AssignOtherProxy(Float3* thisobj, const Float3* otherobj){

	*thisobj = *otherobj;
	return thisobj;
}

Float3 Float3AddOtherProxy(Float3* thisobj, const Float3* otherobj){

	return *thisobj + *otherobj;
}

Float3 Float3SubOtherProxy(Float3* thisobj, const Float3* otherobj){

	return *thisobj - *otherobj;
}

Float3 Float3MulFloatProxy(Float3* thisobj, float val){

	return (*thisobj)*val;
}

float Float3HAddAbsProxy(Float3* obj){
	return obj->HAddAbs();
}

Float3 Float3NormalizeProxy(Float3* obj){
	return obj->Normalize();
}

void BaseObjectAddRefProxy(BaseObject* obj){
	obj->AddRef();
}

void BaseObjectReleaseProxy(BaseObject* obj){
	obj->Release();
}

void BrushAddRefProxy(Entity::Brush* obj){
	obj->AddRef();
}

void BrushReleaseProxy(Entity::Brush* obj){
	obj->Release();
}

void PropAddRefProxy(Entity::Prop* obj){
	obj->AddRef();
}

void PropReleaseProxy(Entity::Prop* obj){
	obj->Release();
}

void TrackEntityControllerAddRefProxy(Entity::TrackEntityController* obj){

	obj->AddRef();
}

void TrackEntityControllerReleaseProxy(Entity::TrackEntityController* obj){

	obj->Release();
}


Float3 TrackEntityControllerGetCurrentNodePositionProxy(Entity::TrackEntityController* obj){

	return obj->GetCurrentNodePosition();
}

Float3 TrackEntityControllerGetNextNodePositionProxy(Entity::TrackEntityController* obj){

	return obj->GetNextNodePosition();
}

void TrackEntityControllerSetProgressTowardsNextNodeProxy(Entity::TrackEntityController* obj, float val){
	obj->SetProgressTowardsNextNode(val);
}


bool BindEngineCommonScriptIterface(asIScriptEngine* engine){

	// Register common float types //


	// Float3 bind //
	if(engine->RegisterObjectType("Float3", sizeof(Float3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_APP_CLASS_ALLFLOATS) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Float3ConstructorProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f(float value)", asFUNCTION(Float3ConstructorProxySingle), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z)", asFUNCTION(Float3ConstructorProxyAll), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f(const Float3 &in other)", asFUNCTION(Float3ConstructorProxyCopy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Float3DestructorProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Operators //
	if(engine->RegisterObjectMethod("Float3", "Float3& opAssign(const Float3 &in other)", asFUNCTION(Float3AssignOtherProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "Float3 opAdd(const Float3 &in other)", asFUNCTION(Float3AddOtherProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "Float3 opSub(const Float3 &in other)", asFUNCTION(Float3SubOtherProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "Float3 opMul(float multiply)", asFUNCTION(Float3MulFloatProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "Float3 Normalize()", asFUNCTION(Float3NormalizeProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "float GetX()", asMETHOD(Float3, GetX), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "float GetY()", asMETHOD(Float3, GetY), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "float GetZ()", asMETHOD(Float3, GetZ), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "void SetX(float &in x)", asMETHOD(Float3, SetX), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "void SetY(float &in y)", asMETHOD(Float3, SetY), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "void SetZ(float &in z)", asMETHOD(Float3, SetZ), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "float HAddAbs()", asFUNCTION(Float3HAddAbsProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

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

	// Bind GameModule //
	if(engine->RegisterObjectType("GameModule", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GameModule", asBEHAVE_ADDREF, "void f()", asMETHOD(GameModule, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GameModule", asBEHAVE_RELEASE, "void f()", asMETHOD(GameModule, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Bind simple name get function //
	if(engine->RegisterObjectMethod("GameModule", "string GetDescription(bool full = false)", asMETHOD(GameModule, GetDescriptionProxy), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Bind GameWorld //
	if(engine->RegisterObjectType("GameWorld", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Bind the ray cast function //
	// Result class for ray cast //
	if(engine->RegisterObjectType("RayCastHitEntity", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RayCastHitEntity", asBEHAVE_ADDREF, "void f()", asMETHOD(RayCastHitEntity, AddRefProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RayCastHitEntity", asBEHAVE_RELEASE, "void f()", asMETHOD(RayCastHitEntity, ReleaseProxy), asCALL_THISCALL) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("RayCastHitEntity", "Float3 GetPosition()", asMETHOD(RayCastHitEntity, GetPosition), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterObjectMethod("GameWorld", "RayCastHitEntity@ CastRayGetFirstHit(Float3 start, Float3 end)", asMETHOD(GameWorld, CastRayGetFirstHitProxy), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Bind the NewtonBody as non counted handle and don't register any methods since it will only be used to compare the pointer //
	if(engine->RegisterObjectType("NewtonBody", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Compare function //
	if(engine->RegisterObjectMethod("RayCastHitEntity", "bool DoesBodyMatchThisHit(NewtonBody@ body)", asMETHOD(RayCastHitEntity, DoesBodyMatchThisHit), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	// ------------------ Game entities ------------------ //
	if(engine->RegisterObjectType("BaseObject", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseObject", asBEHAVE_ADDREF, "void f()", asFUNCTION(BaseObjectAddRefProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseObject", asBEHAVE_RELEASE, "void f()", asFUNCTION(BaseObjectReleaseProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseObject", "NewtonBody@ CustomMessageGetNewtonBody()", asFUNCTION(BaseObjectCustomMessageGetNewtonBody), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterObjectType("Prop", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Prop", asBEHAVE_ADDREF, "void f()", asFUNCTION(PropAddRefProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Prop", asBEHAVE_RELEASE, "void f()", asFUNCTION(PropReleaseProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Prop", "NewtonBody@ GetPhysicalBody()", asFUNCTION(GetPropPhysicalBodyProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Prop", "BaseObject& GetAsBaseObject()", asFUNCTION(CastPropToBaseObjectProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Prop", "Float3 GetPosition()", asFUNCTION(GetPropPosition), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Prop", "Float3 GetVelocity()", asFUNCTION(GetPropVelocity), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectType("Brush", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Brush", asBEHAVE_ADDREF, "void f()", asFUNCTION(BrushAddRefProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Brush", asBEHAVE_RELEASE, "void f()", asFUNCTION(BrushReleaseProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Brush", "NewtonBody@ GetPhysicalBody()", asFUNCTION(GetBrushPhysicalBodyProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Brush", "BaseObject& GetAsBaseObject()", asFUNCTION(CastBrushToBaseObjectProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterObjectType("TrackEntityController", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("TrackEntityController", asBEHAVE_ADDREF, "void f()", asFUNCTION(TrackEntityControllerAddRefProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("TrackEntityController", asBEHAVE_RELEASE, "void f()", asFUNCTION(TrackEntityControllerReleaseProxy), asCALL_CDECL_OBJFIRST) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("TrackEntityController", "Float3 GetCurrentNodePosition()", asFUNCTION(TrackEntityControllerGetCurrentNodePositionProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("TrackEntityController", "Float3 GetNextNodePosition()", asFUNCTION(TrackEntityControllerGetNextNodePositionProxy), asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("TrackEntityController", "void SetProgressTowardsNextNode(float progress)", asFUNCTION(TrackEntityControllerSetProgressTowardsNextNodeProxy), asCALL_CDECL_OBJFIRST) < 0)
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
	typeids.insert(make_pair(engine->GetTypeIdByDecl("Float3"), L"Float3"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("GameModule"), L"GameModule"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("GameWorld"), L"GameWorld"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("RayCastHitEntity"), L"RayCastHitEntity"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("Prop"), L"Prop"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("Brush"), L"Brush"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("BaseObject"), L"BaseObject"));
}