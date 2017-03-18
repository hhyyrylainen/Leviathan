//! \file This file is used to separate the declaration and implementation of BaseNotifiable template class
//! This is needed because BaseNotifier requires BaseNotifiable and BaseNotifiable requires it and both are templates

// ------------------------------------ //
#pragma once

#include "BaseNotifiable.h"
#include "BaseNotifier.h"
// ------------------------------------ //
template<class ParentType, class ChildType>
Leviathan::BaseNotifiable<ParentType, ChildType>::BaseNotifiable(ChildType* ourptr) : PointerToOurNotifiable(ourptr){

}

template<class ParentType, class ChildType>
Leviathan::BaseNotifiable<ParentType, ChildType>::~BaseNotifiable(){
	GUARD_LOCK();

	// Last chance to unhook //
	if(ConnectedToParents.size())
		ReleaseParentHooks(guard);
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::ReleaseParentHooks(Lock &guard){
	// This needs a bit of trickery since the lock order must be parent, child so we may not lock ourselves

	while(!ConnectedToParents.empty()){

		// Get the parent and erase it from the vector, this should avoid problems during this
        // object is unlocked
		auto parentptr = *ConnectedToParents.begin();
		ConnectedToParents.erase(ConnectedToParents.begin());

		guard.unlock();

		// Lock the parent //
		GUARD_LOCK_OTHER_NAME(parentptr, guard2);

		// Now that the parent is locked we can re-lock ourselves //
		guard.lock();

		parentptr->_OnUnhookNotifiable(guard2, this, guard);

		// Remove it //
		_OnNotifierDisconnected(guard, parentptr->GetActualPointerToNotifierObject(), guard2);
	}
}
// ------------------------------------ //
template<class ParentType, class ChildType>
bool Leviathan::BaseNotifiable<ParentType, ChildType>::UnConnectFromNotifier(int id){

	// Used to force the specific locking order //
	BaseNotifier<ParentType, ChildType>* foundtarget = NULL;

	{
		GUARD_LOCK();

		// Find child matching the provided id //
		auto end = ConnectedToParents.end();
		for(auto iter = ConnectedToParents.begin(); iter != end; ++iter){

			if(iter->GetID() == id){
				// Found the target //
				foundtarget = *iter;
			}
		}
	}

	if(!foundtarget){
		// Didn't find it //
		return false;
	}

	// Found a target, do the locking in the right order //
	
	GUARD_LOCK_OTHER_NAME(foundtarget, guard2);
	GUARD_LOCK();
	
	return UnConnectFromNotifier(guard, foundtarget, guard2);
}

template<class ParentType, class ChildType>
bool Leviathan::BaseNotifiable<ParentType, ChildType>::UnConnectFromNotifier(Lock &guard,
    BaseNotifier<ParentType, ChildType>* specificnotifier, Lock &notifierlock)
{
	VerifyLock(guard);

	// Remove from the list and call functions //
	auto end = ConnectedToParents.end();
	for(auto iter = ConnectedToParents.begin(); iter != end; ++iter){

		if(*iter == specificnotifier){
			// Call unhook on the child //
            (*iter)->_OnUnhookNotifiable(notifierlock, this, guard);

			// Remove it //
			_OnNotifierDisconnected(guard, (*iter)->GetActualPointerToNotifierObject(),
                notifierlock);
            
			ConnectedToParents.erase(iter);
			return true;
		}
	}
	return false;
}

template<class ParentType, class ChildType>
bool Leviathan::BaseNotifiable<ParentType, ChildType>::ConnectToNotifier(
    BaseNotifier<ParentType, ChildType>* owner)
{
	// Lock the other first //
	GUARD_LOCK_OTHER_NAME(owner, guard2);
	GUARD_LOCK();

	// Check is it already connected //
	if(IsConnectedTo(owner, guard)){

		return false;
	}

	// Call hook on the parent //
	owner->_OnHookNotifiable(guard2, this, guard);

	// Add to the list //
	ConnectedToParents.push_back(owner);

	// Finally call the callback //
	_OnNotifierConnected(guard, owner->GetActualPointerToNotifierObject(), guard2);


	return true;
}

template<class ParentType, class ChildType>
bool Leviathan::BaseNotifiable<ParentType, ChildType>::ConnectToNotifier(Lock &unlockable,
    BaseNotifier<ParentType, ChildType>* owner)
{
    unlockable.unlock();
    
	// Lock the other first //
	GUARD_LOCK_OTHER_NAME(owner, guard2);

    unlockable.lock();

	// Check is it already connected //
	if(IsConnectedTo(owner, unlockable)){

		return false;
	}

	// Call hook on the parent //
	owner->_OnHookNotifiable(guard2, this, unlockable);

	// Add to the list //
	ConnectedToParents.push_back(owner);

	// Finally call the callback //
	_OnNotifierConnected(unlockable, owner->GetActualPointerToNotifierObject(), guard2);

	return true;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
bool Leviathan::BaseNotifiable<ParentType, ChildType>::IsConnectedTo(
    BaseNotifier<ParentType, ChildType>* check, Lock &guard)
{
	VerifyLock(guard);

	auto end = ConnectedToParents.end();
	for(auto iter = ConnectedToParents.begin(); iter != end; ++iter){

		if(*iter == check)
			return true;
	}

	// Didn't find a match //
	return false;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnHookNotifier(Lock &locked,
    BaseNotifier<ParentType, ChildType>* parent, Lock &parentlock)
{

	// Add the object to the list of objects //
	ConnectedToParents.push_back(parent);
	_OnNotifierConnected(locked, parent->GetActualPointerToNotifierObject(), parentlock);
}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnUnhookNotifier(Lock &locked,
    BaseNotifier<ParentType, ChildType>* parent, Lock &parentlock)
{

	// Remove from list //
	auto end = ConnectedToParents.end();
	for(auto iter = ConnectedToParents.begin(); iter != end; ++iter){

		if(*iter == parent){
			// Remove it //
			_OnNotifierDisconnected(locked, (*iter)->GetActualPointerToNotifierObject(),
                parentlock);
            
			ConnectedToParents.erase(iter);
			return;
		}
	}
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnNotifierDisconnected(
    Lock &guard, ParentType* parenttoremove, Lock &parentlock)
{

}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnNotifierConnected(
    Lock &guard, ParentType* parentadded, Lock &parentlock)
{

}
// ------------------------------------ //
template<class ParentType, class ChildType>
ChildType* Leviathan::BaseNotifiable<ParentType, ChildType>::GetActualPointerToNotifiableObject(){
	return PointerToOurNotifiable;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::NotifyAll(Lock &guard){
	// More trickery needed here to keep the locking order //
	auto currentparent = *ConnectedToParents.begin();
    auto actualptr = GetActualPointerToNotifiableObject();

	while(currentparent){
		
		// Now we need to unlock and the lock the parent //
		guard.unlock();
		{
			GUARD_LOCK_OTHER_NAME(currentparent, guard2);

			// Now that the parent is locked we can re-lock ourselves //
			guard.lock();

			// Notify now //
            // TODO: add guard2 to this call
			currentparent->OnNotified(guard2, actualptr, guard);

		}
		// The current parent doesn't need to be locked while we change to a different parent //

		// Seek back to the current position //
		auto end = ConnectedToParents.end();
		for(auto iter = ConnectedToParents.begin(); iter != end; ){
			// Check did we find the parent we are after, otherwise continue //
			if(*iter == currentparent){
				// The next parent will be the next target //
				++iter;

				currentparent = iter != ConnectedToParents.end() ? *iter: NULL;
				break;

			} else {
				++iter;
			}
		}
	}
}

template<class ParentType, class ChildType>
    void Leviathan::BaseNotifiable<ParentType, ChildType>::OnNotified(Lock &ownlock,
        ParentType* parent, Lock &parentlock)
{

}


