// ------------------------------------ //
#include "ScriptSystemWrapper.h"

#include "GameWorld.h"
#include "Script/CustomScriptRunHelpers.h"
#include "Script/ScriptExecutor.h"
#include "ScriptComponentHolder.h"

#include "add_on/scriptarray/scriptarray.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT ScriptSystemWrapper::ScriptSystemWrapper(
    const std::string& name, asIScriptObject* impl) :
    Name(name),
    ImplementationObject(impl)
{
    if(!ImplementationObject)
        throw InvalidArgument("ScriptSystemWrapper not given an angelscript object");
}

DLLEXPORT ScriptSystemWrapper::~ScriptSystemWrapper()
{
    _ReleaseCachedFunctions();

    if(ImplementationObject) {
        LOG_ERROR("ScriptSystemWrapper: Release has not been called before destructor");
        ImplementationObject->Release();
    }
}
// ------------------------------------ //
DLLEXPORT asIScriptObject* ScriptSystemWrapper::GetASImplementationObject()
{
    if(ImplementationObject)
        ImplementationObject->AddRef();
    return ImplementationObject;
}
// ------------------------------------ //
DLLEXPORT void ScriptSystemWrapper::Init(GameWorld* world)
{
    asIScriptFunction* func = ImplementationObject->GetObjectType()->GetMethodByName("Init");

    if(!func) {

        LOG_ERROR("Script system(" + Name + "): failed to find Init method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, func, ImplementationObject, world);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system(" + Name + "): failed to call Init");
        return;
    }
}
DLLEXPORT void ScriptSystemWrapper::Release()
{
    asIScriptFunction* func =
        ImplementationObject->GetObjectType()->GetMethodByName("Release");

    if(!func) {

        LOG_ERROR("Script system(" + Name + "): failed to find Release method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, func, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system(" + Name + "): failed to call Release");
        return;
    }

    _ReleaseCachedFunctions();

    ImplementationObject->Release();
    ImplementationObject = nullptr;
}
// ------------------------------------ //
DLLEXPORT void ScriptSystemWrapper::Run()
{
    // For performance reasons this might be good to store
    if(!RunMethod) {

        RunMethod = ImplementationObject->GetObjectType()->GetMethodByName("Run");
        RunMethod->AddRef();
    }

    if(!RunMethod) {

        LOG_ERROR("Script system(" + Name + "): failed to find Run method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, RunMethod, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system(" + Name + "): failed to call Run");
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
        CreateAndDestroyNodesMethod->AddRef();
    }

    if(!CreateAndDestroyNodesMethod) {

        LOG_ERROR("Script system(" + Name +
                  "): failed to find CreateAndDestroyNodes method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, CreateAndDestroyNodesMethod, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system(" + Name + "): failed to call CreateAndDestroyNodes");
        return;
    }
}
// ------------------------------------ //
DLLEXPORT void ScriptSystemWrapper::Clear()
{
    asIScriptFunction* func = ImplementationObject->GetObjectType()->GetMethodByName("Clear");

    if(!func) {

        LOG_ERROR("Script system(" + Name + "): failed to find Clear method on as object");
        return;
    }

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, func, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system(" + Name + "): failed to call Clear");
        return;
    }
}

DLLEXPORT void ScriptSystemWrapper::Suspend()
{
    asIScriptFunction* func =
        ImplementationObject->GetObjectType()->GetMethodByName("Suspend");

    // This is optional
    if(!func)
        return;

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, func, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system(" + Name + "): failed to call Suspend");
        return;
    }
}

DLLEXPORT void ScriptSystemWrapper::Resume()
{
    asIScriptFunction* func = ImplementationObject->GetObjectType()->GetMethodByName("Resume");

    // This is optional
    if(!func)
        return;

    ScriptRunningSetup setup;
    auto result =
        static_cast<ScriptExecutor*>(ImplementationObject->GetEngine()->GetUserData())
            ->RunScriptMethod<void>(setup, func, ImplementationObject);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {

        LOG_ERROR("Script system(" + Name + "): failed to call Resume");
        return;
    }
}
// ------------------------------------ //
DLLEXPORT void ScriptSystemWrapper::_ReleaseCachedFunctions()
{

    if(RunMethod) {
        RunMethod->Release();
        RunMethod = nullptr;
    }

    if(CreateAndDestroyNodesMethod) {
        CreateAndDestroyNodesMethod->Release();
        CreateAndDestroyNodesMethod = nullptr;
    }
}
// ------------------------------------ //
// ScriptSystemNodeHelper
//! Helper for ScriptSystemNodeHelper
//! \returns True on success. False if failed (will also set exception on context)
inline bool GetCachedScriptObjectIDAtIndex(CScriptArray* cached, asUINT index,
    asIScriptContext* context, ScriptExecutor* exec, ObjectID& id)
{
    asIScriptObject* obj = *static_cast<asIScriptObject**>(cached->At(index));

    if(obj->GetPropertyCount() < 1) {

        context->SetException(("cachedcomponents number " + std::to_string(index) +
                               " doesn't have 'ObjectID id' as its first property")
                                  .c_str());
        return false;
    }

    // This check can be disabled once this is working (as reading 4 bytes will most
    // often probably work, but for developing new systems this check is good)
    const auto propertyType = obj->GetPropertyTypeId(0);

    const auto neededType = AngelScriptTypeIDResolver<ObjectID>::Get(exec);

    if(propertyType != neededType) {

        context->SetException(("cachedcomponents number " + std::to_string(index) +
                               " doesn't have 'ObjectID id' as its first property. Type " +
                               std::to_string(propertyType) +
                               " doesn't match ObjectID type: " + std::to_string(neededType))
                                  .c_str());
        return false;
    }

    // Now that the property is (probably) validated we can read it
    ObjectID* idProperty = static_cast<ObjectID*>(obj->GetAddressOfProperty(0));

    id = *idProperty;
    return true;
}

