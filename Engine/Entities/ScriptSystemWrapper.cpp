// ------------------------------------ //
#include "ScriptSystemWrapper.h"

#include "GameWorld.h"
#include "Script/ScriptExecutor.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT ScriptSystemWrapper::ScriptSystemWrapper(
    const std::string& name, asIScriptObject* impl) :
    ImplementationObject(impl)
{
    if(!ImplementationObject)
        throw InvalidArgument("ScriptSystemWrapper not given an angelscript object");
}

DLLEXPORT ScriptSystemWrapper::~ScriptSystemWrapper()
{
    if(ImplementationObject) {
        LOG_ERROR("ScriptSystemWrapper: Release has not been called before destructor");
        ImplementationObject->Release();
    }
}
// ------------------------------------ //
DLLEXPORT void ScriptSystemWrapper::Init(GameWorld* world)
{
    asIScriptFunction* func = ImplementationObject->GetObjectType()->GetMethodByName("Init");

    if(!func) {

        LOG_ERROR("Script system: failed to find Init method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, func, ImplementationObject, world);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system: failed to call Init");
        return;
    }
}
DLLEXPORT void ScriptSystemWrapper::Release()
{
    asIScriptFunction* func =
        ImplementationObject->GetObjectType()->GetMethodByName("Release");

    if(!func) {

        LOG_ERROR("Script system: failed to find Release method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, func, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system: failed to call Release");
        return;
    }

    ImplementationObject->Release();
    ImplementationObject = nullptr;
    RunMethod = nullptr;
    CreateAndDestroyNodesMethod = nullptr;
}
// ------------------------------------ //
DLLEXPORT void ScriptSystemWrapper::Run()
{
    // For performance reasons this might be good to store
    if(!RunMethod) {

        RunMethod = ImplementationObject->GetObjectType()->GetMethodByName("Run");
    }

    if(!RunMethod) {

        LOG_ERROR("Script system: failed to find Run method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, RunMethod, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system: failed to call Run");
        return;
    }
}
// ------------------------------------ //
DLLEXPORT void ScriptSystemWrapper::CreateAndDestroyNodes()
{
    // For performance reasons this might be good to store
    if(!CreateAndDestroyNodesMethod) {

        CreateAndDestroyNodesMethod =
            ImplementationObject->GetObjectType()->GetMethodByName("CreateAndDestroyNodes");
    }

    if(!CreateAndDestroyNodesMethod) {

        LOG_ERROR("Script system: failed to find CreateAndDestroyNodes method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, CreateAndDestroyNodesMethod, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system: failed to call CreateAndDestroyNodes");
        return;
    }
}
// ------------------------------------ //
DLLEXPORT void ScriptSystemWrapper::Clear(){

    asIScriptFunction* func =
        ImplementationObject->GetObjectType()->GetMethodByName("Clear");

    if(!func) {

        LOG_ERROR("Script system: failed to find Clear method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
        ->RunScriptMethod<void>(setup, func, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system: failed to call Clear");
        return;
    }
}
