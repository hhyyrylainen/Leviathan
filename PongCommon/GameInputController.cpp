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
#include "Exceptions.h"
#include "PlayerSlot.h"
#include "Networking/NetworkServerInterface.h"
#include "Networking/NetworkedInput.h"
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
	typedef std::map<OIS::KeyCode, CONTROLKEYACTION> KeyMap;
	
	// Would be nice to be able to use boost::assign here...
	
	KeyMap WASD;
	WASD.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"A"), CONTROLKEYACTION_LEFT));
	WASD.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"D"), CONTROLKEYACTION_RIGHT));
	WASD.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"W"), CONTROLKEYACTION_POWERUPUP));
	WASD.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"S"), CONTROLKEYACTION_POWERUPDOWN));
	
	KeyMap Arrows;
	Arrows.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"LEFTARROW"), CONTROLKEYACTION_LEFT));
	Arrows.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"RIGHTARROW"), CONTROLKEYACTION_RIGHT));
	Arrows.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"UPARROW"), CONTROLKEYACTION_POWERUPUP));
	Arrows.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"DOWNARROW"), CONTROLKEYACTION_POWERUPDOWN));
	
	KeyMap IJKL;
	Arrows.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"J"), CONTROLKEYACTION_LEFT));
	Arrows.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"L"), CONTROLKEYACTION_RIGHT));
	Arrows.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"I"), CONTROLKEYACTION_POWERUPUP));
	Arrows.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"K"), CONTROLKEYACTION_POWERUPDOWN));
	
	KeyMap numpad;
	numpad.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"NUMPAD4"), CONTROLKEYACTION_LEFT));
	numpad.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"NUMPAD6"), CONTROLKEYACTION_RIGHT));
	numpad.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"NUMPAD8"), CONTROLKEYACTION_POWERUPUP));
	numpad.insert(make_pair(Window::ConvertWstringToOISKeyCode(L"NUMPAD5"), CONTROLKEYACTION_POWERUPDOWN));
	
	
	GroupToKeyMap.insert(make_pair(PLAYERCONTROLS_WASD, WASD));
	GroupToKeyMap.insert(make_pair(PLAYERCONTROLS_ARROWS, Arrows));
	GroupToKeyMap.insert(make_pair(PLAYERCONTROLS_IJKL, IJKL));
	GroupToKeyMap.insert(make_pair(PLAYERCONTROLS_NUMPAD, numpad));
	
}

std::map<OIS::KeyCode, CONTROLKEYACTION>& Pong::GameInputController::MapControlsToKeyGrouping(PLAYERCONTROLS controls)
    THROWS
{

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

        // Store a pointer to the currently handled one, as subslots need to be handled //
        PlayerSlot* curply = plys[i];

        while(curply){

            if(curply->GetNetworkedInputID() == inputid){

                // Store the data and set us as this slot's thing //
                curplayer = curply;
                activecontrols = curplayer->GetControlType();
                playerid = curplayer->GetPlayerID();

                break;
            }

            // Move to the subslot //
            curply = curply->GetSplit();
        }

        // Quit if already found //
        if(playerid >= 0)
            break;
	}

	if(playerid == -1){

		Logger::Get()->Error(L"Pong input thing failed to find player ID");
		return NULL;
	}

    Logger::Get()->Info("Creating our input object: "+Convert::ToString(playerid)+", controls: "+
        Convert::ToString(activecontrols));
	
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

        // Subslots... //
        PlayerSlot* curply = plys[i];

        while(curply){

            if(curply->GetNetworkedInputID() == input->GetID()){

                // Store the data and set us as this slot's thing //
                PlayerSlot* curplayer = curply;

                tmpobj->StartSendingInput(curplayer);

                Logger::Get()->Info("Pong input linked to player");
                return;
            }

            curply = curply->GetSplit();
        }
	}


	Logger::Get()->Error(L"Pong input thing failed to link from network, finalize replication fail");
}

DLLEXPORT void Pong::PongInputFactory::NoLongerNeeded(NetworkedInput &todiscard){

	// Must still be the same type //
	PongNInputter* tmpobj = dynamic_cast<PongNInputter*>(&todiscard);

    if(!tmpobj){

        if(todiscard.GetState() == Leviathan::NETWORKEDINPUT_STATE_DESTRUCTED){

            // It's already destructed to the base class level //
            return;
        }

        // Wrong type got passed here //
        Logger::Get()->Warning(L"Wrong type passed to PongInputFactory");
        return;
    }
    
	// Unset the target if it is still set //
	GUARD_LOCK_OTHER_OBJECT(tmpobj);


	if(tmpobj->ControlledSlot && tmpobj->ControlledSlot->GetInputObj() == tmpobj){

		tmpobj->ControlledSlot->SetInputThatSendsControls(NULL);
	}

    tmpobj->ControlledSlot = NULL;
}
// ------------------------------------ //
PongInputFactory* Pong::PongInputFactory::Get(){
	return Staticinstance;
}

PongInputFactory* Pong::PongInputFactory::Staticinstance = new PongInputFactory();

// ------------------------------------ //
DLLEXPORT bool Pong::PongInputFactory::DoesServerAllowCreate(NetworkedInput* input, ConnectionInfo* connection){
	return IsConnectionAllowed(input, connection);
}

