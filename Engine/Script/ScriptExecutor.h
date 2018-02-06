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

//! Needed to keep this mess in one file as ScriptExecutor depends on AngelScriptTypeIDResolver
//! and that depends on it
DLLEXPORT int ResolveProxy(const char* type, ScriptExecutor* resolver);

//! Helper for querying each type for their corresponding angelscript type once
template<class T>
struct AngelScriptTypeIDResolver {
public:
    static int Get(ScriptExecutor* resolver)
    {
        if constexpr(std::is_pointer_v<T>) {
            static int cached =
                ResolveProxy(std::remove_pointer_t<T>::ANGELSCRIPT_TYPE, resolver);
            return cached;
        } else {
            static int cached = ResolveProxy(T::ANGELSCRIPT_TYPE, resolver);
            return cached;
        }
    }
};

#define TYPE_RESOLVER_AS_PREDEFINED(x, astype)                  \
    template<>                                                  \
    struct AngelScriptTypeIDResolver<x> {                       \
        static int Get(ScriptExecutor* resolver)                \
        {                                                       \
            static int cached = ResolveProxy(astype, resolver); \
            return cached;                                      \
        }                                                       \
    };

// Special cases of AngelScriptTypeIDResolver for fundamental types //
TYPE_RESOLVER_AS_PREDEFINED(int, "int");
TYPE_RESOLVER_AS_PREDEFINED(unsigned int, "uint");
TYPE_RESOLVER_AS_PREDEFINED(int64_t, "int64");
TYPE_RESOLVER_AS_PREDEFINED(uint64_t, "uint64");
TYPE_RESOLVER_AS_PREDEFINED(int16_t, "int16");
TYPE_RESOLVER_AS_PREDEFINED(uint16_t, "uint16");
TYPE_RESOLVER_AS_PREDEFINED(int8_t, "int8");
TYPE_RESOLVER_AS_PREDEFINED(uint8_t, "uint8");
TYPE_RESOLVER_AS_PREDEFINED(double, "double");
TYPE_RESOLVER_AS_PREDEFINED(float, "float");

// And other inbuilt types that can't have ANGELSCRIPT_TYPE in their class
TYPE_RESOLVER_AS_PREDEFINED(std::string, "string");

// Special case void //
template<>
struct AngelScriptTypeIDResolver<void> {
    static int Get(ScriptExecutor* resolver)
    {
        return ANGELSCRIPT_VOID_TYPEID;
    }
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
    //! \todo Allow recursive calls and more context reuse. Also wrap context in an object that
    //! automatically returns it in case of expections (_HandleEndedScriptExecution can throw)
    template<typename ReturnT, class... Args>
    DLLEXPORT ScriptRunResult<ReturnT> RunScript(const std::shared_ptr<ScriptModule>& module,
        ScriptRunningSetup& parameters, Args&&... args)
    {
        if(!parameters.Parameters.empty())
            LOG_FATAL("RunScript: called with old style parameters. Move parameters to the "
                      "function call");

        asIScriptFunction* func = GetFunctionFromModule(module.get(), parameters);

        if(!func)
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);

        // Create a running context for the function //
        asIScriptContext* scriptContext = _GetContextForExecution();

