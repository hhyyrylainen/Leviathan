#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_INPUTCONTROLLER
#include "GameInputController.h"
#endif
using namespace Pong;
// ------------------------------------ //
Pong::GameInputController::GameInputController() : ControlGroups(4), Blocked(false){
	// create default control groups //
	_SetupControlGroups();

	// TODO: load control configuration //
}

Pong::GameInputController::~GameInputController(){

}
// ------------------------------------ //
bool Pong::GameInputController::StartReceivingInput(vector<PlayerSlot*> &PlayerList){
	// assign PlayerSlot pointers to control groups //
	for(size_t i = 0; i < PlayerList.size(); i++){

		PlayerSlot* slot = PlayerList[i];

		while(slot != NULL){
			bool linked = false;

			// TODO: add check for controller and while at it implement controller support //
			//if(PlayerList[i]->GetControlType() == 

			for(size_t a = 0; a < ControlGroups.size(); a++){

				if(ControlGroups[a].CtrlGroup == slot->GetControlType()){
					// link! //
					ControlGroups[a].ControlledSlot = slot;
					linked = true;
					Logger::Get()->Info(L"Player "+Convert::ToWstring(slot->GetPlayerIdentifier())+L" linked!");
					break;
				}
			}
			// complain //
			if(!linked && !(slot->GetControlType() != PLAYERCONTROLS_NONE || slot->GetControlType() != PLAYERCONTROLS_AI))
				Logger::Get()->Warning(L"GameInputController: StartReceivingInput: couldn't link player "+Convert::ToWstring(slot->GetPlayerIdentifier()));
			
			// Change to potential sub slots //
			slot = slot->GetSplit();
		}
	}
	return true;
}

void Pong::GameInputController::UnlinkPlayers(){
	// unlink all control groups //
	for(size_t i = 0; i < ControlGroups.size(); i++){
		ControlGroups[i].ControlledSlot = NULL;
	}
}
// ------------------------------------ //
DLLEXPORT bool Pong::GameInputController::ReceiveInput(OIS::KeyCode key, int modifiers, bool down){
	// resolve key to a control group //
	CONTROLKEYACTION usedaction;

	ControlGroup* tmp = _ResolveKeyToGroup(key, usedaction);
	if(tmp){
		if(Blocked){
			// force up message if input is blocked //
			down = false;
		}
		// pass matching input //
		if(tmp->ControlledSlot)
			tmp->ControlledSlot->PassInputAction(usedaction, down);
		return true;
	}
	return false;
}

DLLEXPORT void Pong::GameInputController::ReceiveBlockedInput(OIS::KeyCode key, int modifiers, bool down){
	// if blocked input is up then we want to pass it //
	if(down)
		return;

	// we can call our other handling function //
	ReceiveInput(key, modifiers, false);
}
// ------------------------------------ //
void Pong::GameInputController::_SetupControlGroups(){

	ControlGroups[0].CtrlGroup = PLAYERCONTROLS_WASD;
	ControlGroups[0].ControlKeys[Window::ConvertWstringToOISKeyCode(L"A")] = CONTROLKEYACTION_LEFT;
	ControlGroups[0].ControlKeys[Window::ConvertWstringToOISKeyCode(L"D")] = CONTROLKEYACTION_RIGHT;
	ControlGroups[0].ControlKeys[Window::ConvertWstringToOISKeyCode(L"W")] = CONTROLKEYACTION_POWERUPUP;
	ControlGroups[0].ControlKeys[Window::ConvertWstringToOISKeyCode(L"S")] = CONTROLKEYACTION_POWERUPDOWN;

	ControlGroups[1].CtrlGroup = PLAYERCONTROLS_ARROWS;
	ControlGroups[1].ControlKeys[Window::ConvertWstringToOISKeyCode(L"LEFTARROW")] = CONTROLKEYACTION_LEFT;
	ControlGroups[1].ControlKeys[Window::ConvertWstringToOISKeyCode(L"RIGHTARROW")] = CONTROLKEYACTION_RIGHT;
	ControlGroups[1].ControlKeys[Window::ConvertWstringToOISKeyCode(L"UPARROW")] = CONTROLKEYACTION_POWERUPUP;
	ControlGroups[1].ControlKeys[Window::ConvertWstringToOISKeyCode(L"DOWNARROW")] = CONTROLKEYACTION_POWERUPDOWN;

	ControlGroups[2].CtrlGroup = PLAYERCONTROLS_IJKL;
	ControlGroups[2].ControlKeys[Window::ConvertWstringToOISKeyCode(L"J")] = CONTROLKEYACTION_LEFT;
	ControlGroups[2].ControlKeys[Window::ConvertWstringToOISKeyCode(L"L")] = CONTROLKEYACTION_RIGHT;
	ControlGroups[2].ControlKeys[Window::ConvertWstringToOISKeyCode(L"I")] = CONTROLKEYACTION_POWERUPUP;
	ControlGroups[2].ControlKeys[Window::ConvertWstringToOISKeyCode(L"K")] = CONTROLKEYACTION_POWERUPDOWN;

	ControlGroups[3].CtrlGroup = PLAYERCONTROLS_NUMPAD;
	ControlGroups[3].ControlKeys[Window::ConvertWstringToOISKeyCode(L"NUMPAD4")] = CONTROLKEYACTION_LEFT;
	ControlGroups[3].ControlKeys[Window::ConvertWstringToOISKeyCode(L"NUMPAD6")] = CONTROLKEYACTION_RIGHT;
	ControlGroups[3].ControlKeys[Window::ConvertWstringToOISKeyCode(L"NUMPAD8")] = CONTROLKEYACTION_POWERUPUP;
	ControlGroups[3].ControlKeys[Window::ConvertWstringToOISKeyCode(L"NUMPAD5")] = CONTROLKEYACTION_POWERUPDOWN;
}

ControlGroup* Pong::GameInputController::_ResolveKeyToGroup(OIS::KeyCode key, CONTROLKEYACTION &returnaction){
	// loop through groups and return one that the key matches //
	for(size_t i = 0; i < ControlGroups.size(); i++){
		// check if key can be found //
		auto iter = ControlGroups[i].ControlKeys.find(key);
		if(iter != ControlGroups[i].ControlKeys.end()){
			// copy action value //
			returnaction = iter->second;

			// return matching group //
			return &ControlGroups[i];
		}
	}

	// didn't match any //
	return NULL;
}

DLLEXPORT bool Pong::GameInputController::OnMouseMove(int xmove, int ymove){
	return false;
}

void Pong::GameInputController::SetBlockState(bool state){
	Blocked = state;
	// send block to all players //
	if(Blocked){
		for(size_t i = 0; i < ControlGroups.size(); i++){

			if(ControlGroups[i].ControlledSlot){

				ControlGroups[i].ControlledSlot->InputDisabled();
			}
		}
	}
}
