// ------------------------------------ //
#include "CommonEngineBind.h"

#include "Addons/GameModule.h"
#include "Entities/Components.h"
#include "Entities/GameWorld.h"
#include "Events/CallableObject.h"
#include "Events/DelegateSlot.h"
#include "Events/Event.h"
#include "Events/EventHandler.h"
#include "Networking/NetworkCache.h"
#include "Script/ScriptExecutor.h"
#include "Sound/SoundDevice.h"
#include "Utility/DataHandling/SimpleDatabase.h"
#include "Utility/Random.h"
#include "add_on/autowrapper/aswrappedcall.h"

#include "Application/Application.h"

#include "Script/Interface/ScriptDelegateSlot.h"
#include "Script/Interface/ScriptEventListener.h"
#include "Script/Interface/ScriptLock.h"

#include "Engine.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //

// This is an assert that prints the callstack for ease of use (and
// should probably also print local variables) (actually doesn't print
// anything as the callstack printing code is in exception handling so
// we just raise one)
void AngelScriptAssertWrapper(asIScriptGeneric* gen)
{
    bool check = gen->GetArgByte(0);

    if(check)
        return;

    // Assertion failed //
    void* messagePtr = gen->GetArgObject(1);
    std::string message;

    if(!messagePtr) {

        message = "No message specified in assert() call";

    } else {

        // Type check for safety //
        LEVIATHAN_ASSERT(gen->GetEngine()->GetTypeIdByDecl("string") == gen->GetArgTypeId(1),
            "AngelScriptAssertWrapper got invalid type of message object in generic call");

        message = *static_cast<std::string*>(messagePtr);
    }

    LOG_WRITE("[SCRIPT] [ASSERT] FAILED. Message: " + message);
    // Callstack is printed by the executor of this script when they get our exception
    // LOG_WRITE("Callstack:");

    asIScriptContext* ctx = asGetActiveContext();

    if(!ctx) {

        LOG_ERROR("Assertion couldn't retrieve active context");

        // Close game
        LeviathanApplication::Get()->MarkAsClosing();
        return;
    }

    // TODO: allow making script assertions fatal
    ctx->SetException(("Assertion failed: " + message).c_str());
}

// Event
GenericEvent* WrapperGenericEventFactory(const std::string& name)
{
    return new GenericEvent(name, NamedVars());
}

Event* WrapperEventFactory(EVENT_TYPE type)
{
    try {
        return new Event(type, nullptr);

    } catch(const Exception& e) {

        Logger::Get()->Error("Failed to construct Event for script, exception: ");
        e.PrintToLog();

        return nullptr;
    }
}

ScriptSafeVariableBlock* ScriptSafeVariableBlockFactoryString(
    const std::string& blockname, const std::string& valuestr)
{
    return new ScriptSafeVariableBlock(new StringBlock(valuestr), blockname);
}

template<typename TType>
ScriptSafeVariableBlock* ScriptSafeVariableBlockFactoryGeneric(
    const std::string& blockname, TType value)
{
    return new ScriptSafeVariableBlock(new DataBlock<TType>(value), blockname);
}

// ------------------------------------ //
static std::string GetLeviathanVersionProxy()
{
    return Leviathan::VERSIONS;
}

static void LOG_WRITEProxy(const std::string& str)
{
    LOG_WRITE(str);
}

static void LOG_INFOProxy(const std::string& str)
{
    LOG_INFO(str);
}

static void LOG_WARNINGProxy(const std::string& str)
{
    LOG_WARNING(str);
}

static void LOG_ERRORProxy(const std::string& str)
{
    LOG_ERROR(str);
}

static void DelegateRegisterProxy(Delegate* obj, asIScriptFunction* callback)
{
    if(!callback)
        return;

    obj->Register(
        Script::ScriptDelegateSlot::MakeShared<Script::ScriptDelegateSlot>(callback));
}

static NamedVars* NamedVarsFactory()
{

    return new NamedVars();
}

static void InvokeProxy(Engine* obj, asIScriptFunction* callback)
{

    obj->Invoke([=]() {

        try {
            ScriptRunningSetup ssetup;
            auto result = ScriptExecutor::Get()->RunScript<void>(callback, nullptr, ssetup);

            if(result.Result != SCRIPT_RUN_RESULT::Success)
                LOG_WARNING("InvokeProxy: failed to run script callback");

        } catch(...) {

            LOG_ERROR("Invoke proxy passing exception up the call chain");
            callback->Release();
            throw;
        }
        callback->Release();
    });
}
// ------------------------------------ //
static float PIProxy = PI;

static int TICKSPEEDProxy = TICKSPEED;


// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {

bool BindDataBlock(asIScriptEngine* engine);

