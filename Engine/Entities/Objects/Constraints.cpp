// ------------------------------------ //
#ifndef LEVIATHAN_CONSTRAINTS
#include "Constraints.h"
#endif
#include "Newton/PhysicalWorld.h"
#include "../Bases/BaseConstraintable.h"
#include "../GameWorld.h"
#include "Entities/Bases/BasePhysicsObject.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::BaseConstraint::BaseConstraint(ENTITY_CONSTRAINT_TYPE type, GameWorld* world,
    BaseConstraintable* parent, BaseConstraintable* child) : 
	ParentObject(parent), ChildObject(child), OwningWorld(world), Joint(NULL), Type(type)
{
}

DLLEXPORT Leviathan::Entity::BaseConstraint::~BaseConstraint(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::BaseConstraint::Init(){
	// We use the virtual functions to make the child class handle this //
	if(!_CheckParameters()){

        Logger::Get()->Error("BaseContraint: invalid parameters");
		return false;
    }
    
	if(!_CreateActualJoint()){

        Logger::Get()->Error("BaseContraint: failed to create actual joint");
		return false;
    }

	return true;
}

DLLEXPORT void Leviathan::Entity::BaseConstraint::Release(){

	// Both are called because neither of them invoked this function //
	ConstraintPartUnlinkedDestroy(NULL);
}

DLLEXPORT void Leviathan::Entity::BaseConstraint::ConstraintPartUnlinkedDestroy(BaseConstraintable* callinginstance){
    
    GUARD_LOCK_THIS_OBJECT();
    
	// Notify the object that isn't calling this function //
	if(ParentObject && ParentObject != callinginstance){
        
		ParentObject->ConstraintDestroyedRemove(this);
	}

	if(ChildObject && ChildObject != callinginstance){

		ChildObject->ConstraintDestroyedRemove(this);
	}
    
	ChildObject = NULL;
	ParentObject = NULL;

	if(Joint && OwningWorld){
        
		NewtonDestroyJoint(OwningWorld->GetPhysicalWorld()->GetNewtonWorld(), Joint);
	}

    Joint = NULL;
}
// ------------------------------------ //
DLLEXPORT BaseConstraintable* Leviathan::Entity::BaseConstraint::GetFirstEntity() const{
    return ParentObject;
}

DLLEXPORT BaseConstraintable* Leviathan::Entity::BaseConstraint::GetSecondEntity() const{
    return ChildObject;
}
// ------------------------------------ //
void Leviathan::Entity::BaseConstraint::_WorldDisowned(){

    GUARD_LOCK_THIS_OBJECT();

    // Destroy the joint //
    Release();

    OwningWorld = NULL;
}

// ------------------ SliderConstraint ------------------ //
DLLEXPORT Leviathan::Entity::SliderConstraint::SliderConstraint(GameWorld* world, BaseConstraintable* parent,
    BaseConstraintable* child) : 
	BaseConstraint(ENTITY_CONSTRAINT_TYPE_SLIDER, world, parent, child), Axis(0)
{

}

DLLEXPORT Leviathan::Entity::SliderConstraint::~SliderConstraint(){

}
// ------------------------------------ //
DLLEXPORT SliderConstraint* Leviathan::Entity::SliderConstraint::SetParameters(const Float3 &slidingaxis){
	Axis = slidingaxis;
	return this;
}
// ------------------------------------ //
DLLEXPORT Float3 Leviathan::Entity::SliderConstraint::GetAxis() const{
    return Axis;
}
// ------------------------------------ //
bool Leviathan::Entity::SliderConstraint::_CheckParameters(){
	if(Axis.IsNormalized())
		return true;
    
	// If the vector is the default one it isn't normalized so the above check will suffice //
	return false;
}

bool Leviathan::Entity::SliderConstraint::_CreateActualJoint(){
	// We'll just call the Newton create function and that should should be it //
	Float3 pos(0.f, 0.f, 0.f);

    // TODO: check if we could add a GetPhysicsBody function to BaseConstraintable
    auto first = dynamic_cast<BasePhysicsObject*>(ChildObject);
    auto second = dynamic_cast<BasePhysicsObject*>(ParentObject);

    if(!first || !second){

        Logger::Get()->Error("SliderConstraint: passed in an entity that doesn't have physics");
        return false;
    }
    
	Joint = NewtonConstraintCreateSlider(OwningWorld->GetPhysicalWorld()->GetNewtonWorld(), &pos.X, &Axis.X, 
		first->GetPhysicsBody(), second->GetPhysicsBody());

	return Joint != NULL;
}
// ------------------ ControllerConstraint ------------------ //
DLLEXPORT Leviathan::Entity::ControllerConstraint::ControllerConstraint(GameWorld* world,
    BaseConstraintable* controller, BaseConstraintable* child) :
    BaseConstraint(ENTITY_CONSTRAINT_TYPE_CONTROLLERCONSTRAINT, world, controller, child)
{

}

DLLEXPORT Leviathan::Entity::ControllerConstraint::~ControllerConstraint(){


}
// ------------------------------------ //
bool Leviathan::Entity::ControllerConstraint::_CheckParameters(){

    return ChildObject != NULL;
}

bool Leviathan::Entity::ControllerConstraint::_CreateActualJoint(){
    return true;
}
