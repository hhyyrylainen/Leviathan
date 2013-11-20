#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASENOTIFIER
#include "BaseNotifier.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
// Virtual base constructor shouldn't be called //
DLLEXPORT Leviathan::BaseNotifier::BaseNotifier() : BaseObject(-1, NULL){

}

DLLEXPORT Leviathan::BaseNotifier::~BaseNotifier(){
	// Last chance to unhook if not already //
	if(ConnectedChilds.size())
		ReleaseChildHooks();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseNotifier::ReleaseChildHooks(){
	// Go through all and unhook them //
	for(auto iter = ConnectedChilds.begin(); iter != ConnectedChilds.end(); ++iter){
		// Call unhook on the child //
		(*iter)->_OnUnhookNotifier(this);
		// Remove it //
		_OnNotifiableDisconnected(*iter);
	}
	// Clear all at once //
	ConnectedChilds.clear();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseNotifier::ConnectToNotifiable(BaseNotifiable* child){
	// Call hook on child //
	child->_OnHookNotifier(this);

	// Add to list //
	ConnectedChilds.push_back(child);

	// Finally call the callback //
	_OnNotifiableConnected(child);

	// TODO: return false and skip adding if already added //
	return true;
}

DLLEXPORT bool Leviathan::BaseNotifier::UnConnectFromNotifiable(BaseNotifiable* unhookfrom){
	// Remove from list and call functions //
	for(auto iter = ConnectedChilds.begin(); iter != ConnectedChilds.end(); ++iter){

		if(*iter == unhookfrom){
			// Call unhook on the child //
			(*iter)->_OnUnhookNotifier(this);
			// Remove it //
			_OnNotifiableDisconnected(unhookfrom);
			ConnectedChilds.erase(iter);
			return true;
		}
	}
	return false;
}

DLLEXPORT bool Leviathan::BaseNotifier::UnConnectFromNotifiable(int id){
	// Find child matching the provided id //
	for(auto iter = ConnectedChilds.begin(); iter != ConnectedChilds.end(); ++iter){

		if((*iter)->GetID() == id){
			// Remove it //
			return UnConnectFromNotifiable(*iter);
		}
	}
	return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseNotifier::SendCustomMessageToChildren(int messagetype, void* data, bool callonall /*= true*/){
	// If the loop won't stop to first successful call this is needed to know if any calls succeeded //
	bool called = false;

	for(auto iter = ConnectedChilds.begin(); iter != ConnectedChilds.end(); ++iter){

		if((*iter)->SendCustomMessage(messagetype, data)){
			called = true;
			if(!callonall)
				return true;
		}
	}

	return called;
}
// ------------------------------------ //
void Leviathan::BaseNotifier::_OnUnhookNotifiable(BaseNotifiable* childtoremove){
	// Remove from list //
	for(auto iter = ConnectedChilds.begin(); iter != ConnectedChilds.end(); ++iter){

		if(*iter == childtoremove){
			// Remove it //
			_OnNotifiableDisconnected(childtoremove);
			ConnectedChilds.erase(iter);
			return;
		}
	}
}

void Leviathan::BaseNotifier::_OnHookNotifiable(BaseNotifiable* child){
	// Add the object to the list of objects //
	ConnectedChilds.push_back(child);
	_OnNotifiableConnected(child);
}
// ------------------ Default callbacks ------------------ //
void Leviathan::BaseNotifier::_OnNotifiableConnected(BaseNotifiable* childadded){

}

void Leviathan::BaseNotifier::_OnNotifiableDisconnected(BaseNotifiable* childtoremove){

}


