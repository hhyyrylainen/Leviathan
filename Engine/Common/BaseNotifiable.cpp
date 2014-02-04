#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASENOTIFIABLE
#include "BaseNotifiable.h"
#endif
#include "BaseNotifier.h"
using namespace Leviathan;
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
	// Find child matching the provided id //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){

		if((*iter)->GetID() == id){
			// Remove it //
			return UnConnectFromNotifier(iter);
		}
	}
	return false;
}

template<class ParentType, class ChildType>
DLLEXPORT bool Leviathan::BaseNotifiable<ParentType, ChildType>::UnConnectFromNotifier(BaseNotifier<ParentType, ChildType>* specificnotifier){
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
	// Add the object to the list of objects //
	ConnectedToParents.push_back(parent->GetActualPointerToNotifierObject());
	_OnNotifierConnected(parent);
}

template<class ParentType, class ChildType>
void Leviathan::BaseNotifiable<ParentType, ChildType>::_OnUnhookNotifier(BaseNotifier<ParentType, ChildType>* parent){
	// Remove from list //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){

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


// ------------------ Template instances ------------------ //
template class BaseNotifiable<BaseNotifierAll, BaseNotifiableAll>;
template class BaseNotifiable<BaseNotifierEntity, BaseNotifiableEntity>;

