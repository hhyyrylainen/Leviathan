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

        std::vector<std::shared_ptr<NamedVariableBlock>> Params = {
            std::make_shared<NamedVariableBlock>(static_cast<void*>(values.get()), "NamedVars")
        };

        values->AddRef();
        
        ScriptRunningSetup ssetup;
        ssetup.SetArguments(Params);

        ScriptExecutor::Get()->RunSetUp(Callback, &ssetup);
    }

private:

    asIScriptFunction* Callback;
};



}
}
