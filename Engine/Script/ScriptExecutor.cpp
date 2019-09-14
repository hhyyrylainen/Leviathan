// ------------------------------------ //
#include "ScriptExecutor.h"

#include "AccessMask.h"
#include "Application/Application.h"
#include "Iterators/StringIterator.h"
#include "ScriptModule.h"
#include "ScriptNotifiers.h"

#include <add_on/datetime/datetime.h>
#include <add_on/scriptany/scriptany.h>
#include <add_on/scriptarray/scriptarray.h>
#include <add_on/scriptdictionary/scriptdictionary.h>
#include <add_on/scriptgrid/scriptgrid.h>
#include <add_on/scripthandle/scripthandle.h>
#include <add_on/scripthelper/scripthelper.h>
#include <add_on/scriptmath/scriptmath.h>
#include <add_on/scriptmath/scriptmathcomplex.h>
#include <add_on/scriptstdstring/scriptstdstring.h>
#include <add_on/weakref/weakref.h>

// Bindings
#include "Bindings/BSFBind.h"
#include "Bindings/BindStandardFunctions.h"
#include "Bindings/CommonEngineBind.h"
#include "Bindings/EntityBind.h"
#include "Bindings/GuiScriptBind.h"
#include "Bindings/PhysicsBind.h"
#include "Bindings/TypesBind.h"

// Exception support

using namespace Leviathan;
// ------------------------------------ //
namespace Leviathan {

void ScriptMessageCallback(const asSMessageInfo* msg, void* param)
{

    if(msg->type == asMSGTYPE_WARNING) {

        Logger::Get()->Write(std::string("[SCRIPT] [WARNING] ") + msg->section + " (" +
                             std::to_string(msg->row) + ", " + std::to_string(msg->col) +
                             ") : " + msg->message);

    } else if(msg->type == asMSGTYPE_INFORMATION) {

        Logger::Get()->Write(std::string("[SCRIPT] [INFO] ") + msg->section + " (" +
                             std::to_string(msg->row) + ", " + std::to_string(msg->col) +
                             ") : " + msg->message);

    } else {

        Logger::Get()->Write(std::string("[SCRIPT] [ERROR] ") + msg->section + " (" +
                             std::to_string(msg->row) + ", " + std::to_string(msg->col) +
                             ") : " + msg->message);
    }
}

#ifdef ANGELSCRIPT_HAS_TRANSLATE_CALLBACK
void ScriptTranslateExceptionCallback(asIScriptContext* context, void* userdata)
{
    try {
        std::rethrow_exception(std::current_exception());

    } catch(const Leviathan::InvalidAccess& e) {
        context->SetException(
            (std::string("Caught Leviathan::InvalidAccess exception: ") + e.what()).c_str());
    } catch(const Leviathan::InvalidArgument& e) {
        context->SetException(
            (std::string("Caught Leviathan::InvalidArgument exception: ") + e.what()).c_str());
    } catch(const Leviathan::InvalidState& e) {
        context->SetException(
            (std::string("Caught Leviathan::InvalidState exception: ") + e.what()).c_str());
    } catch(const Leviathan::InvalidType& e) {
        context->SetException(
            (std::string("Caught Leviathan::InvalidType exception: ") + e.what()).c_str());
    } catch(const Leviathan::NotFound& e) {
        context->SetException(
            (std::string("Caught Leviathan::NotFound exception: ") + e.what()).c_str());
    } catch(const Leviathan::NULLPtr& e) {
        context->SetException(
            (std::string("Caught Leviathan::NULLPtr exception: ") + e.what()).c_str());
    } catch(const Leviathan::Exception& e) {
        context->SetException(
            (std::string("Caught Leviathan::Exception: ") + e.what()).c_str());
        // } catch(const Ogre::Exception& e) {
        //     context->SetException((std::string("Caught Ogre::Exception: ") +
        //     e.what()).c_str());
    } catch(const std::exception& e) {
        context->SetException(
            (std::string("Caught (unknown type) application exception: ") + e.what()).c_str());
    } catch(...) {
        // Use default message
    }
}
#endif // ANGELSCRIPT_HAS_TRANSLATE_CALLBACK

asIScriptContext* RequestContextCallback(asIScriptEngine* engine, void* userdata)
{
    return static_cast<ScriptExecutor*>(userdata)->_GetContextForExecution();
}

void ReturnContextCallback(asIScriptEngine* engine, asIScriptContext* context, void* userdata)
{
    static_cast<ScriptExecutor*>(userdata)->_DoneWithContext(context);
}

} // namespace Leviathan

