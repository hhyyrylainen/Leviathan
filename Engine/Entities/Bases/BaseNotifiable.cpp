#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASENOTIFIABLE
#include "BaseNotifiable.h"
#endif
#include "BaseNotifier.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseNotifiable::BaseNotifiable() : BaseObject(-1, NULL){

}

DLLEXPORT Leviathan::BaseNotifiable::~BaseNotifiable(){
	// Last chance to unhook if not already //
	if(ConnectedToParents.size())
		ReleaseParentHooks();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseNotifiable::ReleaseParentHooks(){
	// Go through all and unhook them //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){
		// Call unhook on the child //
		(*iter)->_OnUnhookNotifiable(this);
		// Remove it //
		_OnNotifierDisconnected(*iter);
	}
	// Clear all at once //
	ConnectedToParents.clear();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseNotifiable::ConnectToNotifier(BaseNotifier* owner){
	// Call hook on parent //
	owner->_OnHookNotifiable(this);

	// Add to list //
	ConnectedToParents.push_back(owner);

	// Finally call the callback //
	_OnNotifierConnected(owner);

	// \todo return false and skip adding if already added //
	return true;
}

DLLEXPORT bool Leviathan::BaseNotifiable::UnConnectFromNotifier(BaseNotifier* specificnotifier){
	// Remove from list and call functions //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){

		if(*iter == specificnotifier){
			// Call unhook on the child //
			(*iter)->_OnUnhookNotifiable(this);
			// Remove it //
			_OnNotifierDisconnected(specificnotifier);
			ConnectedToParents.erase(iter);
			return true;
		}
	}
	return false;
}

DLLEXPORT bool Leviathan::BaseNotifiable::UnConnectFromNotifier(int id){
	// Find child matching the provided id //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){

		if((*iter)->GetID() == id){
			// Remove it //
			return UnConnectFromNotifier(*iter);
		}
	}
	return false;
}
// ------------------------------------ //
void Leviathan::BaseNotifiable::_OnUnhookNotifier(BaseNotifier* parent){
	// Remove from list //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){

		if(*iter == parent){
			// Remove it //
			_OnNotifierDisconnected(parent);
			ConnectedToParents.erase(iter);
			return;
		}
	}
}

void Leviathan::BaseNotifiable::_OnHookNotifier(BaseNotifier* parent){
	// Add the object to the list of objects //
	ConnectedToParents.push_back(parent);
	_OnNotifierConnected(parent);
}
// ------------------ Default callbacks ------------------ //
void Leviathan::BaseNotifiable::_OnNotifierConnected(BaseNotifier* parentadded){

}

void Leviathan::BaseNotifiable::_OnNotifierDisconnected(BaseNotifier* parenttoremove){

}
