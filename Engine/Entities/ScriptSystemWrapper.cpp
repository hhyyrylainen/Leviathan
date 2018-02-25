// ------------------------------------ //
#include "ScriptSystemWrapper.h"

#include "GameWorld.h"
#include "Script/ScriptExecutor.h"

#include "add_on/scriptarray/scriptarray.h"
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
DLLEXPORT void ScriptSystemWrapper::Clear()
{
    asIScriptFunction* func = ImplementationObject->GetObjectType()->GetMethodByName("Clear");

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
// ------------------------------------ //
// ScriptSystemNodeHelper
DLLEXPORT void Leviathan::ScriptSystemNodeHelper(
    GameWorld* world, void* cachedcomponents, int cachedtypeid, CScriptArray& systemcomponents)
{
    LOG_WRITE("System component count: " + std::to_string(systemcomponents.GetSize()));

    asIScriptContext* context = asGetActiveContext();

    if(!context)
        throw InvalidState(
            "ScriptSystemNodeHelper: not called from a script function (no active context");

    if(!world) {

        context->SetException("world reference is null");
        return;
    }

    auto* engine = context->GetEngine();
    auto* exec = static_cast<ScriptExecutor*>(engine->GetUserData());

    // Verify types //
    const auto elementID = systemcomponents.GetElementTypeId();
    const auto wantedID = AngelScriptTypeIDResolver<ScriptSystemUses>::Get(exec);

    if(elementID != wantedID) {

        context->SetException(("expected systemcomponents array to hold objects of type " +
                               std::to_string(wantedID) + " but it contains type " +
                               std::to_string(elementID))
                                  .c_str());
        return;
    }

    // And then the harder to verify the one that can be anything //
    asITypeInfo* givenComponentsType = engine->GetTypeInfoById(cachedtypeid);

    // This is always named array so this isn't needed
    // asITypeInfo* baseArrayType = engine->GetTypeInfoByDecl("array<T>");
    // const char* baseName = baseArrayType->GetName();
    const char* baseName = "array";

    if(std::strcmp(givenComponentsType->GetName(), baseName) != 0) {

        context->SetException(
            ("expected cachedcomponents to be an array type: " + std::string(baseName) +
                " but it is type: " + std::string(givenComponentsType->GetName()))
                .c_str());
        return;
    }

    CScriptArray* cached = static_cast<CScriptArray*>(cachedcomponents);

    asITypeInfo* cacheclass = engine->GetTypeInfoById(systemcomponents.GetElementTypeId());

    LOG_WRITE("Creating components of class: " + std::string(cacheclass->GetName()));

    std::vector<std::tuple<void*, ObjectID>> addedCpp;
    std::vector<std::tuple<asIScriptObject*, ObjectID>> addedScript;
    std::vector<std::tuple<void*, ObjectID>> removedCpp;
    std::vector<std::tuple<asIScriptObject*, ObjectID>> removedScript;

    // Get all the added and removed at once //
    // TODO: Might be more cache efficient to first get all added c++ and then all added script
    // and then move on to the removed ones
    for(asUINT i = 0; i < systemcomponents.GetSize(); ++i) {

        ScriptSystemUses* type = static_cast<ScriptSystemUses*>(systemcomponents.At(i));

        if(type->UsesName) {

            world->GetAddedForScriptDefined(type->Name, addedScript);
            world->GetRemovedForScriptDefined(type->Name, removedScript);

        } else {

            world->GetAddedFor(static_cast<COMPONENT_TYPE>(type->Type), addedCpp);
            world->GetRemovedFor(static_cast<COMPONENT_TYPE>(type->Type), removedCpp);
        }
    }

    // Only do more checks if something has changed //
    if(!addedCpp.empty() || !addedScript.empty()) {

        // Handle added like in TupleCachedComponentCollectionHelper //
    }

    if(!removedCpp.empty() || !removedScript.empty()) {
        // And deleted like in any system that does CachedComponents.RemoveBasedOnKeyTupleList
    }
}
