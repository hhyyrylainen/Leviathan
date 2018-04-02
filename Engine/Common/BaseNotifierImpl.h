//! \file This file is used to separate the declaration and implementation of BaseNotifier template class
//! This is needed because BaseNotifiable requires BaseNotifier and BaseNotifier requires it and both are templates

// ------------------------------------ //
#pragma once

#include "BaseNotifier.h"
#include "BaseNotifiable.h"
// ------------------------------------ //
template<class ParentType, class ChildType>
Leviathan::BaseNotifier<ParentType, ChildType>::BaseNotifier(ParentType* ourptr) : PointerToOurNotifier(ourptr){

}

template<class ParentType, class ChildType>
Leviathan::BaseNotifier<ParentType, ChildType>::~BaseNotifier(){
	GUARD_LOCK();

	// Last chance to unhook if not already //
	if(ConnectedChildren.size())
		ReleaseChildHooks(guard);
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::ReleaseChildHooks(Lock &guard){

	// Go through all and unhook them //
	while(ConnectedChildren.size()){
		// Get the iterator //
		auto iter = ConnectedChildren.begin();

		auto tmpobj = (*iter);

		GUARD_LOCK_OTHER_NAME((*iter), guard2);

		// Call unhook on the child //
		tmpobj->_OnUnhookNotifier(guard2, this, guard);

		// Remove it //
		_OnNotifiableDisconnected(guard, tmpobj->GetActualPointerToNotifiableObject(), guard2);
		
		ConnectedChildren.erase(iter);
	}
}
// ------------------------------------ //
template<class ParentType, class ChildType>
bool Leviathan::BaseNotifier<ParentType, ChildType>::UnConnectFromNotifiable(int id){
	GUARD_LOCK();

	// Find child matching the provided id //
	auto end = ConnectedChildren.end();
	for(auto iter = ConnectedChildren.begin(); iter != end; ++iter){

		if((*iter)->GetID() == id){
			// Remove it //
			// We don't need to lock the child since the called function will do that //
			return UnConnectFromNotifiable(iter, guard);
		}
	}
	return false;
}

template<class ParentType, class ChildType>
bool Leviathan::BaseNotifier<ParentType, ChildType>::UnConnectFromNotifiable(Lock &guard,
    BaseNotifiable<ParentType, ChildType>* unhookfrom)
{
	VerifyLock(guard);
	GUARD_LOCK_OTHER_NAME(unhookfrom, guard2);

	// Remove from the list and call functions //
	auto end = ConnectedChildren.end();
	for(auto iter = ConnectedChildren.begin(); iter != end; ++iter){

		if(*iter == unhookfrom){
			// Call unhook on the child //
            
			(*iter)->_OnUnhookNotifier(guard2, this, guard);
            
			// Remove it //
			_OnNotifiableDisconnected(guard, (*iter)->GetActualPointerToNotifiableObject(),
                guard2);
            
			ConnectedChildren.erase(iter);
			return true;
		}
	}
	return false;
}

template<class ParentType, class ChildType>
bool Leviathan::BaseNotifier<ParentType, ChildType>::ConnectToNotifiable(Lock &guard,
    BaseNotifiable<ParentType, ChildType>* child, Lock &childlock)
{
	// Check is it already connected //
	if(IsConnectedTo(child, guard)){

		return false;
	}

	// Call hook on the child //
	child->_OnHookNotifier(childlock, this, guard);

	// Add to list //
	ConnectedChildren.push_back(child);

	// Finally call the callback //
	_OnNotifiableConnected(guard, child->GetActualPointerToNotifiableObject(), childlock);

	return true;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
bool Leviathan::BaseNotifier<ParentType, ChildType>::IsConnectedTo(
    BaseNotifiable<ParentType, ChildType>* check, Lock &guard)
{
	VerifyLock(guard);

	auto end = ConnectedChildren.end();
	for(auto iter = ConnectedChildren.begin(); iter != end; ++iter){

		if(*iter == check)
			return true;
	}

	// Didn't find a match //
	return false;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::_OnHookNotifiable(Lock &guard, 
    BaseNotifiable<ParentType, ChildType>* child, Lock &childlock)
{

	// Add the object to the list of objects //
	ConnectedChildren.push_back(child);
	_OnNotifiableConnected(guard, child->GetActualPointerToNotifiableObject(), childlock);
}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::_OnUnhookNotifiable(Lock &guard,
    BaseNotifiable<ParentType, ChildType>* childtoremove, Lock &childlock)
{

	// Remove from list //
	auto end = ConnectedChildren.end();
	for(auto iter = ConnectedChildren.begin(); iter != end; ++iter){

		if(*iter == childtoremove){

			_OnNotifiableDisconnected(guard, (*iter)->GetActualPointerToNotifiableObject(),
                childlock);
            
			ConnectedChildren.erase(iter);
			return;
		}
	}
}
// ------------------------------------ //
template<class ParentType, class ChildType>
ParentType* Leviathan::BaseNotifier<ParentType, ChildType>::GetActualPointerToNotifierObject(){
	return PointerToOurNotifier;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::_OnNotifiableDisconnected(
    Lock &guard, ChildType* childtoremove, Lock &childlock)
{

}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::_OnNotifiableConnected(
    Lock &guard, ChildType* childadded, Lock &childlock)
{

}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::NotifyAll(Lock &guard){
	// Notify all the children //
    auto* actualptr = GetActualPointerToNotifierObject();

	auto end = ConnectedChildren.end();
	for(auto iter = ConnectedChildren.begin(); iter != end; ++iter){

		GUARD_LOCK_OTHER_NAME((*iter), guard2);
		(*iter)->OnNotified(guard2, actualptr, guard);
	}
}

template<class ParentType, class ChildType>
    void Leviathan::BaseNotifier<ParentType, ChildType>::OnNotified(Lock &ownlock,
        ChildType* child, Lock &childlock)
{

}



