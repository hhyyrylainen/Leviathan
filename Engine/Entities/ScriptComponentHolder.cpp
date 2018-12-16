// ------------------------------------ //
#include "ScriptComponentHolder.h"

#include "GameWorld.h"
#include "Script/ScriptConversionHelpers.h"
#include "Script/ScriptExecutor.h"

#include <boost/range/adaptor/map.hpp>
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

    Factory->Release();

    LEVIATHAN_ASSERT(CreatedObjects.empty(), "ScriptComponentHolder didn't properly clear");
}
// ------------------------------------ //
DLLEXPORT bool ScriptComponentHolder::ReleaseComponent(ObjectID entity)
{
    auto iter = CreatedObjects.find(entity);

    if(iter == CreatedObjects.end())
        return false;

    Removed.push_back(std::make_tuple(iter->second, entity));

    // Remove from added if there //
    for(auto iter = Added.begin(); iter != Added.end(); ++iter) {

        if(std::get<1>(*iter) == entity) {

            std::iter_swap(iter, Added.rbegin());
            Added.pop_back();
            break;
        }
    }

    // TODO: call release on the actual script object to let it do shutdown stuff
    // Release our reference to let the object be destroyed once all references are released
    iter->second->Release();

    CreatedObjects.erase(iter);

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
    asIScriptObject* found = Find(entity);
    if(found != nullptr) {

        LOG_WARNING("ScriptComponentHolder: Create: called for existing component, id: " +
                    std::to_string(entity));

        // Must release reference
        found->Release();
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

    // And we return it so increase refcount for that too //
    result.Value->AddRef();
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
// ------------------------------------ //
DLLEXPORT CScriptArray* ScriptComponentHolder::GetIndex() const
{
    asIScriptContext* ctx = asGetActiveContext();

    asIScriptEngine* engine = ctx ? ctx->GetEngine() : ScriptExecutor::Get()->GetASEngine();

    return ConvertIteratorToASArray((CreatedObjects | boost::adaptors::map_keys).begin(),
        (CreatedObjects | boost::adaptors::map_keys).end(), engine, "array<ObjectID>");
}
