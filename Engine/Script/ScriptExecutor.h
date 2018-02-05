// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/DataBlock.h"
#include "Common/ThreadSafe.h"
#include "Handlers/IDFactory.h"
#include "Script/FunctionParameterInfo.h"
#include "Script/ScriptRunningSetup.h"
#include "Script/ScriptScript.h"


#include <memory>
#include <string>

// angelscript //
//#define ANGELSCRIPT_DLL_LIBRARY_IMPORT
#include "angelscript.h"



namespace Leviathan {

//! This has to be constant (and luckily so far it has been)
constexpr auto ANGELSCRIPT_VOID_TYPEID = 0;

//! Helper for querying each type for their corresponding angelscript type once
template<class T>
class AngelScriptTypeIDResolver {
public:
    static int Get(ScriptExecutor* resolver);
};

#define TYPE_RESOLVER_AS_PREDEFINED(x, astype)                         \
    template<>                                                         \
    class AngelScriptTypeIDResolver<x> {                               \
        static int Get(ScriptExecutor* resolver)                       \
        {                                                              \
            static int cached = resolver->ResolveStringToASID(astype); \
            return cached;                                             \
        }                                                              \
    };


//! \todo Make the module finding more efficient, store module IDs in all call sites
class ScriptExecutor {
    friend ScriptModule;

public:
    DLLEXPORT ScriptExecutor();
    DLLEXPORT ~ScriptExecutor();

    DLLEXPORT void ScanAngelScriptTypes();

    // module managing //
    DLLEXPORT std::weak_ptr<ScriptModule> CreateNewModule(const std::string& name,
        const std::string& source, const int& modulesid = IDFactory::GetID());

    DLLEXPORT void DeleteModule(ScriptModule* ptrtomatch);
    DLLEXPORT bool DeleteModuleIfNoExternalReferences(int ID);
    DLLEXPORT std::weak_ptr<ScriptModule> GetModule(const int& ID);
    DLLEXPORT std::weak_ptr<ScriptModule> GetModuleByAngelScriptName(const char* nameofmodule);

    DLLEXPORT inline asIScriptEngine* GetASEngine()
    {
        return engine;
    }

    //! \deprecated use AngelScriptTypeIDResolver (or ResolveStringToASID)
    DLLEXPORT int GetAngelScriptTypeID(const std::string& typesname);


    // //! \brief Runs a script
    // DLLEXPORT std::shared_ptr<VariableBlock> RunSetUp(
    //     ScriptScript* scriptobject, ScriptRunningSetup* parameters);

    //! \brief Finds release ref behaviour on object and calls it
    //! \exception Exception if it didn't work
    //! \note Should only be called if the object of type has asOBJ_REF and no asOBJ_NOCOUNT
    void RunReleaseRefOnObject(void* obj, int objid);

    //! \brief Runs a script
    DLLEXPORT std::shared_ptr<VariableBlock> RunSetUp(
        ScriptModule* scrptmodule, ScriptRunningSetup* parameters);

    //! \brief Runs a script function whose pointer is passed in
    DLLEXPORT std::shared_ptr<VariableBlock> RunSetUp(
        asIScriptFunction* function, ScriptRunningSetup* parameters);


    //! \brief Runs a function in a script
    //! \note This is the recommended way to run scripts (other than GameModule that has its
    //! own method)
    //! \todo Allow recursive calls and more context reuse
    template<typename ReturnT, class... Args>
    DLLEXPORT ScriptRunResult<ReturnT> RunScript(
        ScriptModule* module, ScriptRunningSetup& parameters, Args&&... args)
    {
        asIScriptFunction* func = GetFunctionFromModule(module, parameters);

        if(!func)
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);

        // Create a running context for the function //
        asIScriptContext* scriptContext = _GetContextForExecution();

