// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
//! \file Specifies helpers for passing parameters to CustomScriptRun
// ------------------------------------ //
#include "ScriptCallingHelpers.h"
#include "ScriptExecutor.h"

namespace Leviathan {

inline bool PassParameterToCustomRun(std::unique_ptr<CustomScriptRun>& run, uint32_t value)
{
    if(run->Context->SetArgDWord(run->PassedIndex, value) < 0)
        return false;

    ++run->PassedIndex;
    return true;
}

//! \brief Passes an object type that has already been resolved
//! \note Does a basic type id check
inline bool PassParameterToCustomRun(
    std::unique_ptr<CustomScriptRun>& run, void* value, int valuetypeid)
{
    int paramTypeId;
    run->Func->GetParam(run->PassedIndex, &paramTypeId);

    if(paramTypeId != valuetypeid) {

        LOG_ERROR("PassParameterToCustomRun: type id mismatch (function) " +
                  std::to_string(paramTypeId) + " != " + std::to_string(valuetypeid) +
                  " (passed)");
        return false;
    }

    if(run->Context->SetArgObject(run->PassedIndex, value) < 0)
        return false;

    ++run->PassedIndex;
    return true;
}

//! \brief Passes a script object type
inline bool PassParameterToCustomRun(
    std::unique_ptr<CustomScriptRun>& run, asIScriptObject* value)
{
    int paramTypeId;
    run->Func->GetParam(run->PassedIndex, &paramTypeId);
    int valuetypeid = value->GetTypeId();

    // We can safely ignore if the wanted type is a handle and the
    // value isn't (it just works with script classes)
    if((paramTypeId != value->GetTypeId()) &&
        ((paramTypeId ^ asTYPEID_OBJHANDLE) != valuetypeid)) {

        asIScriptEngine* engine = run->Context->GetEngine();

        LOG_ERROR("PassParameterToCustomRun: type id mismatch for script class (function) " +
                  std::string(engine->GetTypeInfoById(paramTypeId)->GetName()) + " " +
                  std::to_string(paramTypeId) +
                  " != " + std::string(engine->GetTypeInfoById(valuetypeid)->GetName()) + " " +
                  std::to_string(valuetypeid) + " (passed)");
        return false;
    }

    if(run->Context->SetArgObject(run->PassedIndex, value) < 0)
        return false;

    ++run->PassedIndex;
    return true;
}


} // namespace Leviathan
