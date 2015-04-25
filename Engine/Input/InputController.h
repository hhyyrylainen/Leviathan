#ifndef LEVIATHAN_INPUTCONTROLLER
#define LEVIATHAN_INPUTCONTROLLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "OISKeyboard.h"


namespace Leviathan{

	class InputController;

	// class to derive from when wanting to connect to input //
	class InputReceiver{
		friend InputController;
		friend NetworkedInputHandler;
	public:
		DLLEXPORT InputReceiver();
		DLLEXPORT virtual ~InputReceiver();

		// called at the beginning of key press receive sequence (you shouldn't reset anything, since you will get blocked input if keys are released
		// while the object isn't getting input
		DLLEXPORT virtual void BeginNewReceiveQueue();

		// Called when the object receives a key press, return true when consumed //
		DLLEXPORT virtual bool ReceiveInput(OIS::KeyCode key, int modifiers, bool down) = 0;
		DLLEXPORT virtual void ReceiveBlockedInput(OIS::KeyCode key, int modifiers, bool down) = 0;

		// when mouse is captured and is moved (relative movement is passed) //
		DLLEXPORT virtual bool OnMouseMove(int xmove, int ymove) = 0;

	protected:
		// called by input controller when certain events happen //
		void _OnConnected(InputController* owner);
		void _OnDisconnect(InputController* owner);


		// ------------------------------------ //
		InputController* ConnectedTo;

	};


	class InputController{
		friend InputReceiver;
	public:
		DLLEXPORT InputController();
		DLLEXPORT ~InputController();


		DLLEXPORT void LinkReceiver(InputReceiver* object);

		DLLEXPORT virtual void StartInputGather();

		DLLEXPORT virtual void OnInputGet(OIS::KeyCode key, int specialmodifiers, bool down);
		// this is called when input is not received, basically everything should be reseted to not received //
		DLLEXPORT virtual void OnBlockedInput(OIS::KeyCode key, int specialmodifiers, bool down);

		DLLEXPORT virtual void SendMouseMovement(int xmoved, int ymoved);

	protected:
		// called by input controllers if they unlink //
		virtual void _OnChildUnlink(InputReceiver* child);

		// ------------------------------------ //

		std::vector<InputReceiver*> ConnectedReceivers;
	};

}
#endif
