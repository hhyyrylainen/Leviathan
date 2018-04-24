// ------------------------------------ //
#include "InputController.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT InputController::InputController() {}

DLLEXPORT InputController::~InputController()
{
    // Unlink all children //
    for(size_t i = 0; i < ConnectedReceivers.size(); i++) {
        // Call disconnect function first //
        ConnectedReceivers[i]->_OnDisconnect(this);
    }

    ConnectedReceivers.clear();
}
// ------------------------------------ //
DLLEXPORT void InputController::StartInputGather()
{
    GUARD_LOCK();
    // reset internal state //

    // tell listeners that new frame begins //
    for(size_t i = 0; i < ConnectedReceivers.size(); i++) {

        ConnectedReceivers[i]->BeginNewReceiveQueue();
    }
}

DLLEXPORT void InputController::OnInputGet(int32_t key, int specialmodifiers, bool down)
{
    // Call on first that wants it //
    bool received = false;

    GUARD_LOCK();

    for(size_t i = 0; i < ConnectedReceivers.size(); i++) {

        if(received) {

            ConnectedReceivers[i]->ReceiveBlockedInput(key, specialmodifiers, down);
            continue;
        }

        if(ConnectedReceivers[i]->ReceiveInput(key, specialmodifiers, down))
            received = true;
    }
}

DLLEXPORT void InputController::OnBlockedInput(int32_t key, int specialmodifiers, bool down)
{
    GUARD_LOCK();

    // call on all //
    for(size_t i = 0; i < ConnectedReceivers.size(); i++) {

        ConnectedReceivers[i]->ReceiveBlockedInput(key, specialmodifiers, down);
    }
}

DLLEXPORT void InputController::OnScroll(int x, int y, int modifiers)
{
    // Call on first that wants it //
    GUARD_LOCK();

    for(size_t i = 0; i < ConnectedReceivers.size(); i++) {

        if(ConnectedReceivers[i]->OnScroll(x, y, modifiers))
            return;
    }
}


DLLEXPORT void InputController::SendMouseMovement(int xmoved, int ymoved)
{

    GUARD_LOCK();

    // call on first that gets it //
    for(size_t i = 0; i < ConnectedReceivers.size(); i++) {

        if(ConnectedReceivers[i]->OnMouseMove(xmoved, ymoved))
            return;
    }
}
// ------------------------------------ //
DLLEXPORT void InputController::LinkReceiver(std::shared_ptr<InputReceiver> object)
{

    GUARD_LOCK();

    // Just add to list and call link function //
    object->_OnConnected(this);

    ConnectedReceivers.push_back(object);
}
// ------------------------------------ //
DLLEXPORT void InputController::_OnChildUnlink(Lock& guard, InputReceiver* child)
{
    for(size_t i = 0; i < ConnectedReceivers.size(); i++) {
        if(ConnectedReceivers[i].get() == child) {
            // call disconnect function first //
            ConnectedReceivers[i]->_OnDisconnect(this);
            ConnectedReceivers.erase(ConnectedReceivers.begin() + i);
            return;
        }
    }
}
// ------------------ InputReceiver ------------------ //
DLLEXPORT InputReceiver::InputReceiver() : ConnectedTo(nullptr) {}

InputReceiver::~InputReceiver()
{
    // tell owner that we no longer exist //
    LEVIATHAN_ASSERT(!ConnectedTo, "InputReceiver not _UnConnectParent called");
}
// ------------------------------------ //
DLLEXPORT bool InputReceiver::OnScroll(int x, int y, int modifiers)
{
    return false;
}
// ------------------------------------ //
DLLEXPORT void InputReceiver::_UnConnectParent(Lock& ownerlock)
{

    if(ConnectedTo)
        ConnectedTo->_OnChildUnlink(ownerlock, this);

    ConnectedTo = NULL;
}
// ------------------------------------ //
void InputReceiver::_OnConnected(InputController* owner)
{
    ConnectedTo = owner;
}

void InputReceiver::_OnDisconnect(InputController* owner)
{
    ConnectedTo = NULL;
}
// ------------------------------------ //
DLLEXPORT void InputReceiver::BeginNewReceiveQueue() {}
