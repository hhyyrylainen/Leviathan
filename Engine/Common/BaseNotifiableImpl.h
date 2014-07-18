//! \file This file is used to separate the declaration and implementation of BaseNotifiable template class
//! This is needed because BaseNotifier requires BaseNotifiable and BaseNotifiable requires it and both are templates

// ------------------------------------ //
#pragma once
#ifndef LEVIATHAN_BASENOTIFIABLE_IMPL
#define LEVIATHAN_BASENOTIFIABLE_IMPL

#include "BaseNotifier.h"
// ------------------------------------ //
template<class ParentType, class ChildType>
Leviathan::BaseNotifiable<ParentType, ChildType>::BaseNotifiable(ChildType* ourptr) : PointerToOurNotifiable(ourptr){

}

template<class ParentType, class ChildType>
DLLEXPORT Leviathan::BaseNotifiable<ParentType, ChildType>::~BaseNotifiable(){
	GUARD_LOCK_THIS_OBJECT();

	// Last chance to unhook if not already //
	if(ConnectedToParents.size())
		ReleaseParentHooks();
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifiable<ParentType, ChildType>::ReleaseParentHooks(){
	// This needs a bit of trickery since the lock order must be parent, child so we may not lock ourselves
	UNIQUE_LOCK_THIS_OBJECT();

	while(!ConnectedToParents.empty()){

		// Get the parent and erase it from the vector, this should avoid problems during this object is unlocked //
		auto parentptr = *ConnectedToParents.begin();
		ConnectedToParents.erase(ConnectedToParents.begin());

		lockit.unlock();

		// Lock the parent //
		GUARD_LOCK_OTHER_OBJECT(parentptr);

		// Now that the parent is locked we can re-lock ourselves //
		lockit.lock();

		parentptr->_OnUnhookNotifiable(this);

		// Remove it //
		_OnNotifierDisconnected(parentptr->GetActualPointerToNotifierObject());
	}
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifiable<ParentType, ChildType>::UnConnectFromNotifier(int id){

	// Used to force the specific locking order //
	BaseNotifier<ParentType, ChildType>* foundtarget = NULL;

	{
		GUARD_LOCK_THIS_OBJECT();

		// Find child matching the provided id //
		auto end = ConnectedToParents.end();
		for(auto iter = ConnectedToParents.begin(); iter != end; ++iter){

			if((*iter)->GetID() == id){
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

	GUARD_LOCK_OTHER_OBJECT_NAME(foundtarget, guard2);
	GUARD_LOCK_THIS_OBJECT();

	return UnConnectFromNotifier(iter, guard);
}

template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifiable<ParentType, ChildType>::UnConnectFromNotifier(BaseNotifier<ParentType, ChildType>* specificnotifier, 
	ObjectLock &guard)
{
	VerifyLock(guard);

	// Remove from the list and call functions //
	auto end = ConnectedToParents.end();
	for(auto iter = ConnectedToParents.begin(); iter != end; ++iter){

		if(*iter == specificnotifier){
			// Call unhook on the child //
			(*iter)->_OnUnhookNotifiable(this);
			// Remove it //
			_OnNotifierDisconnected((*iter)->GetActualPointerToNotifierObject());
			ConnectedToParents.erase(iter);
			return true;
		}
	}
	return false;
}

template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifiable<ParentType, ChildType>::ConnectToNotifier(BaseNotifier<ParentType, ChildType>* owner){
	// Lock the other first //
	GUARD_LOCK_OTHER_OBJECT_NAME(owner, guard2);
	GUARD_LOCK_THIS_OBJECT();

	// Check is it already connected //
	if(IsConnectedTo(child, guard)){

		return false;
	}

	// Call hook on the parent //
	owner->_OnHookNotifiable(this);

	// Add to the list //
	ConnectedToParents.push_back(owner);

	// Finally call the callback //
	_OnNotifierConnected(owner->GetActualPointerToNotifierObject());


	return true;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifiable<ParentType, ChildType>::IsConnectedTo(BaseNotifier<ParentType, ChildType>* check, ObjectLock &guard){
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
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnHookNotifier(BaseNotifier<ParentType, ChildType>* parent){

	// Add the object to the list of objects //
	ConnectedToParents.push_back(parent);
	_OnNotifierConnected(parent->GetActualPointerToNotifierObject());
}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnUnhookNotifier(BaseNotifier<ParentType, ChildType>* parent){

	// Remove from list //
	auto end = ConnectedToParents.end();
	for(auto iter = ConnectedToParents.begin(); iter != end; ++iter){

		if(*iter == parent){
			// Remove it //
			_OnNotifierDisconnected((*iter)->GetActualPointerToNotifierObject());
			ConnectedToParents.erase(iter);
			return;

		}
	}
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnNotifierDisconnected(ParentType* parenttoremove){

}

template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnNotifierConnected(ParentType* parentadded){

}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT ChildType* Leviathan::BaseNotifiable<ParentType, ChildType>::GetActualPointerToNotifiableObject(){
	return PointerToOurNotifiable;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifiable<ParentType, ChildType>::NotifyAll(){
	// More trickery needed here to keep the locking order //
	UNIQUE_LOCK_THIS_OBJECT();

	auto currentparent = *ConnectedToParents.begin();

	while(currentparent){
		
		// Now we need to unlock and the lock the parent //
		lockit.unlock();
		{
			GUARD_LOCK_OTHER_OBJECT(currentparent);

			// Now that the parent is locked we can re-lock ourselves //
			lockit.lock();

			// Notify now //
			currentparent->OnNotified();

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
void Leviathan::BaseNotifiable<ParentType, ChildType>::OnNotified(){

}



#endif