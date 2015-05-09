// ------------------------------------ //
#include "BaseConstraintable.h"

using namespace Leviathan;
using namespace Entity;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseConstraintable::BaseConstraintable() : BaseObject(-1, NULL){

}

DLLEXPORT Leviathan::BaseConstraintable::~BaseConstraintable(){
    GUARD_LOCK_NAME(lockit);
    
    AggressiveConstraintUnlink(lockit);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseConstraintable::UnlinkConstraint(
    shared_ptr<BaseConstraint> constraintptr, Lock &lockit)
{
    VerifyLock(lockit);
    
	for(size_t i = 0; i < PartInConstraints.size(); i++){
		if(PartInConstraints[i]->ParentPtr == constraintptr){

            auto constraint = PartInConstraints[i];
			PartInConstraints.erase(PartInConstraints.begin()+i);

            lockit.unlock();
            
            SAFE_DELETE(constraint);

            lockit.lock();

			return true;
		}
	}
    
	return false;
}

DLLEXPORT void Leviathan::BaseConstraintable::AggressiveConstraintUnlink(Lock &lockit){

    VerifyLock(lockit);
    
    while(!PartInConstraints.empty()){

        auto constraint = PartInConstraints.back();
        PartInConstraints.pop_back();

        lockit.unlock();
        
        SAFE_DELETE(constraint);

        lockit.lock();
    }
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<BaseConstraint> Leviathan::BaseConstraintable::GetConstraintPtr(BaseConstraint* unsafeptr){

    GUARD_LOCK();

    // Returns this object's copy of the ptr if it is found //
	for(size_t i = 0; i < PartInConstraints.size(); i++){
        
		if(PartInConstraints[i]->ParentPtr.get() == unsafeptr){

			return PartInConstraints[i]->ParentPtr;
		}
	}
    
	return NULL;
}
// ------------------------------------ //
void Leviathan::BaseConstraintable::ConstraintDestroyedRemove(BaseConstraint* ptr, Lock &lockit){

    VerifyLock(lockit);
    
	// This function may not call destruct, so we need to reset the ptrs by hand //
	for(size_t i = 0; i < PartInConstraints.size(); i++){
        
		if(PartInConstraints[i]->IsParent ? PartInConstraints[i]->ParentPtr.get() == ptr: 
			PartInConstraints[i]->ChildPartPtr.lock().get() == ptr)
		{
            auto constraint = PartInConstraints[i];
            
			constraint->ChildPartPtr.reset();
			constraint->ParentPtr.reset();

			PartInConstraints.erase(PartInConstraints.begin()+i);

            lockit.unlock();
            
            SAFE_DELETE(constraint);

            lockit.lock();
            
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
    GUARD_LOCK();
	PartInConstraints.push_back(new EntitysConstraintEntry(constraintptr, this));
}

DLLEXPORT void Leviathan::BaseConstraintable::AddConstraintWhereThisIsParent(shared_ptr<BaseConstraint>
    constraintptr)
{
    GUARD_LOCK();
	PartInConstraints.push_back(new EntitysConstraintEntry(constraintptr, this));
}

DLLEXPORT void Leviathan::BaseConstraintable::OnRemoveConstraint(BaseConstraint* tmpptr,
    Lock &lockit)
{

    VerifyLock(lockit);
    
	// This function may call destruct joint destruct so we can just erase the element //
	for(size_t i = 0; i < PartInConstraints.size(); i++){
        
		if(PartInConstraints[i]->IsParent ? PartInConstraints[i]->ParentPtr.get() == tmpptr: 
			PartInConstraints[i]->ChildPartPtr.lock().get() == tmpptr)
		{
            auto constraint = PartInConstraints[i];
			PartInConstraints.erase(PartInConstraints.begin()+i);

            lockit.unlock();
            
			SAFE_DELETE(constraint);

            lockit.lock();
			return;
		}
	}
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::BaseConstraintable::GetConstraintCount(Lock &guard) const{

    return PartInConstraints.size();
}

DLLEXPORT std::shared_ptr<BaseConstraint> Leviathan::BaseConstraintable::GetConstraint(Lock &guard,
    size_t index) const
{
    if(PartInConstraints.size() <= index)
        return nullptr;

    return PartInConstraints[index]->IsParent ? PartInConstraints[index]->ParentPtr:
        PartInConstraints[index]->ChildPartPtr.lock();
}
// ------------------------------------ //
void Leviathan::BaseConstraintable::_OnDisowned(){

    GUARD_LOCK();
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
