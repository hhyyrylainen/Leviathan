// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"

#include <vector>

namespace Leviathan {

class InputController;

//! class to derive from when wanting to connect to input
class InputReceiver {
    friend InputController;

public:
    DLLEXPORT InputReceiver();
    DLLEXPORT virtual ~InputReceiver();

    //! called at the beginning of key press receive sequence (you
    //! shouldn't reset anything, since you will get blocked input
    //! if keys are released while the object isn't getting input
    DLLEXPORT virtual void BeginNewReceiveQueue();

    //! Called when the object receives a key press, return true when consumed
    DLLEXPORT virtual bool ReceiveInput(int32_t key, int modifiers, bool down) = 0;

    //! \brief This is called when input is not received
    //!
    //! basically everything should be reseted to not received
    DLLEXPORT virtual void ReceiveBlockedInput(int32_t key, int modifiers, bool down) = 0;

    //! Called the scroll wheel is used and the GUI didn't take the scroll input
    //! \param x The x side scroll. NOTE: this is the side to side scrolling
    //! \param y The y direction scroll amount. This is the usual scroll direction
    DLLEXPORT virtual bool OnScroll(int x, int y, int modifiers);

    //! when mouse is captured and is moved (relative movement is passed)
    DLLEXPORT virtual bool OnMouseMove(int xmove, int ymove) = 0;

    //! \brief If ConnectedTo is not NULL notifies it that we are no longer valid
    DLLEXPORT void _UnConnectParent(Lock& ownerlock);

protected:
    // called by input controller when certain events happen //
    void _OnConnected(InputController* owner);
    void _OnDisconnect(InputController* owner);

protected:
    InputController* ConnectedTo;
};


//! \brief Manages registering multiple InputReceiver objects to a window to easily control
//! where input is sent
class InputController : public ThreadSafe {
    friend InputReceiver;

public:
    DLLEXPORT InputController();
    DLLEXPORT ~InputController();


    DLLEXPORT void LinkReceiver(std::shared_ptr<InputReceiver> object);

    DLLEXPORT virtual void StartInputGather();

    //! Called when the GUI didn't eat the whole keypress (it only ate half of the A-press)
    DLLEXPORT virtual void OnInputGet(int32_t key, int specialmodifiers, bool down);

    //! \brief This is called when input is not received
    DLLEXPORT virtual void OnBlockedInput(int32_t key, int specialmodifiers, bool down);

    //! \copydoc Leviathan::InputReceiver::OnScroll
    DLLEXPORT virtual void OnScroll(int x, int y, int modifiers);

    //! \brief When the mouse input is captured it is passed here
    DLLEXPORT virtual void SendMouseMovement(int xmoved, int ymoved);

protected:
    //! Called by input controllers if they unlink
    DLLEXPORT virtual void _OnChildUnlink(Lock& guard, InputReceiver* child);

protected:
    std::vector<std::shared_ptr<InputReceiver>> ConnectedReceivers;
};

} // namespace Leviathan
