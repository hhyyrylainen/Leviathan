#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASENOTIFIER
#include "BaseNotifier.h"
#endif
#include "BaseNotifiable.h"
using namespace Leviathan;
// ------------------------------------ //
template<class ParentType, class ChildType>
Leviathan::BaseNotifier<ParentType, ChildType>::BaseNotifier(ParentType* ourptr) : PointerToOurNotifier(ourptr){

}

template<class ParentType, class ChildType>
DLLEXPORT Leviathan::BaseNotifier<ParentType, ChildType>::~BaseNotifier(){
	// Last chance to unhook if not already //
	if(ConnectedChildren.size())
		ReleaseChildHooks();
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT void Leviathan::BaseNotifier<ParentType, ChildType>::ReleaseChildHooks(){
	// Go through all and unhook them //
	for(auto iter = ConnectedChildren.begin(); iter != ConnectedChildren.end(); ++iter){
		// Call unhook on the child //
		(*iter)->_OnUnhookNotifier(this);
		// Remove it //
		_OnNotifiableDisconnected((*iter)->GetActualPointerToNotifiableObject());
	}
	// Clear all at once //
	ConnectedChildren.clear();
}
// ------------------------------------ //
template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifier<ParentType, ChildType>::UnConnectFromNotifiable(int id){
	// Find child matching the provided id //
	for(auto iter = ConnectedChildren.begin(); iter != ConnectedChildren.end(); ++iter){

		if((*iter)->GetID() == id){
			// Remove it //
			return UnConnectFromNotifiable(iter);
		}
	}
	return false;
}

template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifier<ParentType, ChildType>::UnConnectFromNotifiable(BaseNotifiable<ParentType, ChildType>* unhookfrom){
	// Remove from list and call functions //
	for(auto iter = ConnectedChildren.begin(); iter != ConnectedChildren.end(); ++iter){

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
DLLEXPORT bool Leviathan::BaseNotifier<ParentType, ChildType>::ConnectToNotifiable(BaseNotifiable<ParentType, ChildType>* child){
	// Call hook on child //
	child->_OnHookNotifier(this);

	// Add to list //
	ConnectedChildren.push_back(child);

	// Finally call the callback //
	_OnNotifiableConnected(child->GetActualPointerToNotifiableObject());

	return true;
}
// ------------------------------------ //
template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::_OnHookNotifiable(BaseNotifiable<ParentType, ChildType>* child){
	// Add the object to the list of objects //
	ConnectedChildren.push_back(child);
	_OnNotifiableConnected(child->GetActualPointerToNotifiableObject());
}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifier<ParentType, ChildType>::_OnUnhookNotifiable(BaseNotifiable<ParentType, ChildType>* childtoremove){
	// Remove from list //
	for(auto iter = ConnectedChildren.begin(); iter != ConnectedChildren.end(); ++iter){

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


// ------------------ Template instances ------------------ //
template class BaseNotifier<BaseNotifierAll, BaseNotifiableAll>;
template class BaseNotifier<BaseNotifierEntity, BaseNotifiableEntity>;






