#ifndef LEVIATHAN_INPUTCONTROLLER
#define LEVIATHAN_INPUTCONTROLLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class InputController;

	// class to derive from when wanting to connect to input //
	class InputReceiver{
		friend InputController;
	public:

		virtual ~InputReceiver();


	protected:
		// called by input controller when certain events happen //
		void _OnConnected(InputController* owner);
		void _OnDisconnect(InputController* owner);


		// ------------------------------------ //
		InputController* ConnectedTo;

	};


	class InputController : public Object{
		friend InputReceiver;
	public:
		DLLEXPORT InputController();
		DLLEXPORT ~InputController();


		DLLEXPORT void LinkReceiver(InputReceiver* object);

		DLLEXPORT void StartInputGather();

		DLLEXPORT void OnInputGet(OIS::KeyCode key, int specialmodifiers, bool down);
		// this is called when input is not received, basically everything should be reseted to not received //
		DLLEXPORT void OnBlockedInput(OIS::KeyCode key, int specialmodifiers, bool down);



	protected:
		// called by input controllers if they unlink //
		void _OnChildUnlink(InputReceiver* child);

		// ------------------------------------ //
		bool SentBlockedEvent;

		std::vector<InputReceiver*> ConnectedReceivers;
	};

}
#endif