struct ComponentFindStatusForCache {

    ComponentFindStatusForCache(asIScriptObject* as) :
        ScriptComponent(as), CType(-1, -1), IsScript(true)
    {
    }
    ComponentFindStatusForCache(void* cpp, const ComponentTypeInfo& info) :
        CppComponent(cpp), CType(info), IsScript(false)
    {
    }

    union {
        void* CppComponent;
        asIScriptObject* ScriptComponent;
    };

    ComponentTypeInfo CType;
    bool IsScript;
};


//! Helper for ScriptSystemNodeHelper
//! \returns False if failed and a script exception was set
inline bool TryToCreateNewCachedComponentsForEntity(ObjectID newentity, CScriptArray* cached,
    asIScriptFunction* factoryfunc,
    std::vector<std::tuple<void*, ObjectID, ComponentTypeInfo>>& addedcpp,
    std::vector<std::tuple<asIScriptObject*, ObjectID, ScriptComponentHolder*>>& addedscript,
    GameWorld* world, CScriptArray& systemcomponents, asIScriptContext* context,
    ScriptExecutor* exec)
{
    // Find the needed components //
    std::vector<ComponentFindStatusForCache> foundComponents;

    const auto sysSize = systemcomponents.GetSize();

    foundComponents.reserve(sysSize);

    for(asUINT i = 0; i < sysSize; ++i) {

        ScriptSystemUses* type = static_cast<ScriptSystemUses*>(systemcomponents.At(i));

        bool found = false;

        if(type->UsesName) {

            // First search added //
            for(const auto& addedTuple : addedscript) {

                if(std::get<1>(addedTuple) != newentity ||
                    std::get<2>(addedTuple)->ComponentType != type->Name)
                    continue;

                // Found //
                foundComponents.push_back(std::get<0>(addedTuple));
                found = true;
                break;
            }

            // And then do full search //
            if(!found) {

                auto* holder = world->GetScriptComponentHolder(type->Name);

                if(!holder) {

                    context->SetException(
                        ("systemcomponents has type that world doesn't have: " + type->Name)
                            .c_str());
                    return false;
                }

                asIScriptObject* fullSearchResult = holder->Find(newentity);
                holder->Release();

                if(fullSearchResult) {

                    // We don't need to keep a reference as the holder will do that for us //
                    fullSearchResult->Release();
                    foundComponents.push_back(fullSearchResult);
                    found = true;
                }
            }


        } else {

            // First search added //
            for(const auto& addedTuple : addedcpp) {

                if(std::get<1>(addedTuple) != newentity ||
                    std::get<2>(addedTuple).LeviathanType != type->Type)
                    continue;

                // Found //
                foundComponents.push_back({std::get<0>(addedTuple), std::get<2>(addedTuple)});
                found = true;
                break;
            }

            // And then do full search //
            if(!found) {

                const auto existingComponent = world->GetComponentWithType(
                    newentity, static_cast<COMPONENT_TYPE>(type->Type));

                if(std::get<0>(existingComponent)) {

                    foundComponents.push_back(
                        {std::get<0>(existingComponent), std::get<1>(existingComponent)});
                    found = true;
                }
            }
        }

        if(!found) {
            // Fail if not found //
            return true;
        }
    }

    // Skip if already exists //
    bool exists = false;

    for(asUINT i = 0; i < cached->GetSize(); ++i) {

        ObjectID currentID;
        if(!GetCachedScriptObjectIDAtIndex(cached, i, context, exec, currentID))
            return false;

        if(currentID == newentity) {

            exists = true;
            break;
        }
    }

    if(exists)
        return true;

    // Call factory to create it //
    auto scriptRunInfo = exec->PrepareCustomScriptRun(factoryfunc);

    if(scriptRunInfo) {

        // Pass parameters //
        // TODO: the types here should be checked only once and then just assumed to be right
        // for more performance

        if(!PassParameterToCustomRun(scriptRunInfo, newentity)) {

            context->SetException(("failed to pass ObjectID as first param to factory func: " +
                                   std::string(factoryfunc->GetDeclaration()))
                                      .c_str());
            return false;
        }

        // Then the rest //
        for(const auto& component : foundComponents) {

            bool success = false;

            if(component.IsScript) {

                success = PassParameterToCustomRun(scriptRunInfo, component.ScriptComponent);

            } else {
                success = PassParameterToCustomRun(
                    scriptRunInfo, component.CppComponent, component.CType.AngelScriptType);
            }

            if(!success) {

                context->SetException(
                    ("failed to pass parameter number " +
                        std::to_string(scriptRunInfo->PassedIndex) +
                        "  to factory func: " + std::string(factoryfunc->GetDeclaration()))
                        .c_str());
                return false;
            }
        }
    }

    auto result = exec->ExecuteCustomRun<asIScriptObject*>(scriptRunInfo);

    if(result.Result != SCRIPT_RUN_RESULT::Success || result.Value == nullptr) {

        context->SetException(("failed to create new cachedcomponent, factory function failed "
                               "to run or returned null, func: " +
                               std::string(factoryfunc->GetDeclaration()))
                                  .c_str());
        return false;
    }

    // And then store it in cached //
    // The array increments reference count
    // handle type so we need to give this a pointer to a pointer
    cached->InsertLast(&result.Value);
    return true;
}

