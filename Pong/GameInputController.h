#ifndef PONG_INPUTCONTROLLER
#define PONG_INPUTCONTROLLER
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "PlayerSlot.h"
#include "Input\InputController.h"


namespace Pong{

	struct ControlGroup{
		ControlGroup() : ControlledSlot(NULL), CtrlGroup(PLAYERCONTROLS_NONE), ControlKeys(){
		}

		PlayerSlot* ControlledSlot;
		PLAYERCONTROLS CtrlGroup;
		std::map<OIS::KeyCode, CONTROLKEYACTION> ControlKeys;
	};

	class GameInputController : public Leviathan::InputReceiver{
	public:
		GameInputController();
		~GameInputController();


		bool StartReceivingInput(vector<PlayerSlot*> &PlayerList);

		// sets whether all input is blocked //
		void SetBlockState(bool state);

		// call when ending the match to stop input handling //
		void UnlinkPlayers();


		virtual bool ReceiveInput(OIS::KeyCode key, int modifiers, bool down);
		virtual void ReceiveBlockedInput(OIS::KeyCode key, int modifiers, bool down);
		virtual bool OnMouseMove(int xmove, int ymove);

	protected:
		void _SetupControlGroups();
		ControlGroup* _ResolveKeyToGroup(OIS::KeyCode key, CONTROLKEYACTION &returnaction);

		// ------------------------------------ //
		std::vector<ControlGroup> ControlGroups;


		bool Blocked;
		// TODO: add grouping for controller players //


	};

}
#endif