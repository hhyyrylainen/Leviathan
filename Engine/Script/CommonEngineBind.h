#pragma once

#include "angelscript.h"
#include "Events/Event.h"
#include "ScriptExecutor.h"
#include "Events/EventHandler.h"
#include "Utility/DataHandling/SimpleDatabase.h"
#include "Addons/GameModule.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/Brush.h"
#include "Entities/Objects/TrackEntityController.h"
#include "add_on/autowrapper/aswrappedcall.h"
#include "Interface/ScriptEventListener.h"
#include "Networking/AINetworkCache.h"

#include "Engine.h"

using namespace Leviathan::Script;

GenericEvent* WrapperGenericEventFactory(const string &name){

	return new GenericEvent(Convert::StringToWstring(name), NamedVars());
}

Event* WrapperEventFactory(EVENT_TYPE type){

    try{
        return new Event(type, NULL);
        
    } catch(const ExceptionInvalidArgument &e){

        Logger::Get()->Error("Failed to construct Event for script, exception: ");
        e.PrintToLog();

        return NULL;
    }
}

ScriptSafeVariableBlock* ScriptSafeVariableBlockFactoryString(const string &blockname, const string &valuestr){

	return new ScriptSafeVariableBlock(new StringBlock(valuestr), Convert::StringToWstring(blockname));
}

template<typename TType>
ScriptSafeVariableBlock* ScriptSafeVariableBlockFactoryGeneric(const string &blockname, TType value){

	return new ScriptSafeVariableBlock(new DataBlock<TType>(value), Convert::StringToWstring(blockname));
}


// ------------------ Prop proxies ------------------ //
Float3 PropGetPosVal(Entity::Prop* obj){

    return obj->GetPos();
}
// ------------------ Float3 proxies ------------------ //
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

// ------------------ BaseObject proxies ------------------ //
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

