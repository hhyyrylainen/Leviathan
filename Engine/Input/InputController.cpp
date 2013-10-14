#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_INPUTCONTROLLER
#include "InputController.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::InputController::InputController(){

}

DLLEXPORT Leviathan::InputController::~InputController(){
	// unlink all children //
	for(size_t i = 0; i < ConnectedReceivers.size(); i++){
		// call disconnect function first //
		ConnectedReceivers[i]->_OnDisconnect(this);
		ConnectedReceivers.erase(ConnectedReceivers.begin()+i);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::InputController::StartInputGather(){
	// reset internal state //

	// tell listeners that new frame begins //
	for(size_t i = 0; i < ConnectedReceivers.size(); i++){

		ConnectedReceivers[i]->BeginNewReceiveQueue();
	}
}

DLLEXPORT void Leviathan::InputController::OnInputGet(OIS::KeyCode key, int specialmodifiers, bool down){
	// call on first that gets it //
	bool received = false;

	for(size_t i = 0; i < ConnectedReceivers.size(); i++){

		if(received){

			ConnectedReceivers[i]->ReceiveBlockedInput(key, specialmodifiers, down);
			continue;
		}

		if(ConnectedReceivers[i]->ReceiveInput(key, specialmodifiers, down))
			received = true;
	}
}

DLLEXPORT void Leviathan::InputController::OnBlockedInput(OIS::KeyCode key, int specialmodifiers, bool down){
	// call on all //
	for(size_t i = 0; i < ConnectedReceivers.size(); i++){

		ConnectedReceivers[i]->ReceiveBlockedInput(key, specialmodifiers, down);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::InputController::LinkReceiver(InputReceiver* object){
	// just add to list and call link function //
	object->_OnConnected(this);
	ConnectedReceivers.push_back(object);
}
// ------------------------------------ //
void Leviathan::InputController::_OnChildUnlink(InputReceiver* child){
	for(size_t i = 0; i < ConnectedReceivers.size(); i++){
		if(ConnectedReceivers[i] == child){
			// call disconnect function first //
			ConnectedReceivers[i]->_OnDisconnect(this);
			ConnectedReceivers.erase(ConnectedReceivers.begin()+i);
			return;
		}
	}
}
// ------------------------------------ //



// ------------------ InputReceiver ------------------ //
Leviathan::InputReceiver::~InputReceiver(){
	// tell owner that we no longer exist //
	if(ConnectedTo)
		ConnectedTo->_OnChildUnlink(this);
}
// ------------------------------------ //
void Leviathan::InputReceiver::_OnConnected(InputController* owner){
	ConnectedTo = owner;
}

void Leviathan::InputReceiver::_OnDisconnect(InputController* owner){
	ConnectedTo = NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::InputReceiver::BeginNewReceiveQueue(){

}