DLLEXPORT bool Pong::PongInputFactory::IsConnectionAllowedToUpdate(NetworkedInput* input, ConnectionInfo* connection){
	return IsConnectionAllowed(input, connection);
}

bool Pong::PongInputFactory::IsConnectionAllowed(NetworkedInput* input, ConnectionInfo* connection){
	// Check does the input id match the player's input that is associated with the connection //
	auto plylist = BasePongParts::Get()->GetPlayers();

	GUARD_LOCK_OTHER_OBJECT(plylist);


	std::vector<PlayerSlot*>& plys = plylist->GetVec();

	for(size_t i = 0; i < plys.size(); i++){

        // We need to loop the subslots here, too //
        PlayerSlot* curply = plys[i];

        while(curply){

            if(curply->GetNetworkedInputID() == input->GetID()){

                GUARD_LOCK_OTHER_OBJECT_NAME(curply, guard2);

                if(curply->GetConnectedPlayer()->GetConnection() == connection){

                    // It is allowed //
                    return true;
                } else {

                    // Not allowed //
                    Logger::Get()->Error("Pong input: connection is not allowed to update input "+
                        Convert::ToString(input->GetID()));
                    return false;
                }
            }

            curply = curply->GetSplit();
        }
	}
    

	Logger::Get()->Error("Pong input thing failed to find target for allow request");
	return false;
}

// ------------------ PongNInputter ------------------ //
Pong::PongNInputter::PongNInputter(int ownerid, int networkid, PlayerSlot* controlthis, PLAYERCONTROLS typetoreceive) : 
	Leviathan::NetworkedInput(ownerid, networkid), ControlledSlot(controlthis), CtrlGroup(typetoreceive),
    CreatedByUs(false), ControlStates(0), ChangedKeys(0)
{
	
}

Pong::PongNInputter::~PongNInputter(){
	GUARD_LOCK_THIS_OBJECT();
	if(ControlledSlot){
		
		// Should have been destroyed already //
        Logger::Get()->Warning("PongNInputter should have already had its slot destroyed");
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

    // We need to do the same as in the replication finalized method //
    // Now add the proper player pointer to it //
	auto plylist = BasePongParts::Get()->GetPlayers();

	GUARD_LOCK_OTHER_OBJECT_NAME(plylist, plylock);


	std::vector<PlayerSlot*>& plys = plylist->GetVec();

	for(size_t i = 0; i < plys.size(); i++){

        // Subslots... //
        PlayerSlot* curply = plys[i];

        while(curply){

            if(curply->GetNetworkedInputID() == GetID()){

                // Store the data and set us as this slot's thing //
                PlayerSlot* curplayer = curply;

                this->StartSendingInput(curplayer);

                Logger::Get()->Info("Pong input linked to local player");
                return;
            }

            curply = curply->GetSplit();
        }
	}


	Logger::Get()->Error(L"Pong input thing failed to create local");
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
			ControlledSlot->PassInputAction(CONTROLKEYACTION_POWERUPDOWN, ChangedKeys & PONG_INPUT_FLAGS_POWERDOWN ?
                true: false);
		}

		if(differences & PONG_INPUT_FLAGS_POWERUP){
			ControlledSlot->PassInputAction(CONTROLKEYACTION_POWERUPUP, ChangedKeys & PONG_INPUT_FLAGS_POWERUP ?
                true: false);
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

		throw InvalidArgument("invalid pong control packet");
	}

	CtrlGroup = static_cast<PLAYERCONTROLS>(tmpgroup);
	

	if(!(packet >> *reinterpret_cast<sf::Int8*>(&ControlStates))){

		throw InvalidArgument("invalid pong control packet");
	}
}
// ------------------------------------ //
void Pong::PongNInputter::OnAddUpdateCustomDataToPacket(sf::Packet &packet){
	GUARD_LOCK_THIS_OBJECT();

	packet << (sf::Int8)ChangedKeys;
}

void Pong::PongNInputter::OnLoadCustomUpdateDataFrompacket(sf::Packet &packet){
	GUARD_LOCK_THIS_OBJECT();

	if(!(packet >> *reinterpret_cast<sf::Int8*>(&ChangedKeys))){

		throw InvalidArgument("invalid pong control packet");
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
	case CONTROLKEYACTION_LEFT: targetbit = 0; break; // bit set in PONG_INPUT_FLAGS_LEFT
	case CONTROLKEYACTION_RIGHT: targetbit = 1; break; // bit set in PONG_INPUT_FLAGS_RIGHT
	case CONTROLKEYACTION_POWERUPDOWN: targetbit = 3; break; // bit set in PONG_INPUT_FLAGS_POWERDOWN
	case CONTROLKEYACTION_POWERUPUP: targetbit = 2; break; // bit set in PONG_INPUT_FLAGS_POWERUP
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

		ControlledSlot->SetInputThatSendsControls(this);
	}

	ControlledSlot = target;
	// The SetInputThatSendsControls should be called by our caller //
}
// ------------------------------------ //
void Pong::PongNInputter::UpdateSettings(PLAYERCONTROLS newcontrols){
	GUARD_LOCK_THIS_OBJECT();

	if(!CreatedByUs)
		return;

	CtrlGroup = newcontrols;
}

