// ------------------------------------ //
#include "PhysicsMaterialManager.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT PhysicsMaterialManager::PhysicsMaterialManager(const NewtonManager* newtoninstanced)
{

    // set static instance //
    StaticInstance = this;
}

DLLEXPORT PhysicsMaterialManager::~PhysicsMaterialManager()
{
    StaticInstance = NULL;
}

PhysicsMaterialManager* PhysicsMaterialManager::StaticInstance = NULL;

DLLEXPORT PhysicsMaterialManager* PhysicsMaterialManager::Get()
{
    return StaticInstance;
}
// ------------------------------------ //
DLLEXPORT void PhysicsMaterialManager::LoadedMaterialAdd(
    std::shared_ptr<PhysicalMaterial> material)
{
    // Add to the map //
    LoadedMaterials[material->GetName()] = material;
}

DLLEXPORT int PhysicsMaterialManager::GetMaterialIDForWorld(
    const std::string& name, NewtonWorld* WorldPtr)
{
    // Search for it by name //
    auto iter = LoadedMaterials.find(name);

    if(iter != LoadedMaterials.end()) {

        return iter->second->GetMaterialIDIfLoaded(WorldPtr);
    }

    // Not found //
    return -1;
}

DLLEXPORT std::shared_ptr<PhysicalMaterial> PhysicsMaterialManager::GetMaterial(
    const std::string& name)
{
    // Search for it by name //
    auto iter = LoadedMaterials.find(name);

    if(iter != LoadedMaterials.end()) {

        return iter->second;
    }

    // Not found //
    return NULL;
}

DLLEXPORT void PhysicsMaterialManager::CreateActualMaterialsForWorld(NewtonWorld* newtonworld)
{
    // Call first the primitive ID list so that all materials are created before applying
    // properties //
    _CreatePrimitiveIDList(newtonworld);
    _ApplyMaterialProperties(newtonworld);
}

DLLEXPORT void PhysicsMaterialManager::DestroyActualMaterialsForWorld(NewtonWorld* world)
{

    for(auto iter = LoadedMaterials.begin(); iter != LoadedMaterials.end(); ++iter) {

        iter->second->_ClearFromWorld(world);
    }
}

void PhysicsMaterialManager::_CreatePrimitiveIDList(NewtonWorld* world)
{
    // Loop the map and call creation functions on the objects //
    for(auto iter = LoadedMaterials.begin(); iter != LoadedMaterials.end(); ++iter) {

        iter->second->_CreateMaterialToWorld(world);
    }
}

void PhysicsMaterialManager::_ApplyMaterialProperties(NewtonWorld* world)
{
    // Loop the map and call creation functions on the objects //
    for(auto iter = LoadedMaterials.begin(); iter != LoadedMaterials.end(); ++iter) {

        iter->second->_ApplyMaterialPropertiesToWorld(world);
    }
}
// ------------------------------------ //
