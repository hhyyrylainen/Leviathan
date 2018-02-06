// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/DataBlock.h"

#include "angelscript.h"

#include <type_traits>

namespace Leviathan {

enum SCRIPT_RUNTYPE { SCRIPT_RUNTYPE_BREAKONERROR, SCRIPT_RUNTYPE_TRYTOCONTINUE };

enum class SCRIPT_RUN_RESULT { Success, Error, Suspended };

class ScriptRunningSetup {
public:
    DLLEXPORT ScriptRunningSetup();
    DLLEXPORT ~ScriptRunningSetup();

    // named constructor idiom //
    DLLEXPORT inline ScriptRunningSetup& SetEntrypoint(const std::string& epoint)
    {
        // set //
        Entryfunction = epoint;
        return *this;
    }

    // This is in the script executor
    // //! \param func Pointer to the script function. The caller must
    // //! keep a reference alive to this function as long as this run
    // //! setup is alive
    // DLLEXPORT inline ScriptRunningSetup& SetEntrypoint(asIScriptFunction* func){

    //     PreFetchedFunction = func;
    //     return *this;
    // }

    DLLEXPORT inline ScriptRunningSetup& SetArguments(
        std::vector<std::shared_ptr<NamedVariableBlock>>& args)
    {
        // set //
        Parameters = args;
        return *this;
    }

    DLLEXPORT inline ScriptRunningSetup& SetUseFullDeclaration(const bool& state)
    {
        // set //
        FullDeclaration = state;
        return *this;
    }

    DLLEXPORT inline ScriptRunningSetup& SetPrintErrors(const bool& state)
    {

        PrintErrors = state;
        return *this;
    }


    // variables //
    std::vector<std::shared_ptr<NamedVariableBlock>> Parameters;

    bool PrintErrors;
    bool FullDeclaration;
    bool ErrorOnNonExistingFunction;
    SCRIPT_RUNTYPE RunType;


    bool ScriptExisted;

    std::string Entryfunction;
};

//! \brief Holds a result of the new script run method
//! \note If the return type is reference counted this automatically releases it, so
//! if you want to keep the object around you must increase the reference count (and then for
//! safety wrap it into intrusive_ptr)
template<typename ReturnT>
struct ScriptRunResult {

    DLLEXPORT ScriptRunResult(SCRIPT_RUN_RESULT result, ReturnT&& value) :
        Result(result), Value(std::move(value))
    {
        // We need to take a reference as the script context is reset
        // before we are returned to the caller
        if constexpr(std::is_pointer_v<ReturnT>) {
            if constexpr(std::is_base_of_v<ReferenceCounted, std::remove_pointer_t<ReturnT>>) {
                if(Value)
                    Value->AddRef();
            }
        }
    }

    //! Only set result code
    DLLEXPORT ScriptRunResult(SCRIPT_RUN_RESULT result) : Result(result)
    {
        if constexpr(std::is_pointer_v<ReturnT>)
            Value = nullptr;
    }

    DLLEXPORT ~ScriptRunResult()
    {
        if constexpr(std::is_pointer_v<ReturnT>) {
            if constexpr(std::is_base_of_v<ReferenceCounted, std::remove_pointer_t<ReturnT>>) {
                Value->Release();
            }
        }
    }


    SCRIPT_RUN_RESULT Result;
    ReturnT Value;
};

template<>
struct ScriptRunResult<void> {

    DLLEXPORT ScriptRunResult(SCRIPT_RUN_RESULT result) : Result(result) {}

    SCRIPT_RUN_RESULT Result;
};

} // namespace Leviathan
