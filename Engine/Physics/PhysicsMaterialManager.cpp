// ------------------------------------ //
#include "PhysicsMaterialManager.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT void PhysicsMaterialManager::LoadedMaterialAdd(
    std::unique_ptr<PhysicalMaterial>&& material)
{
    if(!material)
        return;

    // Add to the map //
    PhysicalMaterial* ptr = material.get();
    LoadedMaterials[material->GetName()] = std::move(material);
    LoadedMaterialsByID[ptr->GetID()] = ptr;
}

DLLEXPORT int PhysicsMaterialManager::GetMaterialID(const std::string& name)
{
    // Search for it by name //
    auto iter = LoadedMaterials.find(name);

    if(iter != LoadedMaterials.end()) {

        return iter->second->GetID();
    }

    // Not found //
    return -1;
}

DLLEXPORT PhysicalMaterial* PhysicsMaterialManager::GetMaterial(const std::string& name)
{
    // Search for it by name //
    auto iter = LoadedMaterials.find(name);

    if(iter != LoadedMaterials.end()) {

        return iter->second.get();
    }

    // Not found //
    return nullptr;
}

DLLEXPORT PhysicalMaterial* PhysicsMaterialManager::GetMaterial(int id)
{
    auto iter = LoadedMaterialsByID.find(id);

    if(iter != LoadedMaterialsByID.end()) {

        return iter->second;
    }

    // Not found //
    return nullptr;
}
