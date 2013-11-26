#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_PLAYERSLOT
#include "PlayerSlot.h"
#endif
#include "Entities\Bases\BasePhysicsObject.h"
using namespace Pong;
// ------------------------------------ //
Pong::PlayerSlot::PlayerSlot(int slotnumber, bool empty) : Slot(slotnumber), PlayerType(PLAYERTYPE_EMPTY), PlayerIdentifier(-1), 
	ControlType(PLAYERCONTROLS_NONE), ControlIdentifier(-1), SplitSlot(NULL), Score(0), MoveState(0), TrackDirectptr(NULL), Colour(0.f)
{

}

Pong::PlayerSlot::PlayerSlot(int slotnumber, PLAYERTYPE type, int playeridentifier, PLAYERCONTROLS controltype, int ctrlidentifier, 
	const Float4 &playercolour): Slot(slotnumber), PlayerType(type), PlayerIdentifier(playeridentifier), ControlType(controltype), 
	ControlIdentifier(ctrlidentifier), SplitSlot(NULL), Score(0), MoveState(0), TrackDirectptr(NULL), Colour(playercolour)
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

	// Set the track speed based on move direction //
	TrackDirectptr->SetTrackAdvanceSpeed(MoveState*INPUT_TRACK_ADVANCESPEED);
}

void Pong::PlayerSlot::InputDisabled(){
	// set apply force to zero //
	if(PaddleObject){

		TrackDirectptr->SetTrackAdvanceSpeed(0.f);
	}

	// reset control state //
	MoveState = 0;
}

int Pong::PlayerSlot::GetScore(){
	return Score;
}

void Pong::PlayerSlot::SetColourFromRML(string rml){

	Float4 colourparsed(1.f, 1.f, 1.f, 1.f);

	WstringIterator iter(Convert::StringToWstring(rml));

	auto ret = iter.GetNextNumber(Leviathan::DECIMALSEPARATORTYPE_DOT);

	if(ret){

		colourparsed.X = Convert::WstringToFloat(*ret)/255.f;
	}

	ret = iter.GetNextNumber(Leviathan::DECIMALSEPARATORTYPE_DOT);

	if(ret){

		colourparsed.Y = Convert::WstringToFloat(*ret)/255.f;
	}

	ret = iter.GetNextNumber(Leviathan::DECIMALSEPARATORTYPE_DOT);

	if(ret){

		colourparsed.Z = Convert::WstringToFloat(*ret)/255.f;
	}

	ret = iter.GetNextNumber(Leviathan::DECIMALSEPARATORTYPE_DOT);

	if(ret){

		colourparsed.W = Convert::WstringToFloat(*ret)/255.f;
	}

	// Set the actual colour //
	SetColour(colourparsed);
}



