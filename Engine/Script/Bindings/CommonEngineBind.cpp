// ------------------------------------ //
#include "CommonEngineBind.h"

#include "Addons/GameModule.h"
#include "Application/Application.h"
#include "Entities/Components.h"
#include "Entities/GameWorld.h"
#include "Events/CallableObject.h"
#include "Events/DelegateSlot.h"
#include "Events/Event.h"
#include "Events/EventHandler.h"
#include "FileSystem.h"
#include "Networking/NetworkCache.h"
#include "Rendering/Graphics.h"
#include "Script/Interface/ScriptDelegateSlot.h"
#include "Script/Interface/ScriptEventListener.h"
#include "Script/Interface/ScriptLock.h"
#include "Script/ScriptExecutor.h"
#include "Sound/SoundDevice.h"
#include "Threading/ThreadingManager.h"
#include "Utility/Random.h"
#include "Window.h"

#include "Engine.h"

#include "add_on/autowrapper/aswrappedcall.h"


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
    void* messagePtr = nullptr;

    if(gen->GetArgCount() >= 1)
        messagePtr = gen->GetArgObject(1);

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

//! \brief Prints current angelscript callstack
void PrintASCallStack()
{
    asIScriptContext* ctx = asGetActiveContext();

    if(!ctx) {

        LOG_ERROR("PrintASCallStack couldn't retrieve active context");
        return;
    }

    ScriptExecutor::PrintCallstack(ctx, *Logger::Get());
}