ScriptExecutor::ScriptExecutor() : engine(nullptr), AllocatedScriptModules()
{

    instance = this;

    // Initialize AngelScript //
    engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if(engine == nullptr) {

        Logger::Get()->Error("ScriptExecutor: Init: asCreateScriptEngine failed");
        Logger::Get()->Info("ScriptExecutor: tried to init angelscript version " +
                            Convert::ToString(ANGELSCRIPT_VERSION));
        Logger::Get()->Write("Did you use a wrong angelscript version? copy header files to "
                             "leviathan/Angelscript/include from your angelscript.zip");
        throw Exception("Failed to init angelscript");
    }

    // set callback to error report function //
    engine->SetMessageCallback(asFUNCTION(ScriptMessageCallback), 0, asCALL_CDECL);

    // The ScriptExecutor can be retrieved from asIScriptEngine user data
    engine->SetUserData(this);

#ifdef ANGELSCRIPT_HAS_TRANSLATE_CALLBACK
    // Set error translation callback
    engine->SetTranslateAppExceptionCallback(
        asFUNCTION(ScriptTranslateExceptionCallback), nullptr, asCALL_CDECL);
#endif // ANGELSCRIPT_HAS_TRANSLATE_CALLBACK

    // Context pool usage callbacks
    engine->SetContextCallbacks(RequestContextCallback, ReturnContextCallback, this);


    // Builtins are in this access group //
    const auto initialMask =
        engine->SetDefaultAccessMask(static_cast<AccessFlags>(ScriptAccess::Builtin));

    // math functions //
    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);

    // register script string type //
    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    // register other script extensions //
    RegisterStdStringUtils(engine);


    RegisterScriptDateTime(engine);

    // register dictionary object //
    RegisterScriptDictionary(engine);

    // Register the grid addon //
    RegisterScriptGrid(engine);

    // Register reference handles //
    RegisterScriptHandle(engine);

    RegisterScriptWeakRef(engine);

    RegisterScriptAny(engine);

    // Put the extended standard stuff also in Builtin access mask

    if(!BindStandardFunctions(engine))
        throw Exception("BindStandardFunctions failed");

    // All normal engine stuff is in the DefaultEngine access mask //
    engine->SetDefaultAccessMask(static_cast<AccessFlags>(ScriptAccess::DefaultEngine));

    if(!BindBSF(engine))
        throw Exception("BindBSF failed");

    if(!BindTypes(engine))
        throw Exception("BindTypes failed");

    if(!BindPhysics(engine))
        throw Exception("BindNewton failed");

    if(!BindEngineCommon(engine))
        throw Exception("BindEngineCommon failed");

    if(!BindGUI(engine))
        throw Exception("BindGUI failed");

    if(!BindEntity(engine))
        throw Exception("BindEntity failed");

    // Bind notifiers //
    if(!RegisterNotifiersWithAngelScript(engine)) {
        // failed //
        LOG_ERROR("ScriptExecutor: Init: AngelScript: register Notifier types failed");
        throw Exception("Script bind failed");
    }

    // Restore the default mask to let the application do what it wants with the masks
    engine->SetDefaultAccessMask(initialMask);

    // bind application specific //
    auto leviathanengine = Engine::GetEngine();

    if(leviathanengine) {

        if(!leviathanengine->GetOwningApplication()->InitLoadCustomScriptTypes(engine)) {

            LOG_ERROR("ScriptExecutor: Init: AngelScript: application register failed");
            throw Exception("Script bind failed");
        }
    }

    // Verify void type //
    const auto actualVoid = engine->GetTypeIdByDecl("void");
    if(actualVoid != ANGELSCRIPT_VOID_TYPEID) {

        LOG_FATAL("ScriptExecutor: angelscript void type has changed! expected " +
                  std::to_string(ANGELSCRIPT_VOID_TYPEID) +
                  " (constexpr) == " + std::to_string(actualVoid) + " (actual value)");
    }
}
ScriptExecutor::~ScriptExecutor()
{
    {
        Lock lock(ModulesLock);
        auto end = AllocatedScriptModules.end();
        for(auto iter = AllocatedScriptModules.begin(); iter != end; ++iter) {

            (*iter)->Release();
        }

        // release/delete all modules //
        AllocatedScriptModules.clear();
    }

    // Release all context objects
    for(asIScriptContext* context : ContextPool) {
        context->Release();
    }

    ContextPool.clear();

    // release AngelScript //
    if(engine) {

        engine->Release();
        engine = nullptr;
    }

    instance = nullptr;
}

