// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Events/DelegateSlot.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptRunningSetup.h"

#include "angelscript.h"

namespace Leviathan{
namespace Script{

class ScriptDelegateSlot final : public BaseDelegateSlot{
public:
    ScriptDelegateSlot(asIScriptFunction* callback) : Callback(callback){

    }

    ~ScriptDelegateSlot(){

        Callback->Release();
    }
    
    void OnCalled(const NamedVars::pointer &values) override{

        ScriptRunningSetup ssetup;

        auto result = ScriptExecutor::Get()->RunScript<void>(Callback, nullptr, ssetup,
            values.get());

        if(result.Result != SCRIPT_RUN_RESULT::Success)
            LOG_WARNING("ScriptDelegateSlot: failed to call callback");
    }

private:

    asIScriptFunction* Callback;
};



}
}
