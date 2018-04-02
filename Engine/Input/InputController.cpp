// ------------------------------------ //
#include "InputController.h"

#include "../Exceptions.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::InputController::InputController(){

}

DLLEXPORT Leviathan::InputController::~InputController(){
    // unlink all children //
    for(size_t i = 0; i < ConnectedReceivers.size(); i++){
        // call disconnect function first //
        ConnectedReceivers[i]->_OnDisconnect(this);
    }
    
    ConnectedReceivers.clear();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::InputController::StartInputGather(){

    GUARD_LOCK();
    // reset internal state //

    // tell listeners that new frame begins //
    for(size_t i = 0; i < ConnectedReceivers.size(); i++){

        ConnectedReceivers[i]->BeginNewReceiveQueue();
    }
}

DLLEXPORT void Leviathan::InputController::OnInputGet(int32_t key, int specialmodifiers,
    bool down)
{
    // call on first that gets it //
    bool received = false;

    GUARD_LOCK();
    
    for(size_t i = 0; i < ConnectedReceivers.size(); i++){

        if(received){

            ConnectedReceivers[i]->ReceiveBlockedInput(key, specialmodifiers, down);
            continue;
        }

        if(ConnectedReceivers[i]->ReceiveInput(key, specialmodifiers, down))
            received = true;
    }
}

DLLEXPORT void Leviathan::InputController::OnBlockedInput(int32_t key, int specialmodifiers,
    bool down)
{

    GUARD_LOCK();
    
    // call on all //
    for(size_t i = 0; i < ConnectedReceivers.size(); i++){

        ConnectedReceivers[i]->ReceiveBlockedInput(key, specialmodifiers, down);
    }
}


DLLEXPORT void Leviathan::InputController::SendMouseMovement(int xmoved, int ymoved){

    GUARD_LOCK();
    
    // call on first that gets it //
    for(size_t i = 0; i < ConnectedReceivers.size(); i++){

        if(ConnectedReceivers[i]->OnMouseMove(xmoved, ymoved))
            return;
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::InputController::LinkReceiver(std::shared_ptr<InputReceiver> object){

    GUARD_LOCK();
    
    // Just add to list and call link function //
    object->_OnConnected(this);

    ConnectedReceivers.push_back(object);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::InputController::_OnChildUnlink(Lock &guard, InputReceiver* child){
    for(size_t i = 0; i < ConnectedReceivers.size(); i++){
        if(ConnectedReceivers[i].get() == child){
            // call disconnect function first //
            ConnectedReceivers[i]->_OnDisconnect(this);
            ConnectedReceivers.erase(ConnectedReceivers.begin()+i);
            return;
        }
    }
}
// ------------------ InputReceiver ------------------ //
DLLEXPORT Leviathan::InputReceiver::InputReceiver() : ConnectedTo(nullptr){

}

Leviathan::InputReceiver::~InputReceiver(){
    // tell owner that we no longer exist //
    LEVIATHAN_ASSERT(!ConnectedTo, "InputReceiver not _UnConnectParent called");
}
// ------------------------------------ //
DLLEXPORT void InputReceiver::_UnConnectParent(Lock &ownerlock){

    if(ConnectedTo)
        ConnectedTo->_OnChildUnlink(ownerlock, this);

    ConnectedTo = NULL;
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


