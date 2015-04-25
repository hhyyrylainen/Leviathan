//! \file This file is used to separate the declaration and implementation of BaseNotifier template class
//! This is needed because BaseNotifiable requires BaseNotifier and BaseNotifier requires it and both are templates

// ------------------------------------ //
#pragma once

#include "BaseNotifiable.h"
// ------------------------------------ //
template<class ParentType, class ChildType>
Leviathan::BaseNotifier<ParentType, ChildType>::BaseNotifier(ParentType* ourptr) : PointerToOurNotifier(ourptr){

}

template<class ParentType, class ChildType>
DLLEXPORT Leviathan::BaseNotifier<ParentType, ChildType>::~BaseNotifier(){
	GUARD_LOCK();

	// Last chance to unhook if not already //
	if(ConnectedChildren.size())
		ReleaseChildHooks();
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifier<ParentType, ChildType>::ReleaseChildHooks(){
	GUARD_LOCK();

	// Go through all and unhook them //
	while(ConnectedChildren.size()){
		// Get the iterator //
		auto iter = ConnectedChildren.begin();

		auto tmpobj = (*iter);

		GUARD_LOCK_OTHER_NAME((*iter), guard2);

		// Call unhook on the child //
		tmpobj->_OnUnhookNotifier(this);

		// Remove it //
		_OnNotifiableDisconnected(tmpobj->GetActualPointerToNotifiableObject());
		
		ConnectedChildren.erase(iter);
	}
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifier<ParentType, ChildType>::UnConnectFromNotifiable(int id){
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
DLLEXPORT bool Leviathan::BaseNotifier<ParentType, ChildType>::UnConnectFromNotifiable(
    BaseNotifiable<ParentType, ChildType>* unhookfrom, Lock &guard)
{
	VerifyLock(guard);
	GUARD_LOCK_OTHER_NAME(unhookfrom, guard2);

	// Remove from the list and call functions //
	auto end = ConnectedChildren.end();
	for(auto iter = ConnectedChildren.begin(); iter != end; ++iter){

		if(*iter == unhookfrom){
			// Call unhook on the child //
			(*iter)->_OnUnhookNotifier(this);
			// Remove it //
			_OnNotifiableDisconnected((*iter)->GetActualPointerToNotifiableObject());
			ConnectedChildren.erase(iter);
			return true;
		}
	}
	return false;
}

template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifier<ParentType, ChildType>::ConnectToNotifiable(
    BaseNotifiable<ParentType, ChildType>* child, Lock &guard)
{
	VerifyLock(guard);
	GUARD_LOCK_OTHER_NAME(child, guard2);

	// Check is it already connected //
	if(IsConnectedTo(child, guard)){

		return false;
	}

	// Call hook on the child //
	child->_OnHookNotifier(this);

	// Add to list //
	ConnectedChildren.push_back(child);

	// Finally call the callback //
	_OnNotifiableConnected(child->GetActualPointerToNotifiableObject());

	return true;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifier<ParentType, ChildType>::IsConnectedTo(
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
void Leviathan::BaseNotifier<ParentType, ChildType>::_OnHookNotifiable(
    BaseNotifiable<ParentType, ChildType>* child)
{

	// Add the object to the list of objects //
	ConnectedChildren.push_back(child);
	_OnNotifiableConnected(child->GetActualPointerToNotifiableObject());
}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::_OnUnhookNotifiable(
    BaseNotifiable<ParentType, ChildType>* childtoremove)
{

	// Remove from list //
	auto end = ConnectedChildren.end();
	for(auto iter = ConnectedChildren.begin(); iter != end; ++iter){

		if(*iter == childtoremove){
			// Remove it //
			_OnNotifiableDisconnected((*iter)->GetActualPointerToNotifiableObject());
			ConnectedChildren.erase(iter);
			return;
		}
	}
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT ParentType* Leviathan::BaseNotifier<ParentType, ChildType>::GetActualPointerToNotifierObject(){
	return PointerToOurNotifier;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifier<ParentType, ChildType>::_OnNotifiableDisconnected(ChildType* childtoremove){

}

template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifier<ParentType, ChildType>::_OnNotifiableConnected(ChildType* childadded){

}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifier<ParentType, ChildType>::NotifyAll(){
	// Notify all the children //
	GUARD_LOCK();

	auto end = ConnectedChildren.end();
	for(auto iter = ConnectedChildren.begin(); iter != end; ++iter){

		GUARD_LOCK_OTHER_NAME((*iter), guard2);
		(*iter)->OnNotified();
	}
}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::OnNotified(){

}



