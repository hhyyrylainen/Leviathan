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
	// Last chance to unhook if not already //
	if(ConnectedToParents.size())
		ReleaseParentHooks();
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifiable<ParentType, ChildType>::ReleaseParentHooks(){
	ObjectLock guard(*this);

	// Go through all and unhook them //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){
		// Call unhook on the child //
		(*iter)->_OnUnhookNotifiable(this);
		// Remove it //
		_OnNotifierDisconnected((*iter)->GetActualPointerToNotifierObject());
	}
	// Clear all at once //
	ConnectedToParents.clear();
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifiable<ParentType, ChildType>::UnConnectFromNotifier(int id){
	ObjectLock guard(*this);

	// Find child matching the provided id //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){

		if((*iter)->GetID() == id){
			// Remove it //
			return UnConnectFromNotifier(iter, guard);
		}
	}
	return false;
}

template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifiable<ParentType, ChildType>::UnConnectFromNotifier(BaseNotifier<ParentType, ChildType>* specificnotifier, ObjectLock &guard){
	VerifyLock(guard);

	// Remove from list and call functions //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){

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
	ObjectLock guard(*this);

	// Call hook on parent //
	owner->_OnHookNotifiable(this);

	// Add to list //
	ConnectedToParents.push_back(owner);

	// Finally call the callback //
	_OnNotifierConnected(owner->GetActualPointerToNotifierObject());

	// \todo return false and skip adding if already added //
	return true;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnHookNotifier(BaseNotifier<ParentType, ChildType>* parent){
	ObjectLock guard(*this);
	// Add the object to the list of objects //
	ConnectedToParents.push_back(parent);
	_OnNotifierConnected(parent->GetActualPointerToNotifierObject());
}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnUnhookNotifier(BaseNotifier<ParentType, ChildType>* parent){
	ObjectLock guard(*this);
	// Remove from list //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ){

		if(*iter == parent){
			// Remove it //
			_OnNotifierDisconnected((*iter)->GetActualPointerToNotifierObject());
			iter = ConnectedToParents.erase(iter);
			return;

		} else {
			++iter;
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

// No longer required //
// ------------------ Template instances ------------------ //
//template class BaseNotifiable<BaseNotifierAll, BaseNotifiableAll>;
//template class BaseNotifiable<BaseNotifierEntity, BaseNotifiableEntity>;

#endif