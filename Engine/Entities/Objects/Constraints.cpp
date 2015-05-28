// ------------------------------------ //
#include "Constraints.h"

#include "Newton/PhysicalWorld.h"
#include "../GameWorld.h"
#include "../../Handlers/IDFactory.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT BaseConstraint::BaseConstraint(ENTITY_CONSTRAINT_TYPE type,
    GameWorld* world, Constraintable &first, Constraintable &second) : 
	FirstObject(first), SecondObject(second), ID(IDFactory::GetID()),
    OwningWorld(world), Joint(NULL), Type(type)

{
    
}

DLLEXPORT BaseConstraint::BaseConstraint(ENTITY_CONSTRAINT_TYPE type, GameWorld* world,
    Constraintable &first, Constraintable &second, int id) :
    FirstObject(first), SecondObject(second), ID(id),
    OwningWorld(world), Joint(NULL), Type(type)
{

}

DLLEXPORT BaseConstraint::~BaseConstraint(){

}
// ------------------------------------ //
DLLEXPORT bool BaseConstraint::Init(){
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

DLLEXPORT void BaseConstraint::Release(){

    Destroy(nullptr);
}

DLLEXPORT void BaseConstraint::Destroy(Constraintable* skipthis /*= nullptr*/){
    {
        GUARD_LOCK();

        if(Type == ENTITY_CONSTRAINT_TYPE_DESTRUCTED)
            return;

        if(&FirstObject != skipthis)
            FirstObject.RemoveConstraint(this);

        if(&SecondObject != skipthis)
            SecondObject.RemoveConstraint(this);
    

        Type = ENTITY_CONSTRAINT_TYPE_DESTRUCTED;

        if(!OwningWorld)
            return;

        if(Joint && OwningWorld){
        
            NewtonDestroyJoint(OwningWorld->GetPhysicalWorld()->GetNewtonWorld(), Joint);
        }

        Joint = NULL;
    }

    // This call should get rid of the last reference
    // We have been unlocked before calling delete
    OwningWorld->ConstraintDestroyed(this);

    // We should be deleted now
}
// ------------------------------------ //
DLLEXPORT int BaseConstraint::GetID() const{

    return ID;
}
// ------------------------------------ //
DLLEXPORT bool BaseConstraint::_CheckParameters(){

    return false;
}

DLLEXPORT bool BaseConstraint::_CreateActualJoint(){

    return false;
}
// ------------------------------------ //
DLLEXPORT Constraintable& BaseConstraint::GetFirstEntity() const{
    return FirstObject;
}

DLLEXPORT Constraintable& BaseConstraint::GetSecondEntity() const{
    return SecondObject;
}
// ------------------ SliderConstraint ------------------ //
DLLEXPORT SliderConstraint::SliderConstraint(GameWorld* world,
    Constraintable &first, Constraintable &second) : 
	BaseConstraint(ENTITY_CONSTRAINT_TYPE_SLIDER, world, first, second), Axis(0)
{

}

DLLEXPORT SliderConstraint::SliderConstraint(GameWorld* world, Constraintable &first,
    Constraintable &second, int id) :
    BaseConstraint(ENTITY_CONSTRAINT_TYPE_SLIDER, world, first, second, id), Axis(0)
{

}

DLLEXPORT SliderConstraint::~SliderConstraint(){

}
// ------------------------------------ //
DLLEXPORT SliderConstraint* SliderConstraint::SetParameters(
    const Float3 &slidingaxis)
{
	Axis = slidingaxis;
	return this;
}
// ------------------------------------ //
DLLEXPORT Float3 SliderConstraint::GetAxis() const{
    return Axis;
}
// ------------------------------------ //
bool SliderConstraint::_CheckParameters(){
	if(Axis.IsNormalized())
		return true;
    
	// If the vector is the default one it isn't normalized so the above check will suffice //
	return false;
}

bool SliderConstraint::_CreateActualJoint(){
	// We'll just call the Newton create function and that should should be it //
	Float3 pos(0.f, 0.f, 0.f);

    // TODO: check if we could add a GetPhysicsBody function to BaseConstraintable
    try{
        
        auto& first = OwningWorld->GetComponent<Physics>(FirstObject.PartOfEntity);
        auto& second = OwningWorld->GetComponent<Physics>(SecondObject.PartOfEntity);

        
        Joint = NewtonConstraintCreateSlider(OwningWorld->GetPhysicalWorld()->GetNewtonWorld(),
            &pos.X, &Axis.X, first.Body, second.Body);

        return Joint != NULL;
    
    } catch(const NotFound&){

        Logger::Get()->Error("SliderCOnstraint: trying to create between non-Physics objects");
        return false;
    }
}