        if(!scriptContext) {
            // Should this be fatal?
            LOG_ERROR("ScriptExecutor: RunScript: failed to create a new context");
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        if(!_PrepareContextForPassingParameters(
               func, scriptContext, &parameters, module.get())) {

            _DoneWithContext(scriptContext);
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        // Pass the parameters //
        if(!_PassParametersToScript(
               scriptContext, parameters, module.get(), func, std::forward<Args>(args)...)) {

            // Failed passing the parameters //
            _DoneWithContext(scriptContext);
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        // Run the script //
        // TODO: timeout and debugging registering with linecallbacks here //
        int retcode = scriptContext->Execute();

        // Get the return value //
        auto returnvalue = _HandleEndedScriptExecution<ReturnT>(
            retcode, scriptContext, parameters, func, module.get());

        // Release the context //
        _DoneWithContext(scriptContext);

        // Return the returned value //
        return returnvalue;
    }

    //! \brief Runs a method in a script
    //! \note This doesn't verify that the object type is correct for the function.
    //! The caller is responsible for making sure that func is part of the class of obj
    //! \note The parameters object is largely ignored (function name)
    //! \todo Merge common parts with RunScript
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
        if(!_PassParametersToScript(
               scriptContext, parameters, nullptr, func, std::forward<Args>(args)...)) {

            // Failed passing the parameters //
            _DoneWithContext(scriptContext);
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
        }

        // Pass the object instance //
        if(scriptContext->SetObject(obj) < 0) {

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
    //! \todo When receiving signed types do we need to reinterpret_cast them from unsigned
    //! types
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

        // script return type isn't void //

        const auto parameterType = AngelScriptTypeIDResolver<ReturnT>::Get(this);

        if constexpr(std::is_same_v<ReturnT, void>) {

            // Success, no return value wanted //
            if(setup.PrintErrors && (returnType != parameterType)) {

                LOG_WARNING("ScriptExecutor: application ignoring script return value");
            }

            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Success);

        } else {
            // TODO: conversions between compatible types
            if(returnType != parameterType) {

                return _ReturnedTypeDidntMatch<ReturnT>(
                    scriptcontext, setup, func, module, parameterType, returnType);
            }

            if constexpr(std::is_same_v<ReturnT, int32_t> ||
                         std::is_same_v<ReturnT, uint32_t>) {

                return ScriptRunResult<ReturnT>(
                    SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnDWord());
            } else if constexpr(std::is_same_v<ReturnT, int64_t> ||
                                std::is_same_v<ReturnT, uint64_t>) {

                return ScriptRunResult<ReturnT>(
                    SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnQWord());
            } else if constexpr(std::is_same_v<ReturnT, float>) {

                return ScriptRunResult<ReturnT>(
                    SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnFloat());
            } else if constexpr(std::is_same_v<ReturnT, double>) {

                return ScriptRunResult<ReturnT>(
                    SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnDouble());
            } else if constexpr(std::is_same_v<ReturnT, char> ||
                                std::is_same_v<ReturnT, int8_t> ||
                                std::is_same_v<ReturnT, bool>) {

                return ScriptRunResult<ReturnT>(
                    SCRIPT_RUN_RESULT::Success, scriptcontext->GetReturnByte());
            } else {

                // This is a class type and we need to do a copy if it was
                // by value or this isn't a handle type

                // According to AS documentation the return object is
                // deleted when the context is recycled, so we need to
                // increase ref or make a copy

                if constexpr(std::is_pointer_v<ReturnT>) {

                    // We have already done type checks, so this should be fine to cast //
                    ReturnT obj = static_cast<ReturnT>(scriptcontext->GetReturnObject());

                    _IncrementRefCountIfRefCountedType(obj);
                    return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Success, ReturnT(obj));

                } else if constexpr(std::is_lvalue_reference_v<ReturnT>) {

                    static_assert(!std::is_class_v<ReturnT>,
                        "Returning by reference from scripts doesn't work");

                } else if constexpr(std::is_class_v<ReturnT>) {

                    // We have already done type checks, so this should be fine to cast //
                    ReturnT* obj = static_cast<ReturnT*>(scriptcontext->GetReturnObject());
                    return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Success, ReturnT(*obj));
                } else {

                    static_assert(
                        std::is_same_v<ReturnT, void> == std::is_same_v<ReturnT, int>,
                        "Tried to return some very weird type from a script function");
                }
            }
        }
    }

    //! Helper for releasing unused handle types in _HandleEndedScriptExecution
    template<typename ReturnT>
    ScriptRunResult<ReturnT> _ReturnedTypeDidntMatch(asIScriptContext* scriptcontext,
        ScriptRunningSetup& setup, asIScriptFunction* func, ScriptModule* module,
        int parameterType, int returnType)
    {
        _DoReceiveParameterTypeError(setup, module, parameterType, returnType);
        return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);
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
               parameterc, i, scriptcontext, setup, module, func, std::forward<Args>(args)...))
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
    //! \todo When passing signed types do we need to reinterpret_cast them to unsigned types
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
        asDWORD flags;
        if(func->GetParam(i, &wantedTypeID, &flags) < 0) {

            LOG_ERROR("ScriptExecutor: failed to get param type from as: " +
                      std::to_string(i) + ", for func: " + func->GetName());
            return false;
        }

        const auto parameterType = AngelScriptTypeIDResolver<CurrentT>::Get(this);
        if(wantedTypeID != parameterType) {

            // Compatibility checks //
            // This is not the most optimal as this results in a duplicate call to
            // func->GetParam
            // TODO: is there a better way than to have this mess here?
            if(wantedTypeID == AngelScriptTypeIDResolver<int32_t>::Get(this)) {

                if constexpr(std::is_convertible_v<CurrentT, int32_t>) {
                    return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
                        func, static_cast<int32_t>(current), std::forward<Args>(args)...);
                }
            } else if(wantedTypeID == AngelScriptTypeIDResolver<uint32_t>::Get(this)) {

                if constexpr(std::is_convertible_v<CurrentT, uint32_t>) {
                    return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
                        func, static_cast<uint32_t>(current), std::forward<Args>(args)...);
                }
            } else if(wantedTypeID == AngelScriptTypeIDResolver<uint64_t>::Get(this)) {

                if constexpr(std::is_convertible_v<CurrentT, uint64_t>) {
                    return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
                        func, static_cast<uint64_t>(current), std::forward<Args>(args)...);
                }
            } else if(wantedTypeID == AngelScriptTypeIDResolver<int64_t>::Get(this)) {

                if constexpr(std::is_convertible_v<CurrentT, int64_t>) {
                    return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
                        func, static_cast<int64_t>(current), std::forward<Args>(args)...);
                }
            } else if(wantedTypeID == AngelScriptTypeIDResolver<int8_t>::Get(this)) {

                if constexpr(std::is_convertible_v<CurrentT, int8_t>) {
                    return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
                        func, static_cast<int8_t>(current), std::forward<Args>(args)...);
                }

            } else if(wantedTypeID == AngelScriptTypeIDResolver<uint8_t>::Get(this)) {

                if constexpr(std::is_convertible_v<CurrentT, uint8_t>) {
                    return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
                        func, static_cast<uint8_t>(current), std::forward<Args>(args)...);
                }
            } else if(wantedTypeID == AngelScriptTypeIDResolver<float>::Get(this)) {

                if constexpr(std::is_convertible_v<CurrentT, float>) {
                    return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
                        func, static_cast<float>(current), std::forward<Args>(args)...);
                }

            } else if(wantedTypeID == AngelScriptTypeIDResolver<double>::Get(this)) {

                if constexpr(std::is_convertible_v<CurrentT, double>) {
                    return _DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
                        func, static_cast<double>(current), std::forward<Args>(args)...);
                }
            }

            // No conversion possible //
            return _DoPassParameterTypeError(setup, module, i, wantedTypeID, parameterType);
        }

        if constexpr(std::is_same_v<CurrentT, int32_t> || std::is_same_v<CurrentT, uint32_t>) {

            r = scriptcontext->SetArgDWord(i, current);
        } else if constexpr(std::is_same_v<CurrentT, int64_t> ||
                            std::is_same_v<CurrentT, uint64_t>) {

            r = scriptcontext->SetArgQWord(i, current);
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

                r = scriptcontext->SetArgAddress(i, current);
            } else if constexpr(std::is_lvalue_reference_v<CurrentT>) {

                _IncrementRefCountIfRefCountedType(&current);

                r = scriptcontext->SetArgAddress(i, &current);
            } else if constexpr(std::is_class_v<CurrentT>) {

                // Has to be a class that isn't a handle type
                // So make sure it isn't
                static_assert(!std::is_base_of_v<CurrentT, ReferenceCounted>,
                    "Trying to pass an object of reference type by value to script, call new "
                    "on the argument");

                // If this is by reference, then it must be const
                if((flags & asTM_OUTREF)) {

                    LOG_ERROR(
                        "ScriptExecutor: script wants to take parameter: " +
                        std::to_string(i) +
                        " as an outref which isn't supported, for func: " + func->GetName());
                    return false;

                } else if((flags & asTM_INREF) && !(flags & asTM_CONST)) {

                    LOG_ERROR(
                        "ScriptExecutor: script wants to take parameter: " +
                        std::to_string(i) +
                        " as non-const inref which isn't supported (add const), for func: " +
                        func->GetName());
                    return false;

                } else {

                    r = scriptcontext->SetArgObject(i, const_cast<CurrentT*>(&current));
                }

            } else {

                static_assert(std::is_same_v<CurrentT, void> == std::is_same_v<CurrentT, int>,
                    "Tried to pass some very weird type to a script function");
            }
        }

        // Move to next parameter for the next recursive call //
        ++i;

        // Error check //
        if(r < 0) {
            LOG_ERROR("ScriptExecutor: failed to pass parameter number: " +
                      std::to_string(i - 1) + ", for func: " + func->GetName());
            return false;
        }

        // Call other parameters //
        return _DoPassEachParameter(
            parameterc, i, scriptcontext, setup, module, func, std::forward<Args>(args)...);
    }

    // End condition for the variadic template
    bool _DoPassEachParameter(asUINT parameterc, asUINT& i, asIScriptContext* scriptcontext,
        ScriptRunningSetup& setup, ScriptModule* module, asIScriptFunction* func)
    {
        return true;
    }

    //! Helper for type errors in _DoPassEachParameter
    //! \returns False so that "return _DoPassParameterTypeError(...)" can be used by callers
    //! \todo Switch this and _DoReceiveParameterTypeError to take a asIScriptFunction from
    //! which these take the name of failed function (this fixes cases where setup didn't
    //! specify entry point)
    DLLEXPORT bool _DoPassParameterTypeError(ScriptRunningSetup& setup, ScriptModule* module,
        int i, int scriptwanted, int provided);

    //! Helper for type errors in _HandleEndedScriptExecution
    //! \note Unlike _DoPassParameterTypeError this just prints an error
    DLLEXPORT void _DoReceiveParameterTypeError(
        ScriptRunningSetup& setup, ScriptModule* module, int applicationwanted, int scripthad);



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

    //! Unused helper for calling release on an unkown script type
    //! \param obj The object to release (scriptcontext->GetReturnObject())
    //! \param returnTypeID id of obj (func->GetReturnTypeId())
    void _CallBehaviourReleaseIfNeeded(void* obj, int returnTypeID)
    {
        asITypeInfo* info = GetASEngine()->GetTypeInfoById(returnTypeID);

        const auto flags = info->GetFlags();
        if((flags & asOBJ_REF) && !(flags & asOBJ_NOCOUNT)) {

            LOG_INFO("ScriptExecutor: attempting to release ref through angelscript");

            RunReleaseRefOnObject(obj, returnTypeID);

            LOG_INFO("Success calling behaviour release");
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
};

} // namespace Leviathan
