#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CONSTRAINTS
#include "Constraints.h"
#endif
#include "Newton/PhysicalWorld.h"
#include "../Bases/BaseContraintable.h"
#include "../GameWorld.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::BaseContraint::BaseContraint(GameWorld* world, BaseContraintable* parent, BaseContraintable* child) : 
	ParentObject(parent), ChildObject(child), OwningWorld(world), Joint(NULL)
{
}

DLLEXPORT Leviathan::Entity::BaseContraint::~BaseContraint(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::BaseContraint::Init(){
	// we use the virtual functions to make child class handle this //
	if(!_CheckParameters())
		return false;
	if(!_CreateActualJoint())
		return false;

	return true;
}

DLLEXPORT void Leviathan::Entity::BaseContraint::Release(){
	// calls both because neither of them invoked the function //
	ConstraintPartUnlinkedDestroy(NULL);
}

DLLEXPORT void Leviathan::Entity::BaseContraint::ConstraintPartUnlinkedDestroy(BaseContraintable* callinginstance){
	// notify the object that isn't calling this function //
	if(ParentObject && ParentObject != callinginstance){
		ParentObject->ConstraintDestroyedRemove(this);

	}
	if(ChildObject && ChildObject != callinginstance){

		ChildObject->ConstraintDestroyedRemove(this);
	}
	ChildObject = NULL;
	ParentObject = NULL;

	if(Joint){
		NewtonDestroyJoint(OwningWorld->GetPhysicalWorld()->GetWorld() , Joint);
		Joint = NULL;
	}
}

// ------------------ SliderConstraint ------------------ //
DLLEXPORT Leviathan::Entity::SliderConstraint::SliderConstraint(GameWorld* world, BaseContraintable* parent, BaseContraintable* child) : 
	BaseContraint(world, parent, child), Axis(0)
{

}

DLLEXPORT Leviathan::Entity::SliderConstraint::~SliderConstraint(){

}
// ------------------------------------ //
DLLEXPORT BaseContraint* Leviathan::Entity::SliderConstraint::SetParameters(const Float3 &slidingaxis){
	Axis = slidingaxis;
	return this;
}
// ------------------------------------ //
bool Leviathan::Entity::SliderConstraint::_CheckParameters(){
	if(Axis.IsNormalized())
		return true;
	// if the vector is the default one it isn't normalized so the above check will suffice //
	return false;
}

bool Leviathan::Entity::SliderConstraint::_CreateActualJoint(){
	// we'll just call the Newton creation function and it should be it //
	Float3 pos(0.f, 0.f, 0.f);

	Joint = NewtonConstraintCreateSlider(OwningWorld->GetPhysicalWorld()->GetWorld(), &pos.X, &Axis.X, 
		ChildObject->GetPhysicsBody(), ParentObject->GetPhysicsBody());

	return Joint != NULL;
}
