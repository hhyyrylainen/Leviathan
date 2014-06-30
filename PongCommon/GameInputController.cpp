#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_INPUTCONTROLLER
#include "GameInputController.h"
#endif
#ifdef PONG_VERSION
#include "PongGame.h"
#include "PongNetHandler.h"
#else
#include "PongServer.h"
#include "PongServerNetworking.h"
#endif // PONG_VERSION
using namespace Pong;
#include "Exceptions/ExceptionInvalidArgument.h"
#include "PlayerSlot.h"
#include "Networking/NetworkServerInterface.h"
// ------------------------------------ //
Pong::GameInputController::GameInputController() : NetworkedInputHandler(PongInputFactory::Get(), 
#ifdef PONG_VERSION
	PongGame::Get()->GetInterface()
#else
	PongServer::Get()->GetServerNetworkInterface()
#endif // PONG_VERSION
	)
{
	// Create the default control groups //
	_SetupControlGroups();
	Staticistance = this;
}

Pong::GameInputController::~GameInputController(){
	Staticistance = NULL;
}
// ------------------------------------ //




// ------------------------------------ //
void Pong::GameInputController::_SetupControlGroups(){

	GroupToKeyMap = boost::assign::map_list_of(PLAYERCONTROLS_WASD, boost::assign::map_list_of
		(Window::ConvertWstringToOISKeyCode(L"A"), CONTROLKEYACTION_LEFT)
		(Window::ConvertWstringToOISKeyCode(L"D"), CONTROLKEYACTION_RIGHT)
		(Window::ConvertWstringToOISKeyCode(L"W"), CONTROLKEYACTION_POWERUPUP)
		(Window::ConvertWstringToOISKeyCode(L"S"), CONTROLKEYACTION_POWERUPDOWN)
		)(PLAYERCONTROLS_ARROWS, boost::assign::map_list_of
		(Window::ConvertWstringToOISKeyCode(L"LEFTARROW"), CONTROLKEYACTION_LEFT)
		(Window::ConvertWstringToOISKeyCode(L"RIGHTARROW"), CONTROLKEYACTION_RIGHT)
		(Window::ConvertWstringToOISKeyCode(L"UPARROW"), CONTROLKEYACTION_POWERUPUP)
		(Window::ConvertWstringToOISKeyCode(L"DOWNARROW"), CONTROLKEYACTION_POWERUPDOWN)
		)(PLAYERCONTROLS_IJKL, boost::assign::map_list_of
		(Window::ConvertWstringToOISKeyCode(L"J"), CONTROLKEYACTION_LEFT)
		(Window::ConvertWstringToOISKeyCode(L"L"), CONTROLKEYACTION_RIGHT)
		(Window::ConvertWstringToOISKeyCode(L"I"), CONTROLKEYACTION_POWERUPUP)
		(Window::ConvertWstringToOISKeyCode(L"K"), CONTROLKEYACTION_POWERUPDOWN)
		)(PLAYERCONTROLS_NUMPAD, boost::assign::map_list_of
		(Window::ConvertWstringToOISKeyCode(L"NUMPAD4"), CONTROLKEYACTION_LEFT)
		(Window::ConvertWstringToOISKeyCode(L"NUMPAD6"), CONTROLKEYACTION_RIGHT)
		(Window::ConvertWstringToOISKeyCode(L"NUMPAD8"), CONTROLKEYACTION_POWERUPUP)
		(Window::ConvertWstringToOISKeyCode(L"NUMPAD5"), CONTROLKEYACTION_POWERUPDOWN)
		);
}

std::map<OIS::KeyCode, CONTROLKEYACTION>& Pong::GameInputController::MapControlsToKeyGrouping(PLAYERCONTROLS controls) THROWS{

	return GroupToKeyMap[controls];
}

GameInputController* Pong::GameInputController::Get(){
	return Staticistance;
}

GameInputController* Pong::GameInputController::Staticistance = NULL;

// ------------------ PongInputFactory ------------------ //
DLLEXPORT unique_ptr<NetworkedInput> Pong::PongInputFactory::CreateNewInstanceForLocalStart(int inputid, bool isclient){
#ifdef PONG_VERSION
	// We need to find the corresponding player with the control id matching this and then stealing it here //
	auto plylist = BasePongParts::Get()->GetPlayers();

	GUARD_LOCK_OTHER_OBJECT(plylist);


	std::vector<PlayerSlot*>& plys = plylist->GetVec();


	PlayerSlot* curplayer;
	PLAYERCONTROLS activecontrols;
	int playerid = -1;

	for(size_t i = 0; i < plys.size(); i++){

		if(plys[i]->GetNetworkedInputID() == inputid){

			// Store the data and set us as this slot's thing //
			curplayer = plys[i];
			activecontrols = curplayer->GetControlType();
			playerid = curplayer->GetPlayerID();

			

			break;
		}
	}

	if(playerid == -1){

		Logger::Get()->Error(L"Pong input thing failed to find player ID");
		return NULL;
	}
	
	unique_ptr<PongNInputter> tmpobj(new PongNInputter(playerid, inputid, curplayer, activecontrols));

	curplayer->SetInputThatSendsControls(tmpobj.get());

	return unique_ptr<NetworkedInput>(tmpobj.release());
#else

	assert(0 && "Cannot call this on the server");
	return NULL;
#endif // PONG_VERSION
}

