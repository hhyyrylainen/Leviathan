#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASECONSTRAINTABLE
#include "BaseContraintable.h"
#endif
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
// I hope that this virtual inherited class' constructor won't run from here //
DLLEXPORT Leviathan::Entity::BaseContraintable::BaseContraintable() : BaseObject(-1, NULL){

}

DLLEXPORT Leviathan::Entity::BaseContraintable::~BaseContraintable(){
	SAFE_DELETE_VECTOR(PartInConstraints);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::BaseContraintable::UnlinkContraint(shared_ptr<BaseContraint> constraintptr){
	for(size_t i = 0; i < PartInConstraints.size(); i++){
		if(PartInConstraints[i]->ParentPtr == constraintptr){

			SAFE_DELETE(PartInConstraints[i]);
			PartInConstraints.erase(PartInConstraints.begin()+i);
			return true;
		}
	}
	return false;
}

DLLEXPORT void Leviathan::Entity::BaseContraintable::AggressiveConstraintUnlink(){
	// destroy the vector, which will run destructors that destroy all constraints //
	SAFE_DELETE_VECTOR(PartInConstraints);
}
// ------------------------------------ //
DLLEXPORT shared_ptr<BaseContraint> Leviathan::Entity::BaseContraintable::GetConstraintPtr(BaseContraint* unsafeptr){
	// returns this object's copy of the ptr if it is found //
	for(size_t i = 0; i < PartInConstraints.size(); i++){
		if(PartInConstraints[i]->ParentPtr.get() == unsafeptr){

			return PartInConstraints[i]->ParentPtr;
		}
	}
	return NULL;
}
// ------------------------------------ //
void Leviathan::Entity::BaseContraintable::ConstraintDestroyedRemove(BaseContraint* ptr){
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
void Leviathan::Entity::BaseContraintable::_OnConstraintAdded(BaseContraint* ptr){

}

void Leviathan::Entity::BaseContraintable::_OnConstraintUnlink(BaseContraint* ptr){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::BaseContraintable::AddConstraintWhereThisIsChild(weak_ptr<BaseContraint> constraintptr){
	PartInConstraints.push_back(new EntitysContraintEntry(constraintptr, this));
}

DLLEXPORT void Leviathan::Entity::BaseContraintable::AddContraintWhereThisIsParent(shared_ptr<BaseContraint> constraintptr){
	PartInConstraints.push_back(new EntitysContraintEntry(constraintptr, this));
}

DLLEXPORT void Leviathan::Entity::BaseContraintable::OnRemoveConstraint(BaseContraint* tmpptr){
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