//! \todo make this safe to be passed to the script
bool BindNamedVars(asIScriptEngine* engine)
{

    ANGELSCRIPT_REGISTER_REF_TYPE("NamedVars", NamedVars);

    if(!BindDataBlock(engine))
        return false;

    if(engine->RegisterObjectBehaviour("NamedVars", asBEHAVE_FACTORY, "NamedVars@ f()",
           asFUNCTION(NamedVarsFactory), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("NamedVars",
           "ScriptSafeVariableBlock@ GetSingleValueByName(const string &in name)",
           asMETHOD(NamedVars, GetScriptCompatibleValue), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("NamedVars",
           "bool AddValue(ScriptSafeVariableBlock@ value)",
           asMETHOD(NamedVars, AddScriptCompatibleValue), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}
// ------------------------------------ //
// Called by BindNamedVars
bool BindDataBlock(asIScriptEngine* engine)
{

    ANGELSCRIPT_REGISTER_REF_TYPE("ScriptSafeVariableBlock", ScriptSafeVariableBlock);

    // Some factories //
    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
           "ScriptSafeVariableBlock@ f(const string &in blockname, const string &in value)",
           asFUNCTION(ScriptSafeVariableBlockFactoryString), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
           "ScriptSafeVariableBlock@ f(const string &in blockname, float value)",
           asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<float>), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
           "ScriptSafeVariableBlock@ f(const string &in blockname, int value)",
           asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<int>), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
           "ScriptSafeVariableBlock@ f(const string &in blockname, double value)",
           asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<double>), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
           "ScriptSafeVariableBlock@ f(const string &in blockname, int8 value)",
           asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<char>), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ScriptSafeVariableBlock", asBEHAVE_FACTORY,
           "ScriptSafeVariableBlock@ f(const string &in blockname, bool value)",
           asFUNCTION(ScriptSafeVariableBlockFactoryGeneric<bool>), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Implicit casts for normal types //
    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "string opImplConv() const",
           WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<std::string>),
           asCALL_GENERIC) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "int opImplConv() const",
           WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<int>),
           asCALL_GENERIC) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "int8 opImplConv() const",
           WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<char>),
           asCALL_GENERIC) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "float opImplConv() const",
           WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<float>),
           asCALL_GENERIC) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "double opImplConv() const",
           WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<double>),
           asCALL_GENERIC) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "bool opImplConv() const",
           WRAP_MFN(ScriptSafeVariableBlock, ConvertAndReturnVariable<bool>),
           asCALL_GENERIC) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // type check //
    if(engine->RegisterObjectMethod("ScriptSafeVariableBlock", "bool IsValidType()",
           asMETHOD(ScriptSafeVariableBlock, IsValidType), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}
// ------------------------------------ //
bool BindEvents(asIScriptEngine* engine)
{

    // bind event type enum //
    if(engine->RegisterEnum("EVENT_TYPE") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGELSCRIPT_REGISTER_ENUM_VALUE(EVENT_TYPE, EVENT_TYPE_SHOW);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(EVENT_TYPE, EVENT_TYPE_HIDE);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(EVENT_TYPE, EVENT_TYPE_TICK);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(EVENT_TYPE, EVENT_TYPE_LISTENERVALUEUPDATED);

    // bind event //
    ANGELSCRIPT_REGISTER_REF_TYPE("Event", Event);

    if(engine->RegisterObjectBehaviour("Event", asBEHAVE_FACTORY, "Event@ f(EVENT_TYPE type)",
           asFUNCTION(WrapperEventFactory), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // bind generic event //
    ANGELSCRIPT_REGISTER_REF_TYPE("GenericEvent", GenericEvent);

    // Factory //
    if(engine->RegisterObjectBehaviour("GenericEvent", asBEHAVE_FACTORY,
           "GenericEvent@ f(const string &in typename)",
           asFUNCTION(WrapperGenericEventFactory), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Data get function //
    if(engine->RegisterObjectMethod("GenericEvent", "NamedVars@ GetNamedVars()",
           asMETHOD(GenericEvent, GetNamedVarsRefCounted), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    // Event handler which cannot be instantiated or copied around //
    if(engine->RegisterObjectType("EventHandler", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Script event firing //
    if(engine->RegisterObjectMethod("EventHandler", "void CallEvent(GenericEvent@ event)",
           asMETHODPR(EventHandler, CallEvent, (GenericEvent*), void), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Event listener //
    ANGELSCRIPT_REGISTER_REF_TYPE("EventListener", Script::EventListener);

    if(engine->RegisterFuncdef("int OnEventCallback(Event@ event)") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterFuncdef("int OnGenericEventCallback(GenericEvent@ event)") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("EventListener", asBEHAVE_FACTORY,
           "EventListener@ f(OnEventCallback@ onevent, OnGenericEventCallback@ ongeneric)",
           asFUNCTION(Script::EventListenerFactory), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("EventListener", "bool RegisterForEvent(EVENT_TYPE type)",
           asMETHOD(Script::EventListener, RegisterForEventType), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("EventListener",
           "bool RegisterForEvent(const string &in name)",
           asMETHOD(Script::EventListener, RegisterForEventGeneric), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}
// ------------------------------------ //
bool BindRandom(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("Random", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // TODO: constructor and reference counting for proper use from scripts

    if(engine->RegisterObjectMethod("Random", "int GetNumber(int min, int max)",
           asMETHODPR(Random, GetNumber, (int, int), int), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Random", "int GetNumber(float min, float max)",
           asMETHODPR(Random, GetNumber, (float, float), float), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Random", "int GetFloat(float min, float max)",
           asMETHOD(Random, GetFloat), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

// ------------------------------------ //
bool BindEngine(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("Engine", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Global get function //
    if(engine->RegisterGlobalFunction(
           "Engine& GetEngine()", asFUNCTION(Engine::Get), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Engine owned singletons //
    if(!BindRandom(engine))
        return false;

    if(engine->RegisterObjectMethod("Engine", "Random& GetRandom()",
           asMETHOD(Engine, GetRandom), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "EventHandler& GetEventHandler()",
           asMETHOD(Engine, GetEventHandler), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "SoundDevice& GetSoundDevice()",
           asMETHOD(Engine, GetSoundDevice), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //

    if(engine->RegisterObjectMethod("Engine", "int64 GetTimeSinceLastTick()",
           asMETHOD(Engine, GetTimeSinceLastTick), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "int GetCurrentTick()",
           asMETHOD(Engine, GetCurrentTick), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "int GetWindowOpenCount()",
           asMETHOD(Engine, GetWindowOpenCount), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(
           "Engine", "void MarkQuit()", asMETHOD(Engine, MarkQuit), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "bool IsOnMainThread()",
           asMETHOD(Engine, IsOnMainThread), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterFuncdef("void InvokeCallbackFunc()") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "void Invoke(InvokeCallbackFunc@ callback)",
           asFUNCTION(InvokeProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindApplication(asIScriptEngine* engine)
{

    if(engine->RegisterEnum("NETWORKED_TYPE") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGELSCRIPT_REGISTER_ENUM_VALUE(NETWORKED_TYPE, Client);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(NETWORKED_TYPE, Server);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(NETWORKED_TYPE, Master);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(NETWORKED_TYPE, Error);

    if(engine->RegisterObjectType("LeviathanApplication", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Global get function //
    if(engine->RegisterGlobalFunction("LeviathanApplication& GetLeviathanApplication()",
           asFUNCTION(LeviathanApplication::Get), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("LeviathanApplication", "void MarkAsClosing()",
           asMETHOD(LeviathanApplication, MarkAsClosing), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("LeviathanApplication", "bool Quitting()",
           asMETHOD(LeviathanApplication, Quitting), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("LeviathanApplication",
           "NETWORKED_TYPE GetProgramNetType() const",
           asMETHOD(LeviathanApplication, GetProgramNetType), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindGameModule(asIScriptEngine* engine)
{

    ANGELSCRIPT_REGISTER_REF_TYPE("GameModule", GameModule);

    // Bind simple name get function //
    if(engine->RegisterObjectMethod("GameModule", "string GetDescription(bool full = false)",
           asMETHOD(GameModule, GetDescription), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}
// ------------------------------------ //
bool BindDelegates(asIScriptEngine* engine)
{

    if(engine->RegisterFuncdef("void DelegateCallbackFunc(NamedVars@ values)") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGELSCRIPT_REGISTER_REF_TYPE("Delegate", Delegate);

    if(engine->RegisterObjectMethod("Delegate", "void Call(NamedVars@ values) const",
           asMETHODPR(Delegate, Call, (NamedVars*)const, void), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Delegate",
           "void Register(DelegateCallbackFunc@ callback)", asFUNCTION(DelegateRegisterProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}

bool BindSound(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("SoundDevice", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SoundDevice",
           "bool PlaySoundEffect(const string &in file)",
           asMETHOD(SoundDevice, PlaySoundEffect), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

} // namespace Leviathan

bool Leviathan::BindEngineCommon(asIScriptEngine* engine)
{

    if(!BindNamedVars(engine))
        return false;

    if(!BindEvents(engine))
        return false;

    if(!BindSound(engine))
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
           asFUNCTION(GetLeviathanVersionProxy), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void LOG_WRITE(const string &in message)",
           asFUNCTION(LOG_WRITEProxy), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void LOG_INFO(const string &in message)",
           asFUNCTION(LOG_INFOProxy), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void LOG_WARNING(const string &in message)",
           asFUNCTION(LOG_WARNINGProxy), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void LOG_ERROR(const string &in message)",
           asFUNCTION(LOG_ERRORProxy), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    // LOG_FATAL not bound
    // Use assert instead

    if(engine->RegisterGlobalFunction("void Print(const string &in message)",
           asFUNCTION(Logger::Print), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("void assert(bool expression, const string &in message)",
           asFUNCTION(AngelScriptAssertWrapper), asCALL_GENERIC) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Global vars
    if(engine->RegisterGlobalProperty("const float PI", &PIProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const int TICKSPEED", &TICKSPEEDProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}
