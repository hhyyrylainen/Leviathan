// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "ScriptCallingHelpers.h"

#include "Common/DataStoring/DataBlock.h"

namespace Leviathan {

enum SCRIPT_RUNTYPE { SCRIPT_RUNTYPE_BREAKONERROR, SCRIPT_RUNTYPE_TRYTOCONTINUE };

enum class SCRIPT_RUN_RESULT { Success, Error, Suspended };

class ScriptRunningSetup {
public:
    DLLEXPORT ScriptRunningSetup();

    //! Set entry point in constructor
    DLLEXPORT ScriptRunningSetup(const std::string& entrypoint);

    DLLEXPORT ~ScriptRunningSetup();

    // named constructor idiom //
    inline ScriptRunningSetup& SetEntrypoint(const std::string& epoint)
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

    inline ScriptRunningSetup& SetUseFullDeclaration(const bool& state)
    {
        // set //
        FullDeclaration = state;
        return *this;
    }

    inline ScriptRunningSetup& SetPrintErrors(const bool& state)
    {

        PrintErrors = state;
        return *this;
    }

    bool PrintErrors = true;
    bool FullDeclaration = false;
    bool ErrorOnNonExistingFunction = true;
    SCRIPT_RUNTYPE RunType = SCRIPT_RUNTYPE_BREAKONERROR;


    bool ScriptExisted = false;

    std::string Entryfunction;
};

//! \brief Holds a result of the new script run method
//! \note If the return type is reference counted this automatically releases it, so
//! if you want to keep the object around you must increase the reference count (and then for
//! safety wrap it into intrusive_ptr)
template<typename ReturnT>
struct ScriptRunResult {

    ScriptRunResult(SCRIPT_RUN_RESULT result, ReturnT&& value) :
        Result(result), Value(std::move(value))
    {
        // We need to take a reference as the script context is reset
        // before we are returned to the caller
        IncreasePointerReference();
    }

    ScriptRunResult(const ScriptRunResult<ReturnT>& other) :
        Result(other.Result), Value(other.Value)
    {
        // We need to take a reference as the other instance will release its pointer
        IncreasePointerReference();
    }

    ScriptRunResult(ScriptRunResult<ReturnT>&& other) :
        Result(other.Result), Value(std::move(other.Value))
    {
    }

    //! Only set result code
    ScriptRunResult(SCRIPT_RUN_RESULT result) : Result(result)
    {
        if constexpr(std::is_pointer_v<ReturnT>)
            Value = nullptr;
    }

    ~ScriptRunResult()
    {
        ReleasePointerReference();
    }

    //! Assign other
    ScriptRunResult& operator=(const ScriptRunResult<ReturnT>& other)
    {
        // Release our old pointer if it was reference counted
        ReleasePointerReference();

        Value = other.Value;
        Result = other.Result;

        // We need to take a reference as the other instance will release its pointer
        IncreasePointerReference();

        return *this;
    }

protected:
    //! Helper for the multiple places that do pointer reference decrease
    void ReleasePointerReference()
    {
        if constexpr(std::is_pointer_v<ReturnT>) {
            DecrementRefCountIfRefCountedType(Value);
        }
    }

    //! Helper for the multiple places that do pointer reference increase
    void IncreasePointerReference()
    {
        if constexpr(std::is_pointer_v<ReturnT>) {

            IncrementRefCountIfRefCountedType(Value);
        }
    }

public:
    //! Result code of the script running
    SCRIPT_RUN_RESULT Result;

    //! Return value received from the script.
    //! Only valid of Result == SCRIPT_RUN_RESULT::Success
    ReturnT Value;
};

template<>
struct ScriptRunResult<void> {

    ScriptRunResult(SCRIPT_RUN_RESULT result) : Result(result) {}

    ScriptRunResult(const ScriptRunResult<void>& other) : Result(other.Result) {}

    //! Result code of the script running
    SCRIPT_RUN_RESULT Result;

    // This is the variant with no wanted return type
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::SCRIPT_RUN_RESULT;
using Leviathan::ScriptRunResult;
using Leviathan::ScriptRunningSetup;
#endif // LEAK_INTO_GLOBAL