        if(!scriptContext) {
            // Should this be fatal?
            LOG_ERROR("ScriptExecutor: RunScript: failed to create a new context");
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        if(!_PrepareContextForPassingParameters(func, scriptContext, &parameters, module)) {

            _DoneWithContext(scriptContext);
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        // Pass the parameters //
        // TODO: Needs a new method for this
        if(!_PassParametersToScript(
               scriptContext, parameters, module, std::forward<Args>(args)...)) {

            // Failed passing the parameters //
            _DoneWithContext(scriptContext);
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        // Run the script //
        // TODO: timeout and debugging registering with linecallbacks here //
        int retcode = scriptContext->Execute();

        // Get the return value //
        auto returnvalue = _HandleEndedScriptExecution<ReturnT>(
            retcode, scriptContext, parameters, func, module);

        // Release the context //
        _DoneWithContext(scriptContext);

        // Return the returned value //
        return returnvalue;
    }

    //! \brief Runs a method in a script
    //! \note This doesn't verify that the object type is correct for the function.
    //! The caller is responsible for making sure that func is part of the class of obj
    //! \note The parameters object is largely ignored (function name)
    //! \todo Merge common parts with
    template<typename ReturnT, class... Args>
    DLLEXPORT ScriptRunResult<ReturnT> RunScriptMethod(
        ScriptRunningSetup& parameters, asIScriptFunction* func, void* obj, Args&&... args)
    {
        if(!func || !obj)
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);

        // Create a running context for the function //
        asIScriptContext* scriptContext = _GetContextForExecution();

        if(!scriptContext) {
            // Should this be fatal?
            LOG_ERROR("ScriptExecutor: RunScriptMethod: failed to create a new context");
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        if(!_PrepareContextForPassingParameters(func, scriptContext, &parameters, nullptr)) {

            _DoneWithContext(scriptContext);
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        // Pass the parameters //
        // TODO: Needs a new method for this
        if(!_PassParametersToScript(
               scriptContext, parameters, nullptr, std::forward<Args>(args)...)) {

            // Failed passing the parameters //
            _DoneWithContext(scriptContext);
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        // Run the script //
        // TODO: timeout and debugging registering with linecallbacks here //
        int retcode = scriptContext->Execute();

        // Get the return value //
        auto returnvalue = _HandleEndedScriptExecution<ReturnT>(
            retcode, scriptContext, parameters, func, nullptr);

        // Release the context //
        _DoneWithContext(scriptContext);

        // Return the returned value //
        return returnvalue;
    }

    //! \brief Finds a script function in module matching setup
    //! \returns The function or null if not found or module is invalid
    DLLEXPORT asIScriptFunction* GetFunctionFromModule(
        ScriptModule* module, ScriptRunningSetup& parameters);

    //! \brief Converts a string to angelscript type id. Returns -1 on error
    //!
    //! Replaces GetAngelScriptTypeID
    DLLEXPORT int ResolveStringToASID(const char* str) const;

    //! \brief Prints exception info and stacktrace to a logger
    DLLEXPORT static void PrintExceptionInfo(asIScriptContext* ctx, LErrorReporter& output,
        asIScriptFunction* func = nullptr, ScriptModule* scrptmodule = nullptr);

    DLLEXPORT static void PrintCallstack(asIScriptContext* ctx, LErrorReporter& output);

    DLLEXPORT static ScriptExecutor* Get();

private:
    //! \brief Handles status and returning a value from ran script function
    template<typename ReturnT>
    ScriptRunResult<ReturnT> _HandleEndedScriptExecution(int retcode,
        asIScriptContext* scriptcontext, ScriptRunningSetup& setup, asIScriptFunction* func,
        ScriptModule* module)
    {
        // Check the return type //
        if(retcode != asEXECUTION_FINISHED) {
            // something went wrong //

            // The execution didn't finish as we had planned. Determine why.
            switch(retcode) {
                // script caused an exception //
            case asEXECUTION_EXCEPTION:
                PrintExceptionInfo(scriptcontext, *Logger::Get(),
                    scriptcontext->GetExceptionFunction(), module);
                [[fallthrough]];
            default:
                // code took too long //
            case asEXECUTION_ABORTED:
                return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
            case asEXECUTION_SUSPENDED:
                return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Suspended);
            }
        }

        // Successfully executed, try to fetch return value //

        // TODO: support for value types (also need support for them in the parameter passing)

        const auto returnType = func->GetReturnTypeId();

        // Script didn't return anything
        if(returnType == ANGELSCRIPT_VOID_TYPEID) {

            // Set the return value to default if it isn't void //
            if constexpr(std::is_void_v<ReturnT>) {
                return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Success);
            } else {

                if(setup.PrintErrors) {

                    LOG_ERROR("ScriptExecutor: script return value is void, but application "
                              "expected a value of type: " +
                              std::string(typeid(ReturnT).name()));
                }

                // Rely on 0 being a valid value for pointer etc.
                return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Success, 0);
            }
        }

        // return type isn't void //
        // TODO: conversions from compatible types

        const auto parameterType = AngelScriptTypeIDResolver<ReturnT>::Get(this);
        if(returnType != parameterType) {

            _DoReceiveParameterTypeError(setup, module, parameterType, returnType);

            asITypeInfo* info = GetASEngine()->GetTypeInfoById(func->GetReturnTypeId());

            const auto flags = info->GetFlags();
            if((flags & asOBJ_REF) && !(flags & asOBJ_NOCOUNT)) {

                LOG_ERROR(
                    "ScriptExecutor: script returned an object of ref type but "
                    "the application wasn't expecting that type. Trying to prevent memory "
                    "leak my calling release ref through angelscript");

                RunReleaseRefOnObject(
                    scriptcontext->GetReturnObject(), func->GetReturnTypeId());
            }

            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }


        if constexpr(std::is_same_v<ReturnT, int32_t>) {

            return ScriptRunResult<ReturnT>(
                SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnDWord());
        } else if constexpr(std::is_same_v<ReturnT, float>) {

            return ScriptRunResult<ReturnT>(
                SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnFloat());
        } else if constexpr(std::is_same_v<ReturnT, double>) {

            return ScriptRunResult<ReturnT>(
                SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnDouble());
        } else if constexpr(std::is_same_v<ReturnT, char> || std::is_same_v<ReturnT, int8_t> ||
                            std::is_same_v<ReturnT, bool>) {

            return ScriptRunResult<ReturnT>(
                SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnByte());

        } else {

            // This is a class type and we need to do a copy if it was
            // by value or this isn't a handle type

            // According to AS documentation the return object is
            // deleted when the context is recycled, so we need to
            // increase ref or make a copy

            // We have already done type checks, so this should be fine to cast //
            ReturnT* obj = static_cast<ReturnT*>(scriptcontext->GetReturnObject());

            if constexpr(std::is_pointer_v<ReturnT>) {

                _IncrementRefCountIfRefCountedType(obj);
                return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Success, obj);

            } else if constexpr(std::is_lvalue_reference_v<ReturnT>) {

                static_assert(!std::is_class_v<ReturnT>,
                    "Returning by reference from scripts doesn't work");

            } else if constexpr(std::is_class_v<ReturnT>) {

                // TODO: this needs testing and implementing passing by value INTO scripts
                return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Success, *obj);
            } else {

                static_assert(std::is_same_v<ReturnT, void> == std::is_same_v<ReturnT, int>,
                    "Tried to return some very weird type from a script function");
            }
        }
    }

