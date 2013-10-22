#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_PLAYERSLOT
#include "PlayerSlot.h"
#endif
using namespace Pong;
// ------------------------------------ //
Pong::PlayerSlot::PlayerSlot(int slotnumber, bool empty) : Slot(slotnumber), PlayerType(PLAYERTYPE_EMPTY), PlayerIdentifier(-1), 
	ControlType(PLAYERCONTROLS_NONE), ControlIdentifier(-1), SplitSlot(NULL)
{

}

Pong::PlayerSlot::PlayerSlot(int slotnumber, PLAYERTYPE type, int playeridentifier, PLAYERCONTROLS controltype, int ctrlidentifier): Slot(slotnumber), 
	PlayerType(type), PlayerIdentifier(playeridentifier), ControlType(controltype), ControlIdentifier(ctrlidentifier), SplitSlot(NULL)
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

