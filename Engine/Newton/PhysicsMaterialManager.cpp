#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_PHYSICSMATERIALMANAGER
#include "PhysicsMaterialManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::PhysicsMaterialManager::PhysicsMaterialManager(const NewtonManager* newtoninstanced){

	// set static instance //
	StaticInstance = this;
}

DLLEXPORT Leviathan::PhysicsMaterialManager::~PhysicsMaterialManager(){
	StaticInstance = NULL;
}

PhysicsMaterialManager* Leviathan::PhysicsMaterialManager::StaticInstance = NULL;

DLLEXPORT PhysicsMaterialManager* Leviathan::PhysicsMaterialManager::Get(){
	return StaticInstance;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicsMaterialManager::LoadedMaterialAdd(PhysicalMaterial* ptrtotakeownership){
	// Add to the map //
	LoadedMaterials.insert(std::make_pair(ptrtotakeownership->GetName(), shared_ptr<PhysicalMaterial>(ptrtotakeownership)));
}

DLLEXPORT int Leviathan::PhysicsMaterialManager::GetMaterialIDForWorld(const wstring &name, NewtonWorld* WorldPtr){
	// Search for it by name //
	auto iter = LoadedMaterials.find(name);

	if(iter != LoadedMaterials.end()){

		return iter->second->GetMaterialIDIfLoaded(WorldPtr);
	}

	// Not found //
	return -1;
}

DLLEXPORT shared_ptr<PhysicalMaterial> Leviathan::PhysicsMaterialManager::GetMaterial(const wstring &name){
	// Search for it by name //
	auto iter = LoadedMaterials.find(name);

	if(iter != LoadedMaterials.end()){

		return iter->second;
	}

	// Not found //
	return NULL;
}

DLLEXPORT void Leviathan::PhysicsMaterialManager::CreateActualMaterialsForWorld(NewtonWorld* newtonworld){
	// Call first the primitive ID list so that all materials are created before applying properties //
	_CreatePrimitiveIDList(newtonworld);
	_ApplyMaterialProperties(newtonworld);
}

void Leviathan::PhysicsMaterialManager::_CreatePrimitiveIDList(NewtonWorld* world){
	// Loop the map and call creation functions on the objects //
	for(auto iter = LoadedMaterials.begin(); iter != LoadedMaterials.end(); ++iter){

		iter->second->_CreateMaterialToWorld(world);
	}
}

void Leviathan::PhysicsMaterialManager::_ApplyMaterialProperties(NewtonWorld* world){
	// Loop the map and call creation functions on the objects //
	for(auto iter = LoadedMaterials.begin(); iter != LoadedMaterials.end(); ++iter){

		iter->second->_ApplyMaterialPropertiesToWorld(world);
	}
}
// ------------------------------------ //