    //! \brief Handles passing parameters to scripts
    //!
    //! Automatically ignores extra parameters that the script didn't want to take.
    //! Also automatically increases reference counts of objects that are derived from
    //! ReferenceCounted
    //! \returns False if passing parameters failed and the script shouldn't be attempted
    //! to be ran
    template<class... Args>
    bool _PassParametersToScript(asIScriptContext* scriptcontext, ScriptRunningSetup& setup,
        ScriptModule* module, asIScriptFunction* func, Args&&... args)
    {
        // Get the number of parameters expected //
        auto parameterc = func->GetParamCount();
        asUINT i = 0;

        // Start passing the parameters provided by the application //
        if(!_DoPassEachParameter(
               parameterc, i, scriptcontext, func, std::forward<Args>(args)...))
            return false;

        // Check that we passed enough parameters for the script (we
        // don't care if the script took less parameters than we gave
        // it)
        if(i < parameterc) {
            // We didn't have enough parameters
            if(setup.PrintErrors) {
                LOG_ERROR("ScriptExecutor: not enough parameters to pass to script function");
            }

            return false;
        }

        return true;
    }

    //! Helper for _PassParametersToScript
    template<class CurrentT, class... Args>
    bool _DoPassEachParameter(asUINT parameterc, asUINT& i, asIScriptContext* scriptcontext,
        ScriptRunningSetup& setup, ScriptModule* module, asIScriptFunction* func,
        const CurrentT& current, Args&&... args)
    {
        // Finished passing parameters //
        if(i >= parameterc)
            return true;

        // Try to pass based on type //
        int r = 0;

        // Check that type matches //
        int wantedTypeID;
        if(func->GetParam(i, &wantedTypeID) < 0) {

            LOG_ERROR("ScriptExecutor: failed to get param type from as: " +
                      std::to_string(i) + ", for func: " + func->GetName());
            return false;
        }

        // TODO: conversions from compatible types
        const auto parameterType = AngelScriptTypeIDResolver<CurrentT>::Get(this);
        if(wantedTypeID != parameterType)
            return _DoPassParameterTypeError(setup, module, i, wantedTypeID, parameterType);

        if constexpr(std::is_same_v<CurrentT, int32_t>) {

            r = scriptcontext->SetArgDWord(i, current);
        } else if constexpr(std::is_same_v<CurrentT, float>) {

            r = scriptcontext->SetArgFloat(i, current);
        } else if constexpr(std::is_same_v<CurrentT, double>) {

            r = scriptcontext->SetArgDouble(i, current);
        } else if constexpr(std::is_same_v<CurrentT, char> ||
                            std::is_same_v<CurrentT, int8_t> ||
                            std::is_same_v<CurrentT, bool>) {

            r = scriptcontext->SetArgByte(i, current);

            /// Error types ///
        } else if constexpr(std::is_same_v<CurrentT, std::wstring>) {

            static_assert(!std::is_same_v<CurrentT, std::wstring>,
                "wstring would have to be saved as a string that can then be passed");

        } else {
            // Non-primitive type //

            // Checks for pointers and references to things with type id verification //
            if constexpr(std::is_pointer_v<CurrentT>) {

                _IncrementRefCountIfRefCountedType(current);

                r = scriptcontext->SetArgObject(i, current);
            } else if constexpr(std::is_lvalue_reference_v<CurrentT>) {

                _IncrementRefCountIfRefCountedType(&current);

                r = scriptcontext->SetArgObject(i, &current);
            } else if constexpr(std::is_class_v<CurrentT>) {

                static_assert(!std::is_class_v<CurrentT>,
                    "Passing objects by value to scripts is not implemented, TODO: "
                    "implement this");
            } else {

                static_assert(std::is_same_v<CurrentT, void> == std::is_same_v<CurrentT, int>,
                    "Tried to pass some very weird type to a script function");
            }
        }

        // Move to next parameter for the next recursive call //
        ++i;

        // Error check //
        if(r < 0) {

            return false;
        }

        // Call other parameters //
        return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module, func,
            std::forward<Args>(args)...);
    }

    // End condition for the variadic template
    bool _DoPassEachParameter(asUINT parameterc, asUINT& i)
    {
        return true;
    }

    //! Helper for type errors in _DoPassEachParameter
    //! \returns False so that "return _DoPassParameterTypeError(...)" can be used by callers
    DLLEXPORT bool _DoPassParameterTypeError(ScriptRunningSetup& setup, ScriptModule* module,
        int i, int scriptwanted, int provided);

    //! \brief Increments refcount of obj if it is derived from ReferenceCounted
    //! Helper _DoPassEachParameter
    template<class T>
    void _IncrementRefCountIfRefCountedType(T* current)
    {
        if constexpr(std::is_base_of_v<ReferenceCounted, T>) {
            if(current)
                current->AddRef();
        }
    }


    // ------------------------------------ //

    //! \brief Handles the return type and return value of a function
    //! \todo Return a tuple with an enum for checking for error conditions
    std::shared_ptr<VariableBlock> _GetScriptReturnedVariable(int retcode,
        asIScriptContext* ScriptContext, ScriptRunningSetup* parameters,
        asIScriptFunction* func, ScriptModule* scrptmodule, FunctionParameterInfo* paraminfo);

    //! \brief Handles passing parameters to a context
    bool _SetScriptParameters(asIScriptContext* ScriptContext, ScriptRunningSetup* parameters,
        ScriptModule* scrptmodule, FunctionParameterInfo* paraminfo);

    //! \brief Checks whether a function is a valid pointer
    bool _CheckScriptFunctionPtr(
        asIScriptFunction* func, ScriptRunningSetup* parameters, ScriptModule* scrptmodule);

    //! \brief Prepares a context for usage
    bool _PrepareContextForPassingParameters(asIScriptFunction* func,
        asIScriptContext* ScriptContext, ScriptRunningSetup* parameters,
        ScriptModule* scrptmodule);

    //! \brief Called when a context is required for script execution
    //! \todo Add a pool from which these are retrieved
    asIScriptContext* _GetContextForExecution();

    //! \brief Called after a script has been executed and the context is no longer needed
    void _DoneWithContext(asIScriptContext* context);

    // ------------------------------ //
    // AngelScript engine script executing part //
    asIScriptEngine* engine;

    // list of modules that have been created, some might only have this as reference, and
    // could potentially be released
    std::vector<std::shared_ptr<ScriptModule>> AllocatedScriptModules;

    Mutex ModulesLock;


    // map of type name and engine type id //
    static std::map<int, std::string> EngineTypeIDS;

    // inverted of the former for better performance //
    static std::map<std::string, int> EngineTypeIDSInverted;

    static ScriptExecutor* instance;
}; // namespace Leviathan

template<class T>
int AngelScriptTypeIDResolver<T>::Get(ScriptExecutor* resolver)
{
    static int cached = resolver->ResolveStringToASID(T::ANGELSCRIPT_TYPE);
    return cached;
}

// Special cases of AngelScriptTypeIDResolver for fundamental types //
TYPE_RESOLVER_AS_PREDEFINED(int, "int");
TYPE_RESOLVER_AS_PREDEFINED(int64_t, "int64");
TYPE_RESOLVER_AS_PREDEFINED(uint64_t, "uint64");
TYPE_RESOLVER_AS_PREDEFINED(int16_t, "int16");
TYPE_RESOLVER_AS_PREDEFINED(uint16_t, "uint16");
TYPE_RESOLVER_AS_PREDEFINED(int8_t, "int8");
TYPE_RESOLVER_AS_PREDEFINED(uint8_t, "uint8");
TYPE_RESOLVER_AS_PREDEFINED(double, "double");
TYPE_RESOLVER_AS_PREDEFINED(float, "float");

// And other inbuild types that can't have ANGELSCRIPT_TYPE in their class
TYPE_RESOLVER_AS_PREDEFINED(std::string, "string");


} // namespace Leviathan
