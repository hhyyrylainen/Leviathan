#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_PHYSICALMATERIAL
#include "PhysicalMaterial.h"
#endif
#include "PhysicalMaterialManager.h"
#include "Handlers/IDFactory.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::PhysicalMaterial::PhysicalMaterial(const wstring &name) : Name(name), EngineID(IDFactory::GetID()){

}

DLLEXPORT Leviathan::PhysicalMaterial::PhysicalMaterial(shared_ptr<ObjectFileObject> fileobject) : EngineID(IDFactory::GetID()){
	throw std::exception();
}

DLLEXPORT Leviathan::PhysicalMaterial::~PhysicalMaterial(){
	// Material ID cannot be removed unless the world is empty, so just let smart ptrs do their magic //

}
// ------------------------------------ //
DLLEXPORT PhysMaterialDataPair& Leviathan::PhysicalMaterial::FormPairWith(const PhysicalMaterial &other){
	InterractionVariables.push_back(shared_ptr<PhysMaterialDataPair>(new PhysMaterialDataPair(other.Name)));

	// Return last element in the list //
	return *InterractionVariables.back().get();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::PhysicalMaterial::GetMaterialIDIfLoaded(NewtonWorld* world){

	auto iter = NewtonWorldAndID.find(world);

	if(iter != NewtonWorldAndID.end()){

		return iter->second;
	}
	// None loaded //
	return -1;
}
// ------------------------------------ //
void Leviathan::PhysicalMaterial::_CreateMaterialToWorld(NewtonWorld* world){
	// Create ID for world //
	NewtonWorldAndID[world] = NewtonMaterialCreateGroupID(world);
}

void Leviathan::PhysicalMaterial::_ApplyMaterialPropertiesToWorld(NewtonWorld* world){
	// Loop interaction variables and apply their properties //
	for(auto iter = InterractionVariables.begin(); iter != InterractionVariables.end(); ++iter){

		(*iter)->ApplySettingsToWorld(world, GetMaterialIDIfLoaded(world),
			PhysicsMaterialManager::Get()->GetMaterial((*iter)->OtherName)->GetMaterialIDIfLoaded(world), this);
	}
}
// ------------------ PhysMaterialDataPair ------------------ //
void Leviathan::PhysMaterialDataPair::ApplySettingsToWorld(NewtonWorld* world, int thisid, int otherid, PhysicalMaterial* materialowner){

	// Variables //
	NewtonMaterialSetDefaultElasticity(world, thisid, otherid, Elasticity);
	NewtonMaterialSetDefaultCollidable(world, thisid, otherid, Collidable);
	NewtonMaterialSetDefaultFriction(world, thisid, otherid, StaticFriction, DynamicFriction);
	NewtonMaterialSetDefaultSoftness(world, thisid, otherid, Softness);

	// Callback setting //
	NewtonMaterialSetCollisionCallback(world, thisid, otherid, materialowner, AABBCallback, ContactCallback);
}


