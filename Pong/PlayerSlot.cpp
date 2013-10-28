#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_PLAYERSLOT
#include "PlayerSlot.h"
#endif
#include "Entities\Bases\BasePhysicsObject.h"
using namespace Pong;
// ------------------------------------ //
Pong::PlayerSlot::PlayerSlot(int slotnumber, bool empty) : Slot(slotnumber), PlayerType(PLAYERTYPE_EMPTY), PlayerIdentifier(-1), 
	ControlType(PLAYERCONTROLS_NONE), ControlIdentifier(-1), SplitSlot(NULL), Score(0), MoveState(0)
{

}

Pong::PlayerSlot::PlayerSlot(int slotnumber, PLAYERTYPE type, int playeridentifier, PLAYERCONTROLS controltype, int ctrlidentifier): Slot(slotnumber), 
	PlayerType(type), PlayerIdentifier(playeridentifier), ControlType(controltype), ControlIdentifier(ctrlidentifier), SplitSlot(NULL), Score(0), 
	MoveState(0)
{

}

Pong::PlayerSlot::~PlayerSlot(){
	SAFE_DELETE(SplitSlot);
}
// ------------------------------------ //
void Pong::PlayerSlot::SetPlayer(PLAYERTYPE type, int identifier){
	PlayerType = type;
	PlayerIdentifier = identifier;
}

Pong::PLAYERTYPE Pong::PlayerSlot::GetPlayerType(){
	return PlayerType;
}

int Pong::PlayerSlot::GetPlayerIdentifier(){
	return PlayerIdentifier;
}
// ------------------------------------ //
void Pong::PlayerSlot::SetControls(PLAYERCONTROLS type, int identifier){
	ControlType = type;
	ControlIdentifier = identifier;
}

Pong::PLAYERCONTROLS Pong::PlayerSlot::GetControlType(){
	return ControlType;
}

int Pong::PlayerSlot::GetControlIdentifier(){
	return ControlIdentifier;
}
// ------------------------------------ //
int Pong::PlayerSlot::GetSlotNumber(){
	return Slot;
}
// ------------------------------------ //
int Pong::PlayerSlot::GetSplitCount(){
	return SplitSlot != NULL ? 1+SplitSlot->GetSplitCount(): 0;
}

void Pong::PlayerSlot::PassInputAction(CONTROLKEYACTION actiontoperform, bool active){
	// if this is player 0 or 3 flip inputs //
	if(Slot == 0 || Slot == 2){

		switch(actiontoperform){
		case CONTROLKEYACTION_LEFT: actiontoperform = CONTROLKEYACTION_POWERUPDOWN; break;
		case CONTROLKEYACTION_POWERUPDOWN: actiontoperform = CONTROLKEYACTION_LEFT; break;
		case CONTROLKEYACTION_RIGHT: actiontoperform = CONTROLKEYACTION_POWERUPUP; break;
		case  CONTROLKEYACTION_POWERUPUP: actiontoperform = CONTROLKEYACTION_RIGHT; break;
		}
	}

	// set control state //
	if(active){

		if(actiontoperform == CONTROLKEYACTION_LEFT){
			MoveState = -1;
		} else if(actiontoperform == CONTROLKEYACTION_RIGHT){
			MoveState = 1;
		}

	} else {

		if(actiontoperform == CONTROLKEYACTION_LEFT && MoveState == -1){
			MoveState = 0;
		} else if(actiontoperform == CONTROLKEYACTION_RIGHT && MoveState == 1){
			MoveState = 0;
		}

	}

	// update apply force //
	if(MoveState == 0){

		if(PaddleObject){

			// paddle should be fine to cast dynamically to physics object //
			Leviathan::BasePhysicsObject* tmp = dynamic_cast<Leviathan::BasePhysicsObject*>(PaddleObject.get());
			if(tmp != NULL){
				tmp->RemoveApplyForce(L"");
				// stop the paddle //
				tmp->SetBodyVelocity(Float3(0));
			}
		}

	} else {

		Float3 dir;

		if(MoveState == -1){
			// if this is player 0 or 3 flip directions //
			if(Slot == 0 || Slot == 2){
				dir = Float3(0.f, 0.f, 1.f)*INPUTFORCE_APPLYSCALE;
			} else {
				dir = Float3(-1.f, 0.f, 0.f)*INPUTFORCE_APPLYSCALE;
			}

		} else {
			// MoveState == 1 should be true here //
			// if this is player 0 or 3 flip directions //
			if(Slot == 0 || Slot == 2){
				dir = Float3(0.f, 0.f, -1.f)*INPUTFORCE_APPLYSCALE;
			} else {
				dir = Float3(1.f, 0.f, 0.f)*INPUTFORCE_APPLYSCALE;
			}

		}

		// apply to object //
		if(PaddleObject){

			// paddle should be fine to cast dynamically to physics object //
			Leviathan::BasePhysicsObject* tmp = dynamic_cast<Leviathan::BasePhysicsObject*>(PaddleObject.get());
			if(tmp != NULL){
				
				tmp->ApplyForce(new Leviathan::ApplyForceInfo(dir, true, true, NULL));
			}
		}
	}
}

void Pong::PlayerSlot::InputDisabled(){
	// set apply force to zero //
	if(PaddleObject){

		// paddle should be fine to cast dynamically to physics object //
		Leviathan::BasePhysicsObject* tmp = dynamic_cast<Leviathan::BasePhysicsObject*>(PaddleObject.get());
		if(tmp != NULL){
			tmp->RemoveApplyForce(L"");
		}
	}

	// reset control state //
	MoveState = 0;
}

int Pong::PlayerSlot::GetScore(){
	return Score;
}