string GetLeviathanVersionProxy(){

	return LEVIATHAN_VERSION_ANSIS;
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
To* DoReferenceCastStatic(From* ptr){
	// If already invalid just return it //
	if(!ptr)
		return NULL;

	To* newptr = static_cast<To*>(ptr);
	if(newptr){
		// Add reference so script doesn't screw up with the handles //
		newptr->AddRef();
	}

	// Return the ptr (which might be invalid) //
	return newptr;
}


//! \todo Create a wrapper around NewtonBody which has reference counting
bool BindEngineCommonScriptIterface(asIScriptEngine* engine){

	// Register common float types //


	// Float3 bind //
	if(engine->RegisterObjectType("Float3", sizeof(Float3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_APP_CLASS_ALLFLOATS) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f()", WRAP_FN(Float3ConstructorProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f(float value)", WRAP_FN(Float3ConstructorProxySingle), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z)", WRAP_FN(Float3ConstructorProxyAll), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f(const Float3 &in other)", WRAP_FN(Float3ConstructorProxyCopy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_DESTRUCT, "void f()", WRAP_FN(Float3DestructorProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Operators //
	if(engine->RegisterObjectMethod("Float3", "Float3& opAssign(const Float3 &in other)", WRAP_FN(Float3AssignOtherProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "Float3 opAdd(const Float3 &in other)", WRAP_FN(Float3AddOtherProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "Float3 opSub(const Float3 &in other)", WRAP_FN(Float3SubOtherProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "Float3 opMul(float multiply)", WRAP_FN(Float3MulFloatProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "Float3 Normalize()", WRAP_FN(Float3NormalizeProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "float GetX()", WRAP_MFN(Float3, GetX), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "float GetY()", WRAP_MFN(Float3, GetY), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "float GetZ()", WRAP_MFN(Float3, GetZ), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "void SetX(float &in x)", WRAP_MFN(Float3, SetX), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "void SetY(float &in y)", WRAP_MFN(Float3, SetY), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "void SetZ(float &in z)", WRAP_MFN(Float3, SetZ), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Float3", "float HAddAbs()", WRAP_MFN(Float3, HAddAbs), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Bind NamedVars //

	// \todo make this safe to be passed to the script //
	if(engine->RegisterObjectType("NamedVars", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("NamedVars", asBEHAVE_ADDREF, "void f()", WRAP_MFN(NamedVars, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("NamedVars", asBEHAVE_RELEASE, "void f()", WRAP_MFN(NamedVars, ReleaseProxy), asCALL_GENERIC) < 0){
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
	if(engine->RegisterObjectBehaviour("Event", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Event, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Event", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Event, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectBehaviour("Event", asBEHAVE_FACTORY, "Event@ f(EVENT_TYPE type)",
            asFUNCTION(WrapperEventFactory), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}

	// bind generic event //
	if(engine->RegisterObjectType("GenericEvent", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
    
	if(engine->RegisterObjectBehaviour("GenericEvent", asBEHAVE_ADDREF, "void f()", asMETHOD(GenericEvent, AddRefProxy),
            asCALL_THISCALL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}
    
	if(engine->RegisterObjectBehaviour("GenericEvent", asBEHAVE_RELEASE, "void f()",
            asMETHOD(GenericEvent, ReleaseProxy), asCALL_THISCALL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}
    
	// Factory //
	if(engine->RegisterObjectBehaviour("GenericEvent", asBEHAVE_FACTORY, "GenericEvent@ f(const string &in typename)",
            asFUNCTION(WrapperGenericEventFactory), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}
    
	// Data get function //
	if(engine->RegisterObjectMethod("GenericEvent", "NamedVars@ GetNamedVars()", WRAP_MFN(GenericEvent, GetNamedVarsRefCounted), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Event handler which cannot be instantiated or copied around //
	if(engine->RegisterObjectType("EventHandler", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Global get function //
	if(engine->RegisterGlobalFunction("EventHandler& GetEventHandler()", WRAP_FN(EventHandler::Get), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Script event firing //
	if(engine->RegisterObjectMethod("EventHandler", "void CallEvent(GenericEvent@ event)", WRAP_MFN(EventHandler, CallEventGenericProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

    // Bind EventListener //
	if(engine->RegisterObjectType("EventListener", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectBehaviour("EventListener", asBEHAVE_ADDREF, "void f()",
            asMETHOD(EventListener, AddRefProxy), asCALL_THISCALL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}
    
	if(engine->RegisterObjectBehaviour("EventListener", asBEHAVE_RELEASE, "void f()",
            asMETHOD(EventListener, ReleaseProxy), asCALL_THISCALL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterFuncdef("int OnEventCallback(Event@ event)") < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
    
    if(engine->RegisterFuncdef("int OnGenericEventCallback(GenericEvent@ event)") < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectBehaviour("EventListener", asBEHAVE_FACTORY,
            "EventListener@ f(OnEventCallback@ onevent, OnGenericEventCallback@ ongeneric)",
            asFUNCTION(EventListenerFactory), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectMethod("EventListener", "bool RegisterForEvent(EVENT_TYPE type)",
            asMETHOD(EventListener, RegisterForEventType), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectMethod("EventListener", "bool RegisterForEvent(const string &in name)",
            asMETHOD(EventListener, RegisterForEventGeneric), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

    
    

	// bind datablock //
	if(engine->RegisterObjectType("ScriptSafeVariableBlock", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_ADDREF, "void f()",
            WRAP_MFN(ScriptSafeVariableBlock, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_RELEASE, "void f()",
            WRAP_MFN(ScriptSafeVariableBlock, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Some factories //
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
            "ScriptSafeVariableBlock@ f(const string &in blockname, const string &in value)", 
		asFUNCTION(ScriptSafeVariableBlockFactoryString), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
            "ScriptSafeVariableBlock@ f(const string &in blockname, float value)", 
            asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<float>), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}
    
    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
            "ScriptSafeVariableBlock@ f(const string &in blockname, int value)", 
            asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<int>), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
            "ScriptSafeVariableBlock@ f(const string &in blockname, double value)", 
            asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<double>), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
            "ScriptSafeVariableBlock@ f(const string &in blockname, int8 value)", 
            asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<char>), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
            "ScriptSafeVariableBlock@ f(const string &in blockname, bool value)", 
            asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<bool>), asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}
    
    
	// Internal type conversions //
	if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "ScriptSafeVariableBlock@ ConvertToWstringBlock()",
            WRAP_MFN(ScriptSafeVariableBlock, CreateNewWstringProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Implicit casts for normal types //
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_VALUE_CAST, "string f() const", 
		WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<string>), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_VALUE_CAST, "int f() const", 
		WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<int>), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_VALUE_CAST, "int8 f() const", 
		WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<char>), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_VALUE_CAST, "float f() const", 
		WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<float>), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_VALUE_CAST, "double f() const", 
		WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<double>), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	//if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_VALUE_CAST, "bool f() const", 
	//	WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<bool>), asCALL_GENERIC) < 0)
	//{
	//	ANGELSCRIPT_REGISTERFAIL;
	//}

	// type check //
	if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "bool IsValidType()",
            asMETHOD(ScriptSafeVariableBlock, IsValidType), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterObjectMethod("NamedVars", "ScriptSafeVariableBlock@ GetSingleValueByName(const string &in name)",
            asMETHOD(NamedVars, GetScriptCompatibleValue), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("NamedVars", "bool AddValue(ScriptSafeVariableBlock@ value)",
            asMETHOD(NamedVars, AddScriptCompatibleValue), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	// Bind SimpleDataBase //
	if(engine->RegisterObjectType("SimpleDatabase", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Bind GameModule //
	if(engine->RegisterObjectType("GameModule", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GameModule", asBEHAVE_ADDREF, "void f()", WRAP_MFN(GameModule, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("GameModule", asBEHAVE_RELEASE, "void f()", WRAP_MFN(GameModule, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	// Bind simple name get function //
	if(engine->RegisterObjectMethod("GameModule", "string GetDescription(bool full = false)", WRAP_MFN(GameModule, GetDescriptionProxy), asCALL_GENERIC) < 0)
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
	if(engine->RegisterObjectBehaviour("RayCastHitEntity", asBEHAVE_ADDREF, "void f()", WRAP_MFN(RayCastHitEntity, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("RayCastHitEntity", asBEHAVE_RELEASE, "void f()", WRAP_MFN(RayCastHitEntity, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("RayCastHitEntity", "Float3 GetPosition()", WRAP_MFN(RayCastHitEntity, GetPosition), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterObjectMethod("GameWorld", "RayCastHitEntity@ CastRayGetFirstHit(Float3 start, Float3 end)", WRAP_MFN(GameWorld, CastRayGetFirstHitProxy), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Bind the NewtonBody as non counted handle and don't register any methods since it will only be used to compare the pointer //
	if(engine->RegisterObjectType("NewtonBody", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// Compare function //
	if(engine->RegisterObjectMethod("RayCastHitEntity", "bool DoesBodyMatchThisHit(NewtonBody@ body)", WRAP_MFN(RayCastHitEntity, DoesBodyMatchThisHit), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}



    // AINetworkCache
    if(engine->RegisterObjectType("AINetworkCache", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
    
	if(engine->RegisterGlobalFunction("AINetworkCache& GetAINetworkCache()", asFUNCTION(AINetworkCache::Get),
            asCALL_CDECL) < 0)
    {
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("AINetworkCache", "ScriptSafeVariableBlock@ GetVariable(const string &in name)",
            asMETHOD(AINetworkCache, GetVariableWrapper), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("AINetworkCache", "void SetVariable(ScriptSafeVariableBlock@ variable)",
            asMETHOD(AINetworkCache, SetVariableWrapper), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}    
    


	// ------------------ Game entities ------------------ //
	if(engine->RegisterObjectType("BaseObject", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseObject", asBEHAVE_ADDREF, "void f()", WRAP_MFN(BaseObject, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseObject", asBEHAVE_RELEASE, "void f()", WRAP_MFN(BaseObject, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("BaseObject", "NewtonBody@ CustomMessageGetNewtonBody()", WRAP_FN(BaseObjectCustomMessageGetNewtonBody), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	using namespace Entity;

	if(engine->RegisterObjectType("Prop", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Prop", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Prop, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Prop", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Prop, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Prop", "NewtonBody@ GetPhysicalBody()", WRAP_MFN(Prop, GetPhysicsBody), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Prop", "Float3 GetPosition()",  asFUNCTION(PropGetPosVal),
            asCALL_CDECL_OBJFIRST) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Prop", "Float3 GetVelocity()", WRAP_MFN(Prop, GetBodyVelocity), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectType("Brush", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Brush", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Brush, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("Brush", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Brush, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("Brush", "NewtonBody@ GetPhysicalBody()", WRAP_MFN(Brush, GetPhysicsBody), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterObjectType("TrackEntityController", 0, asOBJ_REF) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("TrackEntityController", asBEHAVE_ADDREF, "void f()", WRAP_MFN(TrackEntityController, AddRefProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("TrackEntityController", asBEHAVE_RELEASE, "void f()", WRAP_MFN(TrackEntityController, ReleaseProxy), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("TrackEntityController", "Float3 GetCurrentNodePosition()", WRAP_MFN(TrackEntityController, GetCurrentNodePosition), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("TrackEntityController", "Float3 GetNextNodePosition()", WRAP_MFN(TrackEntityController, GetNextNodePosition), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("TrackEntityController", "void SetProgressTowardsNextNode(float progress)", WRAP_MFN(TrackEntityController, SetProgressTowardsNextNode), asCALL_GENERIC) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}


	// ------------------ Entity casts ------------------ //
	if(engine->RegisterObjectBehaviour("Prop", asBEHAVE_IMPLICIT_REF_CAST, "BaseObject@ f()", WRAP_FN((DoReferenceCastStatic<Prop, BaseObject>)), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseObject", asBEHAVE_REF_CAST, "Prop@ f()", WRAP_FN((DoReferenceCastDynamic<BaseObject, Prop>)), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectBehaviour("Brush", asBEHAVE_IMPLICIT_REF_CAST, "BaseObject@ f()", WRAP_FN((DoReferenceCastStatic<Brush, BaseObject>)), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseObject", asBEHAVE_REF_CAST, "Brush@ f()", WRAP_FN((DoReferenceCastDynamic<BaseObject, Brush>)), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectBehaviour("TrackEntityController", asBEHAVE_IMPLICIT_REF_CAST, "BaseObject@ f()", WRAP_FN((DoReferenceCastStatic<TrackEntityController, BaseObject>)), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectBehaviour("BaseObject", asBEHAVE_REF_CAST, "TrackEntityController@ f()", WRAP_FN((DoReferenceCastDynamic<BaseObject, TrackEntityController>)), asCALL_GENERIC) < 0){
		ANGELSCRIPT_REGISTERFAIL;
	}

	// ------------------ Global functions ------------------ //

#ifdef _DEBUG


	if(engine->RegisterGlobalFunction("void DumpMemoryLeaks()", asFUNCTION(Engine::DumpMemoryLeaks), asCALL_CDECL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

	

#endif // _DEBUG


	if(engine->RegisterGlobalFunction("string GetLeviathanVersion()", asFUNCTION(GetLeviathanVersionProxy), asCALL_CDECL) < 0)
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
    typeids.insert(make_pair(engine->GetTypeIdByDecl("EventListener"), L"EventListener"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("TrackEntityController"), L"TrackEntityController"));
}