DLLEXPORT unique_ptr<NetworkedInput> Pong::PongInputFactory::CreateNewInstanceForReplication(int inputid, int ownerid){
	// We still don't know whether this will be accepted or not so we avoid expensive linking here //
	return unique_ptr<NetworkedInput>(new PongNInputter(ownerid, inputid, NULL, PLAYERCONTROLS_NONE));
}

DLLEXPORT void Pong::PongInputFactory::ReplicationFinalized(NetworkedInput* input){
	// Now add the proper player pointer to it //
	auto plylist = BasePongParts::Get()->GetPlayers();

	GUARD_LOCK_OTHER_OBJECT(plylist);


	std::vector<PlayerSlot*>& plys = plylist->GetVec();


	PongNInputter* tmpobj = dynamic_cast<PongNInputter*>(input);
	
	for(size_t i = 0; i < plys.size(); i++){

		if(plys[i]->GetNetworkedInputID() == input->GetID()){

			// Store the data and set us as this slot's thing //
			PlayerSlot* curplayer = plys[i];

			tmpobj->StartSendingInput(curplayer);
			break;
		}
	}


	Logger::Get()->Error(L"Pong input thing failed to link from network");
}

DLLEXPORT void Pong::PongInputFactory::NoLongerNeeded(NetworkedInput &todiscard){
	// Must still be the same type //
	PongNInputter* tmpobj = dynamic_cast<PongNInputter*>(&todiscard);

	// Unset the target if it is still set //
	GUARD_LOCK_OTHER_OBJECT(tmpobj);


	if(tmpobj->ControlledSlot){

		tmpobj->ControlledSlot->SetInputThatSendsControls(NULL, tmpobj);
		tmpobj->ControlledSlot = NULL;
	}
}
// ------------------------------------ //
PongInputFactory* Pong::PongInputFactory::Get(){
	return Staticinstance;
}

DLLEXPORT bool Pong::PongInputFactory::DoesServerAllowCreate(NetworkedInput* input, ConnectionInfo* connection){
	// Check does the input id match the player's input that is associated with the connection //
	auto plylist = BasePongParts::Get()->GetPlayers();

	GUARD_LOCK_OTHER_OBJECT(plylist);


	std::vector<PlayerSlot*>& plys = plylist->GetVec();

	for(size_t i = 0; i < plys.size(); i++){

		if(plys[i]->GetNetworkedInputID() == input->GetID()){

			GUARD_LOCK_OTHER_OBJECT_NAME(plys[i], guard2);

			if(plys[i]->GetConnectedPlayer()->GetConnection() == connection){

				// It is allowed //
				return true;
			} else {

				// Not allowed //
				return false;
			}
		}
	}

	Logger::Get()->Error(L"Pong input thing failed to find target for allow request");
	// Failed to find the target //
	return false;
}

PongInputFactory* Pong::PongInputFactory::Staticinstance = new PongInputFactory();
// ------------------ PongNInputter ------------------ //
Pong::PongNInputter::PongNInputter(int ownerid, int networkid, PlayerSlot* controlthis, PLAYERCONTROLS typetoreceive) : 
	Leviathan::NetworkedInput(ownerid, networkid), ControlledSlot(controlthis), CtrlGroup(typetoreceive), CreatedByUs(false), ControlStates(0),
	ChangedKeys(0)
{
	
}

Pong::PongNInputter::~PongNInputter(){
	GUARD_LOCK_THIS_OBJECT();
	if(ControlledSlot){
		
		
		ControlledSlot->SetInputThatSendsControls(NULL, this);
		ControlledSlot = NULL;
	}
}

void Pong::PongNInputter::StopSendingInput(PlayerSlot* tohere){
	GUARD_LOCK_THIS_OBJECT();
	ControlledSlot = NULL;
}
// ------------------------------------ //
DLLEXPORT void Pong::PongNInputter::InitializeLocal(){
	GUARD_LOCK_THIS_OBJECT();

	CreatedByUs = true;
}
// ------------------------------------ //
void Pong::PongNInputter::_OnInputChanged(){
	GUARD_LOCK_THIS_OBJECT();
	
	if(ControlledSlot){
		// Check which keys have changed //
		char differences = ChangedKeys^ControlStates;



		// Send our input actions //

		// Check is each control changed and if it is send the new value //
		if(differences & PONG_INPUT_FLAGS_LEFT){
			ControlledSlot->PassInputAction(CONTROLKEYACTION_LEFT, ChangedKeys & PONG_INPUT_FLAGS_LEFT ? true: false);
		}

		if(differences & PONG_INPUT_FLAGS_RIGHT){
			ControlledSlot->PassInputAction(CONTROLKEYACTION_RIGHT, ChangedKeys & PONG_INPUT_FLAGS_RIGHT ? true: false);
		}

		if(differences & PONG_INPUT_FLAGS_POWERDOWN){
			ControlledSlot->PassInputAction(CONTROLKEYACTION_POWERUPDOWN, ChangedKeys & PONG_INPUT_FLAGS_POWERDOWN ? true: false);
		}

		if(differences & PONG_INPUT_FLAGS_POWERUP){
			ControlledSlot->PassInputAction(CONTROLKEYACTION_POWERUPUP, ChangedKeys & PONG_INPUT_FLAGS_POWERUP ? true: false);
		}
		

		// All changes are now processed //
		ControlStates = ChangedKeys;
	}
}
// ------------------------------------ //
void Pong::PongNInputter::OnAddFullCustomDataToPacket(sf::Packet &packet){
	GUARD_LOCK_THIS_OBJECT();

	packet << (int)CtrlGroup << ControlStates;
}