DLLEXPORT ScriptExecutor* Leviathan::ScriptExecutor::Get()
{
    return instance;
}

ScriptExecutor* Leviathan::ScriptExecutor::instance = nullptr;
// ------------------------------------ //
DLLEXPORT asIScriptFunction* ScriptExecutor::GetFunctionFromModule(
    ScriptModule* module, ScriptRunningSetup& parameters)
{
    if(!module) {

        if(parameters.PrintErrors) {
            Logger::Get()->Error("ScriptExecutor: GetFunctionFromModule: module is nullptr");
        }

        return nullptr;
    }

    asIScriptFunction* func;

    asIScriptModule* asModule = module->GetModule();

    if(!asModule) {

        if(parameters.PrintErrors) {
            Logger::Get()->Error(
                "ScriptExecutor: GetFunctionFromModule: cannot get function from "
                "an invalid script module: " +
                module->GetInfoString());
        }

        return nullptr;
    }

    // Get the entry function from the module //
    if(!parameters.FullDeclaration) {

        func = asModule->GetFunctionByName(parameters.Entryfunction.c_str());

    } else {

        func = asModule->GetFunctionByDecl(parameters.Entryfunction.c_str());
    }

    if(!_CheckScriptFunctionPtr(func, parameters, module)) {

        return nullptr;
    }

    return func;
}
// ------------------------------------ //
DLLEXPORT void ScriptExecutor::RunReleaseRefOnObject(void* obj, int objid)
{
    asITypeInfo* info = engine->GetTypeInfoById(objid);

    asUINT count = info->GetBehaviourCount();

    for(asUINT i = 0; i < count; ++i) {

        asEBehaviours behaviour;
        asIScriptFunction* func = info->GetBehaviourByIndex(i, &behaviour);

        if(!func) {

            LOG_ERROR("ScriptExecutor: RunReleaseRefOnObject: failed to get behaviour");
            continue;
        }

        if(behaviour == asBEHAVE_RELEASE) {

            LOG_INFO(
                "ScriptExecutor: RunReleaseRefOnObject: Found asBEHAVE_RELEASE, calling it");

            ScriptRunningSetup ssetup;
            const auto result = RunScriptMethod<void>(ssetup, func, obj);

            if(result.Result != SCRIPT_RUN_RESULT::Success)
                throw Exception("Failed to run release behaviour");

            return;
        }
    }

    throw Exception("Didn't find release ref behaviour on object type");
}
// ------------------------------------ //
DLLEXPORT std::unique_ptr<CustomScriptRun> ScriptExecutor::PrepareCustomScriptRun(
    asIScriptFunction* func, ScriptRunningSetup extraoptions /*= ScriptRunningSetup()*/)
{
    if(!func)
        return nullptr;

    auto run = std::make_unique<CustomScriptRun>(this);
    run->Setup = extraoptions;
    run->Func = func;

    // // TODO: this is a performance waste if there are no errors
    // std::shared_ptr<ScriptModule> module =
    //     GetScriptModuleByFunction(func, run->Setup.PrintErrors);

    // Create a running context for the function //
    run->Context = _GetContextForExecution();

    if(!run->Context) {
        // Should this be fatal?
        LOG_ERROR("ScriptExecutor: PrepareCustomScriptRun: failed to create a new context");
        return nullptr;
    }

    if(!_PrepareContextForPassingParameters(
           func, run->Context, run->Setup, run->Module.get())) {

        _DoneWithContext(run->Context);
        return nullptr;
    }

    return run;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<ScriptModule> ScriptExecutor::GetScriptModuleByFunction(
    asIScriptFunction* func, bool reporterror)
{
    const char* nameStr = func->GetModuleName();

    if(!nameStr || strlen(nameStr) <= 1)
        return nullptr;

    // static_cast<ScriptModule*>(func->GetModule()->GetUserData());
    // then find by pointer in valid script modules

    std::shared_ptr<ScriptModule> module =
        GetModuleByAngelScriptName(func->GetModuleName()).lock();

    if(!module) {

        if(reporterror) {
            LOG_WARNING("ScriptExecutor: GetScriptModuleByFunction: the module is no longer "
                        "available: " +
                        std::string(func->GetModuleName()));
        }
    }

    return module;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptExecutor::_CheckScriptFunctionPtr(
    asIScriptFunction* func, ScriptRunningSetup& parameters, ScriptModule* scriptmodule)
{
    // Check is it null //
    if(func == nullptr) {
        // Set exists state //
        parameters.ScriptExisted = false;

        // Check should we print an error //
        if(parameters.PrintErrors && parameters.ErrorOnNonExistingFunction) {

            LOG_ERROR(
                "ScriptExecutor: RunScript: Could not find starting function: " +
                parameters.Entryfunction +
                (scriptmodule ? (" in: " + scriptmodule->GetInfoString()) : std::string()));

            if(scriptmodule)
                scriptmodule->PrintFunctionsInModule();
        }

        // Not valid //
        return false;
    }

    // Set exists state //
    parameters.ScriptExisted = true;

    return true;
}

DLLEXPORT bool Leviathan::ScriptExecutor::_PrepareContextForPassingParameters(
    asIScriptFunction* func, asIScriptContext* ScriptContext, ScriptRunningSetup& parameters,
    ScriptModule* scriptmodule)
{
    if(ScriptContext->Prepare(func) < 0) {

        Logger::Get()->Error(
            "ScriptExecutor: RunScript: prepare context failed, func: " +
            parameters.Entryfunction +
            (scriptmodule ? (" in: " + scriptmodule->GetInfoString()) : std::string()));

        return false;
    }

    return true;
}

// ------------------------------------ //
DLLEXPORT asIScriptContext* Leviathan::ScriptExecutor::_GetContextForExecution()
{
    // TODO: check for recursive call

    Lock guard(ContextPoolLock);

    // Get from pool if possible
    if(!ContextPool.empty()) {
        auto* ptr = ContextPool.back();
        ContextPool.pop_back();
        return ptr;
    }

    // Needs more contexts
    asIScriptContext* scriptContext = engine->CreateContext();

    if(!scriptContext) {

        LOG_ERROR("ScriptExecutor: _GetContextForExecution: Failed to create a new context");
        return nullptr;
    }

    return scriptContext;
}

DLLEXPORT void Leviathan::ScriptExecutor::_DoneWithContext(asIScriptContext* context)
{
    Lock guard(ContextPoolLock);

    // Only keep 1000 contexts at most
    if(ContextPool.size() < 1000) {

        ContextPool.push_back(context);
        context->Unprepare();

    } else {

        context->Release();
    }
}
// ------------------------------------ //
DLLEXPORT std::weak_ptr<ScriptModule> Leviathan::ScriptExecutor::GetModule(const int& ID)
{
    // loop modules and return a ptr to matching id //
    Lock lock(ModulesLock);

    for(size_t i = 0; i < AllocatedScriptModules.size(); i++) {
        if(AllocatedScriptModules[i]->GetID() == ID)
            return AllocatedScriptModules[i];
    }

    return std::shared_ptr<ScriptModule>(nullptr);
}

DLLEXPORT std::weak_ptr<ScriptModule> Leviathan::ScriptExecutor::GetModuleByAngelScriptName(
    const char* nameofmodule)
{
    // Find a matching name //
    std::string module(nameofmodule);

    Lock lock(ModulesLock);

    // TODO: check could this be checked by comparing pointers
    for(size_t i = 0; i < AllocatedScriptModules.size(); i++) {
        if(AllocatedScriptModules[i]->GetModuleName() == module)
            return AllocatedScriptModules[i];
    }

    return std::shared_ptr<ScriptModule>(nullptr);
}
// ------------------------------------ //
DLLEXPORT std::weak_ptr<ScriptModule> Leviathan::ScriptExecutor::CreateNewModule(
    const std::string& name, const std::string& source, const int& modulesid
    /*= IDFactory::GetID()*/)
{
    // create new module to a smart pointer //
    auto tmpptr = std::make_shared<ScriptModule>(engine, name, modulesid, source);

    // add to vector and return //
    Lock lock(ModulesLock);
    AllocatedScriptModules.push_back(tmpptr);
    return tmpptr;
}

DLLEXPORT void Leviathan::ScriptExecutor::DeleteModule(ScriptModule* ptrtomatch)
{

    Lock lock(ModulesLock);

    // find module based on pointer and remove //
    for(size_t i = 0; i < AllocatedScriptModules.size(); i++) {
        if(AllocatedScriptModules[i].get() == ptrtomatch) {

            AllocatedScriptModules[i]->Release();
            // remove //
            AllocatedScriptModules.erase(AllocatedScriptModules.begin() + i);
            return;
        }
    }
}

DLLEXPORT bool Leviathan::ScriptExecutor::DeleteModuleIfNoExternalReferences(int ID)
{

    Lock lock(ModulesLock);

    // Find based on the id //
    for(size_t i = 0; i < AllocatedScriptModules.size(); i++) {
        if(AllocatedScriptModules[i]->GetID() == ID) {
            // Check reference count //
            if(AllocatedScriptModules[i].use_count() != 1) {
                // Other references exist //
                return false;
            }

            AllocatedScriptModules[i]->Release();

            // remove //
            AllocatedScriptModules.erase(AllocatedScriptModules.begin() + i);
            return true;
        }
    }
    // Nothing found //
    return false;
}
// ------------------------------------ //
DLLEXPORT bool ScriptExecutor::_DoPassParameterTypeError(
    ScriptRunningSetup& setup, ScriptModule* module, int i, int scriptwanted, int provided)
{
    if(setup.PrintErrors) {

        LOG_ERROR("ScriptExecutor: pass parameters to script failed, func: " +
                  setup.Entryfunction + " param number: " + std::to_string(i) +
                  " script wanted type: " + std::to_string(scriptwanted) +
                  " but application provided: " + std::to_string(provided) +
                  (module ? (" in: " + module->GetInfoString()) : std::string()));
    }

    return false;
}

DLLEXPORT void ScriptExecutor::_DoReceiveParameterTypeError(
    ScriptRunningSetup& setup, ScriptModule* module, int applicationwanted, int scripthad)
{
    if(setup.PrintErrors) {

        LOG_ERROR("ScriptExecutor: return parameter from script failed, func: " +
                  setup.Entryfunction +
                  " application wanted type: " + std::to_string(applicationwanted) +
                  " but script return type is: " + std::to_string(scripthad) +
                  (module ? (" in: " + module->GetInfoString()) : std::string()));
    }
}
// ------------------------------------ //
DLLEXPORT void ScriptExecutor::PrintExceptionInfo(asIScriptContext* ctx,
    LErrorReporter& output, asIScriptFunction* func /*= nullptr*/,
    ScriptModule* scriptmodule /*= nullptr*/)
{

    std::string declaration = ctx->GetExceptionFunction()->GetDeclaration() ?
                                  ctx->GetExceptionFunction()->GetDeclaration() :
                                  "unknown function";

    std::string section = ctx->GetExceptionFunction()->GetScriptSectionName() ?
                              ctx->GetExceptionFunction()->GetScriptSectionName() :
                              "unknown";

    std::string exception =
        ctx->GetExceptionString() ? ctx->GetExceptionString() : "unknown exception";

    std::string funcDeclaration =
        func ? (func->GetDeclaration() ? func->GetDeclaration() : "unknown function") : "";

    output.Error(
        std::string("[SCRIPT][EXCEPTION] ") + exception +
        (func ? std::string(", while running function: ") + funcDeclaration : std::string()) +
        "\n\t in function " + declaration + " defined in " + section + "(" +
        std::to_string(ctx->GetExceptionLineNumber()) + ") " +
        (scriptmodule ? scriptmodule->GetInfoString() : std::string()));

    PrintCallstack(ctx, output);
}

DLLEXPORT void ScriptExecutor::PrintCallstack(asIScriptContext* ctx, LErrorReporter& output)
{

    // Print callstack as additional information //
    output.WriteLine("// ------------------ CallStack ------------------ //");

    // Loop the stack starting from the frame below the current function
    // (actually might be nice to print the top frame too)
    for(asUINT n = 0; n < ctx->GetCallstackSize(); n++) {

        // Get the function object //
        const asIScriptFunction* function = ctx->GetFunction(n);

        // If the function doesn't exist this frame is used internally by the script engine //
        if(function) {

            // Check function type //
            if(function->GetFuncType() == asFUNC_SCRIPT) {

                // Print info about the script function //
                output.WriteLine(std::string("\t> ") + function->GetScriptSectionName() + ":" +
                                 std::to_string(ctx->GetLineNumber(n)) + " " +
                                 function->GetDeclaration());

            } else {
                // Info about the application functions //
                // The context is being reused by the application for a nested call
                output.WriteLine(
                    std::string("\t> {...Application...}: ") + function->GetDeclaration());
            }
        } else {
            // The context is being reused by the script engine for a nested call
            output.WriteLine("\t> {...Script internal...}");
        }
    }
}
// ------------------------------------ //
DLLEXPORT int ScriptExecutor::ResolveStringToASID(
    const char* str, bool constversion /*= false*/) const
{
    if(!constversion) {
        return engine->GetTypeIdByDecl(str);
    } else {
        // TODO: it would be nice to not have to allocate memory here
        return engine->GetTypeIdByDecl((std::string("const ") + str).c_str());
    }
}

DLLEXPORT asITypeInfo* ScriptExecutor::GetTypeInfo(int type) const
{
    if(type < 0)
        return nullptr;

    return engine->GetTypeInfoById(type);
}
// ------------------------------------ //
DLLEXPORT void ScriptExecutor::CollectGarbage()
{
    engine->GarbageCollect(asGC_FULL_CYCLE);
}
// ------------------------------------ //
// CustomScriptRun
DLLEXPORT CustomScriptRun::~CustomScriptRun()
{
    if(Context) {

        Exec->_DoneWithContext(Context);
    }
}
