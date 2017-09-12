// ------------------------------------ //
#include "CommonEngineBind.h"

#include "Events/Event.h"
#include "Script/ScriptExecutor.h"
#include "Events/EventHandler.h"
#include "Events/CallableObject.h"
#include "Events/Event.h"
#include "Events/DelegateSlot.h"
#include "Utility/DataHandling/SimpleDatabase.h"
#include "Addons/GameModule.h"
#include "Entities/Components.h"
#include "Entities/GameWorld.h"
#include "add_on/autowrapper/aswrappedcall.h"
#include "Networking/NetworkCache.h"

#include "Application/Application.h"

#include "Script/Interface/ScriptEventListener.h"
#include "Script/Interface/ScriptLock.h"
#include "Script/Interface/ScriptDelegateSlot.h"

#include "Engine.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
// Event
GenericEvent* WrapperGenericEventFactory(const std::string &name){

	return new GenericEvent(name, NamedVars());
}

Event* WrapperEventFactory(EVENT_TYPE type){

    try{
        return new Event(type, nullptr);
        
    } catch(const Exception &e){

        Logger::Get()->Error("Failed to construct Event for script, exception: ");
        e.PrintToLog();

        return nullptr;
    }
}

ScriptSafeVariableBlock* ScriptSafeVariableBlockFactoryString(
    const std::string &blockname, const std::string &valuestr)
{
	return new ScriptSafeVariableBlock(new StringBlock(valuestr), blockname);
}

template<typename TType>
    ScriptSafeVariableBlock* ScriptSafeVariableBlockFactoryGeneric(
        const std::string &blockname, TType value)
{
	return new ScriptSafeVariableBlock(new DataBlock<TType>(value), blockname);
}

// ------------------------------------ //
static std::string GetLeviathanVersionProxy(){

    return Leviathan::VERSIONS;
}

static void LOG_WRITEProxy(const std::string &str){

    LOG_WRITE(str);
}

static void LOG_INFOProxy(const std::string &str){

    LOG_INFO(str);
}

static void LOG_WARNINGProxy(const std::string &str){

    LOG_WARNING(str);
}

static void LOG_ERRORProxy(const std::string &str){

    LOG_ERROR(str);
}

static void DelegateRegisterProxy(Delegate* obj, asIScriptFunction* callback){

    if(!callback)
        return;

    obj->Register(Script::ScriptDelegateSlot::MakeShared(
            new Script::ScriptDelegateSlot(callback)));
}

static NamedVars* NamedVarsFactory(){

    return new NamedVars();
}

static void InvokeProxy(Engine* obj, asIScriptFunction* callback){

    obj->Invoke([=](){

            try{
                ScriptRunningSetup ssetup;
                ScriptExecutor::Get()->RunSetUp(callback, &ssetup);
            } catch(...){

                LOG_ERROR("Invoke proxy passing exception up the call chain");
                callback->Release();
                throw;
            }
            callback->Release();
        });
}