void Pong::PongNInputter::OnLoadCustomFullDataFrompacket(sf::Packet &packet){
	GUARD_LOCK_THIS_OBJECT();

	int tmpgroup;

	if(!(packet >> tmpgroup)){

		throw ExceptionInvalidArgument(L"invalid pong control packet", 0, __WFUNCTION__, L"packet", L"");
	}

	CtrlGroup = static_cast<PLAYERCONTROLS>(tmpgroup);
	

	if(!(packet >> *reinterpret_cast<sf::Int8*>(&ControlStates))){

		throw ExceptionInvalidArgument(L"invalid pong control packet", 0, __WFUNCTION__, L"packet", L"");
	}
}
// ------------------------------------ //
void Pong::PongNInputter::OnAddUpdateCustomDataToPacket(sf::Packet &packet){
	GUARD_LOCK_THIS_OBJECT();

	packet << ChangedKeys;
}

void Pong::PongNInputter::OnLoadCustomUpdateDataFrompacket(sf::Packet &packet){
	GUARD_LOCK_THIS_OBJECT();

	if(!(packet >> *reinterpret_cast<sf::Int8*>(&ChangedKeys))){

		throw ExceptionInvalidArgument(L"invalid pong control packet", 0, __WFUNCTION__, L"packet", L"");
	}
}
// ------------------------------------ //
bool Pong::PongNInputter::ReceiveInput(OIS::KeyCode key, int modifiers, bool down){
	// Reject if not local input //
	if(!CreatedByUs)
		return false;

	return _HandleKeyThing(key, down);
}

void Pong::PongNInputter::ReceiveBlockedInput(OIS::KeyCode key, int modifiers, bool down){
	// Ignore if not local input also ignore if it is going down //
	if(!CreatedByUs || down)
		return;

	_HandleKeyThing(key, false);
}

bool Pong::PongNInputter::OnMouseMove(int xmove, int ymove){
	// Reject if not local input //
	if(!CreatedByUs)
		return false;

	// We don't use mouse input //
	return false;
}
// ------------------------------------ //
bool Pong::PongNInputter::_HandleKeyThing(OIS::KeyCode key, bool down){
	// This might be the case if input hasn't been chosen //
	if(CtrlGroup == PLAYERCONTROLS_NONE)
		return false;


	// Get the map which contains the keys that we are monitoring //
	std::map<OIS::KeyCode, CONTROLKEYACTION>& mapref = GameInputController::Get()->MapControlsToKeyGrouping(CtrlGroup);
	auto iter = mapref.find(key);

	// The iter is valid if this is a key we should be concerned by //
	if(iter == mapref.end()){

		// Not one of our keys //
		return false;
	}

	// It is our key //
	CONTROLKEYACTION newaction = iter->second;

	char targetbit;

	// Set the bit //
	switch(newaction){
	case CONTROLKEYACTION_LEFT: targetbit = PONG_INPUT_FLAGS_LEFT; break;
	case CONTROLKEYACTION_RIGHT: targetbit = PONG_INPUT_FLAGS_RIGHT; break;
	case CONTROLKEYACTION_POWERUPDOWN: targetbit = PONG_INPUT_FLAGS_POWERDOWN; break;
	case CONTROLKEYACTION_POWERUPUP: targetbit = PONG_INPUT_FLAGS_POWERUP; break;
	}

	// Set/unset the changed bit //
	if(down){

		ChangedKeys |= 1 << targetbit;

	} else {

		ChangedKeys &= ~(1 << targetbit);
	}


	// Our thing has changed! //
	if(ChangedKeys != ControlStates)
		OnUpdateInputStates();


	// It was our key which we handled //
	return true;
}

void Pong::PongNInputter::StartSendingInput(PlayerSlot* target){

	GUARD_LOCK_THIS_OBJECT();

	if(ControlledSlot){

		ControlledSlot->SetInputThatSendsControls(NULL, this);
	}

	ControlledSlot = target;
	// The SetInputThatSendsControls should be called by our caller //
}

