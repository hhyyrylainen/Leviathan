// ------------------------------------ //
#include "ScriptComponentHolder.h"

#include "GameWorld.h"
#include "Script/ScriptExecutor.h"
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT ScriptComponentHolder::ScriptComponentHolder(
    const std::string& name, asIScriptFunction* factory, GameWorld* world) :
    ComponentType(name),
    Factory(factory), World(world)
{
    if(!Factory)
        throw InvalidArgument("ScriptComponentHolder not given a factory function");
}
DLLEXPORT ScriptComponentHolder::~ScriptComponentHolder()
{
    // Make sure all are released
    ReleaseAllComponents();

    LEVIATHAN_ASSERT(CreatedObjects.empty(), "ScriptComponentHolder didn't properly clear");
}
// ------------------------------------ //
DLLEXPORT bool ScriptComponentHolder::ReleaseComponent(ObjectID entity)
{
    auto iter = CreatedObjects.find(entity);

    if(iter == CreatedObjects.end())
        return false;

    // TODO: call release on the actual script object to let it do shutdown stuff
    // Release our reference to let the object be destroyed once all references are released
    iter->second->Release();

    CreatedObjects.erase(iter);

    Removed.push_back(std::make_tuple(iter->second, entity));

    // Remove from added if there //
    for(auto iter = Added.begin(); iter != Added.end(); ++iter) {

        if(std::get<1>(*iter) == entity) {

            std::iter_swap(iter, Added.rbegin());
            Added.pop_back();
            break;
        }
    }

    return true;
}

DLLEXPORT void ScriptComponentHolder::ReleaseAllComponents()
{
    for(auto iter = CreatedObjects.begin(); iter != CreatedObjects.end(); ++iter) {

        // TODO: also call release here
        iter->second->Release();
    }

    CreatedObjects.clear();
    Added.clear();
    Removed.clear();
}

DLLEXPORT asIScriptObject* ScriptComponentHolder::Create(ObjectID entity)
{
    // Fail if already exists //
    if(Find(entity) != nullptr) {

        LOG_WARNING("ScriptComponentHolder: Create: called for existing component, id: " +
                    std::to_string(entity));
        return nullptr;
    }

    ScriptRunningSetup setup;

    auto result = static_cast<ScriptExecutor*>(Factory->GetEngine()->GetUserData())
                      ->RunScript<asIScriptObject*>(Factory, nullptr, setup, World);

    if(result.Result != SCRIPT_RUN_RESULT::Success || result.Value == nullptr) {

        LOG_ERROR("ScriptComponentHolder: Create: failed to run the angelscript factory "
                  "function for this type (" +
                  ComponentType + ")");
        return nullptr;
    }

    Added.push_back(std::make_tuple(result.Value, entity));

    // Remove from removed if there //
    for(auto iter = Removed.begin(); iter != Removed.end(); ++iter) {

        if(std::get<1>(*iter) == entity) {

            std::iter_swap(iter, Removed.rbegin());
            Removed.pop_back();
            break;
        }
    }

    // Remember that we take a reference to it
    result.Value->AddRef();

    CreatedObjects[entity] = result.Value;
    return result.Value;
}

DLLEXPORT asIScriptObject* ScriptComponentHolder::Find(ObjectID entity)
{
    auto iter = CreatedObjects.find(entity);

    if(iter == CreatedObjects.end())
        return nullptr;

    iter->second->AddRef();
    return iter->second;
}