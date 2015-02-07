// ------------------------------------ //
#ifndef LEVIATHAN_BASECONSTRAINTABLE
#include "BaseConstraintable.h"
#endif
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
// I hope that this virtual inherited class' constructor won't run from here //
DLLEXPORT Leviathan::BaseConstraintable::BaseConstraintable() : BaseObject(-1, NULL){

}

DLLEXPORT Leviathan::BaseConstraintable::~BaseConstraintable(){
	SAFE_DELETE_VECTOR(PartInConstraints);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseConstraintable::UnlinkConstraint(shared_ptr<BaseConstraint> constraintptr){
	for(size_t i = 0; i < PartInConstraints.size(); i++){
		if(PartInConstraints[i]->ParentPtr == constraintptr){

			SAFE_DELETE(PartInConstraints[i]);
			PartInConstraints.erase(PartInConstraints.begin()+i);
			return true;
		}
	}
	return false;
}

DLLEXPORT void Leviathan::BaseConstraintable::AggressiveConstraintUnlink(){
	// destroy the vector, which will run destructors that destroy all constraints //
	SAFE_DELETE_VECTOR(PartInConstraints);
}
// ------------------------------------ //
DLLEXPORT shared_ptr<BaseConstraint> Leviathan::BaseConstraintable::GetConstraintPtr(BaseConstraint* unsafeptr){
	// returns this object's copy of the ptr if it is found //
	for(size_t i = 0; i < PartInConstraints.size(); i++){
		if(PartInConstraints[i]->ParentPtr.get() == unsafeptr){

			return PartInConstraints[i]->ParentPtr;
		}
	}
	return NULL;
}
// ------------------------------------ //
void Leviathan::BaseConstraintable::ConstraintDestroyedRemove(BaseConstraint* ptr){
	// this function may not call destruct, so we need to reset the ptrs by hand //
	for(size_t i = 0; i < PartInConstraints.size(); i++){
		if(PartInConstraints[i]->IsParent ? PartInConstraints[i]->ParentPtr.get() == ptr: 
			PartInConstraints[i]->ChildPartPtr.lock().get() == ptr)
		{
			PartInConstraints[i]->ChildPartPtr.reset();
			PartInConstraints[i]->ParentPtr.reset();
			SAFE_DELETE(PartInConstraints[i]);
			PartInConstraints.erase(PartInConstraints.begin()+i);
			return;
		}
	}
}
// ------------------------------------ //
void Leviathan::BaseConstraintable::_OnConstraintAdded(BaseConstraint* ptr){

}

void Leviathan::BaseConstraintable::_OnConstraintUnlink(BaseConstraint* ptr){

}

void Leviathan::BaseConstraintable::_SendCreatedConstraint(BaseConstraintable* other, Entity::BaseConstraint* ptr){
    
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseConstraintable::AddConstraintWhereThisIsChild(weak_ptr<BaseConstraint>
    constraintptr)
{
	PartInConstraints.push_back(new EntitysConstraintEntry(constraintptr, this));
}

DLLEXPORT void Leviathan::BaseConstraintable::AddConstraintWhereThisIsParent(shared_ptr<BaseConstraint>
    constraintptr)
{
	PartInConstraints.push_back(new EntitysConstraintEntry(constraintptr, this));
}

DLLEXPORT void Leviathan::BaseConstraintable::OnRemoveConstraint(BaseConstraint* tmpptr){
	// this function may call destruct joint destruct so we can just erase the element //
	for(size_t i = 0; i < PartInConstraints.size(); i++){
		if(PartInConstraints[i]->IsParent ? PartInConstraints[i]->ParentPtr.get() == tmpptr: 
			PartInConstraints[i]->ChildPartPtr.lock().get() == tmpptr)
		{
			SAFE_DELETE(PartInConstraints[i]);
			PartInConstraints.erase(PartInConstraints.begin()+i);
			return;
		}
	}
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::BaseConstraintable::GetConstraintCount() const{
    return PartInConstraints.size();
}

DLLEXPORT shared_ptr<BaseConstraint> Leviathan::BaseConstraintable::GetConstraint(size_t index) const{

    GUARD_LOCK_THIS_OBJECT();

    if(PartInConstraints.size() <= index)
        return nullptr;

    return PartInConstraints[index]->IsParent ? PartInConstraints[index]->ParentPtr:
        PartInConstraints[index]->ChildPartPtr.lock();
}
// ------------------------------------ //
void Leviathan::BaseConstraintable::_OnDisowned(){

    auto end = PartInConstraints.end();
    for(auto iter = PartInConstraints.begin(); iter != end; ++iter){

        // It is enough if only the parents disown the constraints (as there cannot be a constraint between
        // entitites in different worlds
        if((*iter)->IsParent){
            
            auto ptr = (*iter)->ParentPtr;
            if(ptr)
                ptr->_WorldDisowned();
        }
    }
}