DLLEXPORT void Leviathan::ScriptSystemNodeHelper(
    GameWorld* world, void* cachedcomponents, int cachedtypeid, CScriptArray& systemcomponents)
{
    asIScriptContext* context = asGetActiveContext();

    if(!context)
        throw InvalidState(
            "ScriptSystemNodeHelper: not called from a script function (no active context)");

    if(!world) {

        context->SetException("ScriptSystemNodeHelper: world reference is null");
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

    // Needs to be a handle type //
    if(!(cachedtypeid & asTYPEID_OBJHANDLE)) {

        context->SetException("expected cachedcomponents to be a handle to array type");
        return;
    }

    // Handle type so it is a double pointer //
    CScriptArray* cached = *static_cast<CScriptArray**>(cachedcomponents);

    asITypeInfo* cacheclass = engine->GetTypeInfoById(cached->GetElementTypeId());

    if(!cacheclass) {

        context->SetException(("failed to get type inside cachedcomponents, id: " +
                               std::to_string(systemcomponents.GetElementTypeId()))
                                  .c_str());
        return;
    }

    std::vector<std::tuple<void*, ObjectID, ComponentTypeInfo>> addedCpp;
    std::vector<std::tuple<asIScriptObject*, ObjectID, ScriptComponentHolder*>> addedScript;
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

        // Find the first factory that has the right number of arguments //
        asIScriptFunction* factoryFunc = nullptr;

        const asUINT factoryCount = cacheclass->GetFactoryCount();
        const auto expectedParamCount = systemcomponents.GetSize() + 1;

        for(asUINT i = 0; i < factoryCount; ++i) {

            asIScriptFunction* currentToCheck = cacheclass->GetFactoryByIndex(i);

            if(currentToCheck->GetParamCount() == expectedParamCount) {
                factoryFunc = currentToCheck;
                break;
            }
        }

        if(!factoryFunc) {

            context->SetException(
                ("type inside cachedcomponents has no suitable factory, expected one with " +
                    std::to_string(expectedParamCount) +
                    " parameters, type: " + std::string(cacheclass->GetName()))
                    .c_str());
            return;
        }


        // For sanity reasons this is split into a helper which goes through all of the added
        // vectors again trying to build an enity of the ID
        for(const auto& tuple : addedCpp) {

            if(!TryToCreateNewCachedComponentsForEntity(std::get<1>(tuple), cached,
                   factoryFunc, addedCpp, addedScript, world, systemcomponents, context, exec))
                return;
        }

        for(const auto& tuple : addedScript) {

            // Skip if already checked //
            bool inCpp = false;
            for(const auto& alreadyDone : addedCpp) {

                if(std::get<1>(tuple) == std::get<1>(alreadyDone)) {

                    inCpp = true;
                    break;
                }
            }

            if(inCpp)
                continue;

            if(!TryToCreateNewCachedComponentsForEntity(std::get<1>(tuple), cached,
                   factoryFunc, addedCpp, addedScript, world, systemcomponents, context, exec))
                return;
        }
    }

    if(!removedCpp.empty() || !removedScript.empty()) {
        // And deleted like in any system that does CachedComponents.RemoveBasedOnKeyTupleList

        // Cheaper to query each object only once about their id //
        for(asUINT i = 0; i < cached->GetSize();) {

            ObjectID currentID;
            if(!GetCachedScriptObjectIDAtIndex(cached, i, context, exec, currentID))
                return;

            bool remove = false;

            for(const auto& tuple : removedCpp) {

                if(std::get<1>(tuple) == currentID) {
                    remove = true;
                    break;
                }
            }

            if(!remove) {
                for(const auto& tuple : removedScript) {

                    if(std::get<1>(tuple) == currentID) {
                        remove = true;
                        break;
                    }
                }
            }

            if(!remove) {
                ++i;
                continue;
            }

            // Remove it //
            // We do a swap trick here //
            // This should work even for size == 1
            cached->SetValue(i, cached->At(cached->GetSize() - 1));
            cached->RemoveLast();
        }
    }
}