// ------------------------------------ //
// Start of the actual bind
namespace Leviathan{

bool BindDataBlock(asIScriptEngine* engine);

//! \todo make this safe to be passed to the script
bool BindNamedVars(asIScriptEngine* engine){

    ANGELSCRIPT_REGISTER_REF_TYPE("NamedVars", NamedVars);

    if(!BindDataBlock(engine))
        return false;

    if(engine->RegisterObjectBehaviour("NamedVars", asBEHAVE_FACTORY,
            "NamedVars@ f()", 
            asFUNCTION(NamedVarsFactory), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("NamedVars",
            "ScriptSafeVariableBlock@ GetSingleValueByName(const string &in name)",
            asMETHOD(NamedVars, GetScriptCompatibleValue), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("NamedVars",
            "bool AddValue(ScriptSafeVariableBlock@ value)",
            asMETHOD(NamedVars, AddScriptCompatibleValue), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    
    return true;
}
// ------------------------------------ //
// Called by BindNamedVars
bool BindDataBlock(asIScriptEngine* engine){

    ANGELSCRIPT_REGISTER_REF_TYPE("ScriptSafeVariableBlock", ScriptSafeVariableBlock);

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
    
    // Implicit casts for normal types //
    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "string opImplConv() const",
            WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<std::string>),
            asCALL_GENERIC) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "int opImplConv() const",
            WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<int>),
            asCALL_GENERIC) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "int8 opImplConv() const",
            WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<char>),
            asCALL_GENERIC) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "float opImplConv() const",
            WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<float>),
            asCALL_GENERIC) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "double opImplConv() const",
            WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<double>),
            asCALL_GENERIC) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "bool opImplConv() const",
            WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<bool>),
            asCALL_GENERIC) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // type check //
    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "bool IsValidType()",
            asMETHOD(ScriptSafeVariableBlock, IsValidType), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    

    return true;
}
// ------------------------------------ //
bool BindEvents(asIScriptEngine* engine){

    // bind event type enum //
    if(engine->RegisterEnum("EVENT_TYPE") < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    ANGELSCRIPT_REGISTER_ENUM_VALUE(EVENT_TYPE, EVENT_TYPE_SHOW);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(EVENT_TYPE, EVENT_TYPE_HIDE);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(EVENT_TYPE, EVENT_TYPE_TICK);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(EVENT_TYPE, EVENT_TYPE_LISTENERVALUEUPDATED);
    
    // bind event //
    ANGELSCRIPT_REGISTER_REF_TYPE("Event", Event);

    if(engine->RegisterObjectBehaviour("Event", asBEHAVE_FACTORY, "Event@ f(EVENT_TYPE type)",
            asFUNCTION(WrapperEventFactory), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }    

    // bind generic event //
    ANGELSCRIPT_REGISTER_REF_TYPE("GenericEvent", GenericEvent);

    // Factory //
    if(engine->RegisterObjectBehaviour("GenericEvent", asBEHAVE_FACTORY,
            "GenericEvent@ f(const string &in typename)",
            asFUNCTION(WrapperGenericEventFactory), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    // Data get function //
    if(engine->RegisterObjectMethod("GenericEvent", "NamedVars@ GetNamedVars()",
            asMETHOD(GenericEvent, GetNamedVarsRefCounted), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }


    // Event handler which cannot be instantiated or copied around //
    if(engine->RegisterObjectType("EventHandler", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Global get function //
    if(engine->RegisterGlobalFunction("EventHandler& GetEventHandler()",
            asFUNCTION(EventHandler::Get),
            asCALL_CDECL) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Script event firing //
    if(engine->RegisterObjectMethod("EventHandler", "void CallEvent(GenericEvent@ event)",
            asMETHOD(EventHandler, CallEventGenericProxy), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Event listener //
    ANGELSCRIPT_REGISTER_REF_TYPE("EventListener", Script::EventListener);

    if(engine->RegisterFuncdef("int OnEventCallback(Event@ event)") < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    if(engine->RegisterFuncdef("int OnGenericEventCallback(GenericEvent@ event)") < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("EventListener", asBEHAVE_FACTORY,
            "EventListener@ f(OnEventCallback@ onevent, OnGenericEventCallback@ ongeneric)",
            asFUNCTION(Script::EventListenerFactory), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("EventListener", "bool RegisterForEvent(EVENT_TYPE type)",
            asMETHOD(Script::EventListener, RegisterForEventType), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("EventListener",
            "bool RegisterForEvent(const string &in name)",
            asMETHOD(Script::EventListener, RegisterForEventGeneric), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    
    return true;
}
// ------------------------------------ //
bool BindEngine(asIScriptEngine* engine){

    if(engine->RegisterObjectType("Engine", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Global get function //
    if(engine->RegisterGlobalFunction("Engine& GetEngine()",
            asFUNCTION(Engine::Get), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine",
            "int64 GetTimeSinceLastTick()",
            asMETHOD(Engine, GetTimeSinceLastTick), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine",
            "int GetCurrentTick()",
            asMETHOD(Engine, GetCurrentTick), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine",
            "int GetWindowOpenCount()",
            asMETHOD(Engine, GetWindowOpenCount), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine",
            "void MarkQuit()",
            asMETHOD(Engine, MarkQuit), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine",
            "bool IsOnMainThread()",
            asMETHOD(Engine, IsOnMainThread), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterFuncdef("void InvokeCallbackFunc()") < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine",
            "void Invoke(InvokeCallbackFunc@ callback)",
            asFUNCTION(InvokeProxy), asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    return true;
}
// ------------------------------------ //
bool BindApplication(asIScriptEngine* engine){

    if(engine->RegisterEnum("NETWORKED_TYPE") < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    ANGELSCRIPT_REGISTER_ENUM_VALUE(NETWORKED_TYPE, Client);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(NETWORKED_TYPE, Server);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(NETWORKED_TYPE, Master);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(NETWORKED_TYPE, Error);

    if(engine->RegisterObjectType("LeviathanApplication", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Global get function //
    if(engine->RegisterGlobalFunction("LeviathanApplication& GetLeviathanApplication()",
            asFUNCTION(LeviathanApplication::Get), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("LeviathanApplication",
            "void MarkAsClosing()",
            asMETHOD(LeviathanApplication, MarkAsClosing), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("LeviathanApplication",
            "bool Quitting()",
            asMETHOD(LeviathanApplication, Quitting), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("LeviathanApplication",
            "NETWORKED_TYPE GetProgramNetType() const",
            asMETHOD(LeviathanApplication, GetProgramNetType), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }        

    return true;
}
// ------------------------------------ //
bool BindGameModule(asIScriptEngine* engine){

    ANGELSCRIPT_REGISTER_REF_TYPE("GameModule", GameModule);

    // Bind simple name get function //
    if(engine->RegisterObjectMethod("GameModule", "string GetDescription(bool full = false)",
            asMETHOD(GameModule, GetDescription), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    
    return true;
}


bool BindDelegates(asIScriptEngine* engine){

    if(engine->RegisterFuncdef("void DelegateCallbackFunc(NamedVars@ values)") < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGELSCRIPT_REGISTER_REF_TYPE("Delegate", Delegate);

    if(engine->RegisterObjectMethod("Delegate", "void Call(NamedVars@ values) const",
            asMETHODPR(Delegate, Call, (NamedVars*) const, void), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Delegate",
            "void Register(DelegateCallbackFunc@ callback)",
            asFUNCTION(DelegateRegisterProxy), asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }    
    
    
    return true;
}

}

bool Leviathan::BindEngineCommon(asIScriptEngine* engine){

    if(!BindNamedVars(engine))
        return false;
    
    if(!BindEvents(engine))
        return false;

    if(!BindEngine(engine))
        return false;

    if(!BindApplication(engine))
        return false;

    if(!BindGameModule(engine))
        return false;

    if(!BindDelegates(engine))
        return false;
    

    // ------------------ Global functions ------------------ //
    if(engine->RegisterGlobalFunction("string GetLeviathanVersion()",
            asFUNCTION(GetLeviathanVersionProxy), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void LOG_WRITE(const string &in message)",
            asFUNCTION(LOG_WRITEProxy), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void LOG_INFO(const string &in message)",
            asFUNCTION(LOG_INFOProxy), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void LOG_WARNING(const string &in message)",
            asFUNCTION(LOG_WARNINGProxy), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void LOG_ERROR(const string &in message)",
            asFUNCTION(LOG_ERRORProxy), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    // LOG_FATAL not bound

	if(engine->RegisterGlobalFunction("void Print(const string &in message)",
            asFUNCTION(Logger::Print), asCALL_CDECL) < 0)
    {
		// error abort //
		Logger::Get()->Error("ScriptExecutor: Init: AngelScript: failed to register print");
        throw Exception("Script bind failed");
	}    

    return true;
}

void Leviathan::RegisterEngineCommon(asIScriptEngine* engine,
    std::map<int, std::string> &typeids)
{

    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Event"), "Event"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("GenericEvent"), "GenericEvent"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("ScriptSafeVariableBlock"),
            "ScriptSafeVariableBlock"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("NamedVars"), "NamedVars"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("GameModule"), "GameModule"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("EventListener"), "EventListener"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("LeviathanApplication"),
            "LeviathanApplication"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Engine"),
            "Engine"));    
}