bool IsInGraphicalMode()
{
    return !Engine::Get()->GetNoGui();
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

// Sound
AudioSource* SoundDevicePlay2DProxy(
    SoundDevice* self, const std::string& filename, bool looping)
{
    auto source = self->Play2DSound(filename, looping);

    if(source)
        source->AddRef();

    return source.get();
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


static void QueueTaskDelayed(
    ThreadingManager* self, asIScriptFunction* callback, int milliseconds)
{
    // This is wrapped to ensure that if the task is never run but discarded that the resource
    // is cleanly released
    // TODO: as is said in BindThreadingManager exposing custom task types to script would be
    // better
    auto wrapped = std::shared_ptr<asIScriptFunction>(
        callback, [](asIScriptFunction* obj) { obj->Release(); });

    self->QueueTask(std::make_shared<DelayedTask>(
        [wrapped]() {
            try {
                ScriptRunningSetup ssetup;
                auto result =
                    ScriptExecutor::Get()->RunScript<void>(wrapped.get(), nullptr, ssetup);

                if(result.Result != SCRIPT_RUN_RESULT::Success)
                    LOG_WARNING("Script queued task: failed to run script callback");

            } catch(...) {

                LOG_ERROR("Queued task proxy passing exception up the call chain");
                throw;
            }
        },
        MillisecondDuration(milliseconds)));
}

// ------------------------------------ //
static float PIProxy = PI;

static float EPSILONProxy = EPSILON;

static int TICKSPEEDProxy = TICKSPEED;

static float DEGREES_TO_RADIANSProxy = DEGREES_TO_RADIANS;
static float RADIANS_TO_DEGREESProxy = RADIANS_TO_DEGREES;


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

    if(engine->RegisterObjectMethod("NamedVars",
           "string Serialize(const string &in lineprefix = \"\") const",
           asMETHOD(NamedVars, Serialize), asCALL_THISCALL) < 0) {
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
    // Bind event type enum //
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

    if(engine->RegisterObjectMethod("GenericEvent", "string GetType() const",
           asMETHOD(GenericEvent, GetType), asCALL_THISCALL) < 0) {
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

    if(engine->RegisterObjectMethod("Random", "float GetNumber(float min, float max)",
           asMETHODPR(Random, GetNumber, (float, float), float), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Random", "float GetFloat(float min, float max)",
           asMETHOD(Random, GetFloat), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindThreadingManager(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("ThreadingManager", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterFuncdef("void BackgroundThreadTask()") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // TODO: allow constructing task wrappers
    if(engine->RegisterObjectMethod("ThreadingManager",
           "void QueueTaskDelayed(BackgroundThreadTask@ callback, int milliseconds)",
           asFUNCTION(QueueTaskDelayed), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindWindow(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Window", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "void SaveScreenShot(const string &in filename)",
           asMETHOD(Window, SaveScreenShot), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "float GetAspectRatio() const",
           asMETHOD(Window, SaveScreenShot), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "void GetSize(int32 &out width, int32 &out height) const",
           asMETHOD(Window, GetSize), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "void GetPosition(int32 &out x, int32 &out y) const", asMETHOD(Window, GetPosition),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "void GetRelativeMouse(int32 &out x, int32 &out y) const",
           asMETHOD(Window, GetRelativeMouse), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "void GetUnclampedRelativeMouse(int32 &out x, int32 &out y) const",
           asMETHOD(Window, GetUnclampedRelativeMouse), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window",
           "void GetNormalizedRelativeMouse(float &out x, float &out y) const",
           asMETHOD(Window, GetNormalizedRelativeMouse), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "bool IsMouseOutsideWindowClientArea() const",
           asMETHOD(Window, IsMouseOutsideWindowClientArea), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "bool IsWindowFocused() const",
           asMETHOD(Window, IsWindowFocused), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Window", "int GetWindowNumber() const",
           asMETHOD(Window, GetWindowNumber), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->SetDefaultNamespace("Window") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("int GetGlobalWindowCount()",
           asFUNCTION(Window::GetGlobalWindowCount), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindGraphics(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Graphics", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Use the shader constructor taking a string instead
    // if(engine->RegisterObjectMethod("Graphics",
    //        "bs::HShader LoadShaderByName(const string &in name)",
    //        asMETHOD(Graphics, LoadShaderByName), asCALL_THISCALL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

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

    if(engine->RegisterObjectMethod("Engine", "FileSystem& GetFileSystem()",
           asMETHOD(Engine, GetFileSystem), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "ThreadingManager& GetThreadingManager()",
           asMETHOD(Engine, GetThreadingManager), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "Window& GetWindowEntity()",
           asMETHOD(Engine, GetWindowEntity), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Engine", "Graphics& GetGraphics()",
           asMETHOD(Engine, GetGraphics), asCALL_THISCALL) < 0) {
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

bool BindAudioSource(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("AudioSource", AudioSource);

    if(engine->RegisterObjectMethod("AudioSource", "void Resume()",
           asMETHOD(AudioSource, Resume), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(
           "AudioSource", "void Pause()", asMETHOD(AudioSource, Pause), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(
           "AudioSource", "void Stop()", asMETHOD(AudioSource, Stop), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("AudioSource", "bool IsPlaying() const",
           asMETHOD(AudioSource, IsPlaying), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("AudioSource", "void SetVolume(float volume)",
           asMETHOD(AudioSource, SetVolume), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("AudioSource", "bool HasInternalSource() const",
           asMETHOD(AudioSource, HasInternalSource), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindSound(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("AudioBuffer", Sound::AudioBuffer);

    if(!BindAudioSource(engine))
        return false;

    if(engine->RegisterObjectType("SoundDevice", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SoundDevice",
           "void Play2DSoundEffect(const string &in filename)",
           asMETHOD(SoundDevice, Play2DSoundEffect), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SoundDevice",
           "AudioSource@ Play2DSound(const string &in filename, bool looping)",
           asFUNCTION(SoundDevicePlay2DProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindFileSystem(asIScriptEngine* engine)
{
    // Many of the filesystem methods aren't safe to expose to every
    // script so they are hidden by default
    if(engine->RegisterEnum("FILEGROUP") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGELSCRIPT_REGISTER_ENUM_VALUE(FILEGROUP, FILEGROUP_MODEL);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(FILEGROUP, FILEGROUP_TEXTURE);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(FILEGROUP, FILEGROUP_SOUND);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(FILEGROUP, FILEGROUP_SCRIPT);
    ANGELSCRIPT_REGISTER_ENUM_VALUE(FILEGROUP, FILEGROUP_OTHER);

    if(engine->RegisterObjectType("FileSystem", 0, asOBJ_REF | asOBJ_NOHANDLE) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    const auto oldMask =
        engine->SetDefaultAccessMask(static_cast<AccessFlags>(ScriptAccess::FullFileSystem));

    if(engine->RegisterObjectMethod("FileSystem",
           "string SearchForFile(FILEGROUP which, const string &in name, const string &in "
           "extensions, bool searchall = true)",
           asMETHOD(FileSystem, SearchForFile), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Restore access for non-full access requiring static stuff
    engine->SetDefaultAccessMask(oldMask);

    // ------------------------------------ //
    // Static methods
    if(engine->SetDefaultNamespace("FileSystem") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // And back to protected functions
    engine->SetDefaultAccessMask(static_cast<AccessFlags>(ScriptAccess::FullFileSystem));

    if(engine->RegisterGlobalFunction("bool FileExists(const string &in filepath)",
           asFUNCTION(FileSystem::FileExists), asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // Restore settings //
    engine->SetDefaultAccessMask(oldMask);
    if(engine->SetDefaultNamespace("") < 0) {
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

    if(!BindFileSystem(engine))
        return false;

    if(!BindGraphics(engine))
        return false;

    if(!BindThreadingManager(engine))
        return false;

    if(!BindWindow(engine))
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
    if(engine->RegisterGlobalFunction("void assert(bool expression)",
           asFUNCTION(AngelScriptAssertWrapper), asCALL_GENERIC) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction(
           "bool IsInGraphicalMode()", asFUNCTION(IsInGraphicalMode), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction(
           "void PrintCallStack()", asFUNCTION(PrintASCallStack), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Global vars
    if(engine->RegisterGlobalProperty("const float PI", &PIProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const float EPSILON", &EPSILONProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty(
           "const float DEGREES_TO_RADIANS", &DEGREES_TO_RADIANSProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty(
           "const float RADIANS_TO_DEGREES", &RADIANS_TO_DEGREESProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const int TICKSPEED", &TICKSPEEDProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
