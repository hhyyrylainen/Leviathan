#ifndef PONG_INPUTCONTROLLER
#define PONG_INPUTCONTROLLER
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
// ---- includes ---- //
#include "PlayerSlot.h"
#include "Networking/NetworkedInputHandler.h"
#include "Networking/NetworkedInput.h"


namespace Pong{
	


	//! \brief The factory that handles creating the things
	class PongInputFactory : public Leviathan::NetworkInputFactory{
	public:





		//! \brief Returns the static instance
		static PongInputFactory* Get();


	protected:



		static PongInputFactory* Staticinstance;
	};


	//! \brief A single input receiver that handles a group of keys
	class PongNInputter : public Leviathan::NetworkedInput{
	public:





	protected:

		//! The slot that this controls, maybe NULL if the factory is doing something wonky
		PlayerSlot* ControlledSlot;

		//! Used for finding the right keys
		PLAYERCONTROLS CtrlGroup;
	};




	class GameInputController : public Leviathan::NetworkedInputHandler{
	public:
		GameInputController();
		~GameInputController();



		std::map<OIS::KeyCode, CONTROLKEYACTION>& MapControlsToKeyGrouping(PLAYERCONTROLS controls) THROWS;
		

	protected:
		void _SetupControlGroups();

		// ------------------------------------ //


		//! Translates a control group to a map of keys that correspond to that group
		std::map<PLAYERCONTROLS, std::map<OIS::KeyCode, CONTROLKEYACTION>> GroupToKeyMap;
	};

}
#endif