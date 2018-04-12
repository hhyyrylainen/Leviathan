#include "PongIncludes.h"
// ------------------------------------ //
#include "GameInputController.h"


#include "Exceptions.h"
#include "PlayerSlot.h"
#include "Networking/NetworkServerInterface.h"
#include "Window.h"
#include "CommonPong.h"

using namespace Pong;
// ------------------------------------ //
Pong::GameInputController::GameInputController()
    
{
    // Create the default control groups //
    _SetupControlGroups();
}

Pong::GameInputController::~GameInputController(){
}
// ------------------------------------ //


// ------------------------------------ //
void Pong::GameInputController::_SetupControlGroups(){
    // typedef std::map<int32_t, CONTROLKEYACTION> KeyMap;

    DEBUG_BREAK;
    // Would be nice to be able to use boost::assign here...
    
    // KeyMap WASD;
    // WASD.insert(std::make_pair(Window::ConvertStringToKeyCode("A"), CONTROLKEYACTION_LEFT));
    // WASD.insert(std::make_pair(Window::ConvertStringToKeyCode("D"), CONTROLKEYACTION_RIGHT));
    // WASD.insert(std::make_pair(Window::ConvertStringToKeyCode("W"), CONTROLKEYACTION_POWERUPUP));
    // WASD.insert(std::make_pair(Window::ConvertStringToKeyCode("S"), CONTROLKEYACTION_POWERUPDOWN));
    
    // KeyMap Arrows;
    // Arrows.insert(std::make_pair(Window::ConvertStringToKeyCode("LEFTARROW"), CONTROLKEYACTION_LEFT));
    // Arrows.insert(std::make_pair(Window::ConvertStringToKeyCode("RIGHTARROW"), CONTROLKEYACTION_RIGHT));
    // Arrows.insert(std::make_pair(Window::ConvertStringToKeyCode("UPARROW"), CONTROLKEYACTION_POWERUPUP));
    // Arrows.insert(std::make_pair(Window::ConvertStringToKeyCode("DOWNARROW"), CONTROLKEYACTION_POWERUPDOWN));
    
    // KeyMap IJKL;
    // Arrows.insert(std::make_pair(Window::ConvertStringToKeyCode("J"), CONTROLKEYACTION_LEFT));
    // Arrows.insert(std::make_pair(Window::ConvertStringToKeyCode("L"), CONTROLKEYACTION_RIGHT));
    // Arrows.insert(std::make_pair(Window::ConvertStringToKeyCode("I"), CONTROLKEYACTION_POWERUPUP));
    // Arrows.insert(std::make_pair(Window::ConvertStringToKeyCode("K"), CONTROLKEYACTION_POWERUPDOWN));
    
    // KeyMap numpad;
    // numpad.insert(std::make_pair(Window::ConvertStringToKeyCode("NUMPAD4"), CONTROLKEYACTION_LEFT));
    // numpad.insert(std::make_pair(Window::ConvertStringToKeyCode("NUMPAD6"), CONTROLKEYACTION_RIGHT));
    // numpad.insert(std::make_pair(Window::ConvertStringToKeyCode("NUMPAD8"), CONTROLKEYACTION_POWERUPUP));
    // numpad.insert(std::make_pair(Window::ConvertStringToKeyCode("NUMPAD5"), CONTROLKEYACTION_POWERUPDOWN));
    
    
    // GroupToKeyMap.insert(std::make_pair(PLAYERCONTROLS_WASD, WASD));
    // GroupToKeyMap.insert(std::make_pair(PLAYERCONTROLS_ARROWS, Arrows));
    // GroupToKeyMap.insert(std::make_pair(PLAYERCONTROLS_IJKL, IJKL));
    // GroupToKeyMap.insert(std::make_pair(PLAYERCONTROLS_NUMPAD, numpad));
    
}

std::map<int32_t, CONTROLKEYACTION>& Pong::GameInputController::MapControlsToKeyGrouping(PLAYERCONTROLS controls)
    
{

    return GroupToKeyMap[controls];
}
// ------------------ PongNInputter ------------------ //
Pong::PongNInputter::PongNInputter(int ownerid, int networkid, PlayerSlot* controlthis, PLAYERCONTROLS typetoreceive) : 
    ControlledSlot(controlthis), CtrlGroup(typetoreceive),
    ControlStates(0), ChangedKeys(0), CreatedByUs(false)
{
    
}

Pong::PongNInputter::~PongNInputter(){

    if(ControlledSlot){
        
        // Should have been destroyed already //
        Logger::Get()->Warning("PongNInputter should have already had its slot destroyed");
    }
}

void Pong::PongNInputter::StopSendingInput(PlayerSlot* tohere){
    // Hopefully locking is not required, if added deadlocking on this thread needs to be fixed
    ControlledSlot = NULL;
}
// ------------------------------------ //
void Pong::PongNInputter::InitializeLocal(){
    GUARD_LOCK();

    CreatedByUs = true;

    // We need to do the same as in the replication finalized method //
    // Now add the proper player pointer to it //
    auto plylist = BasePongParts::Get()->GetPlayers();

    // We have to assume that the list is already locked
    // TOOD: verify the claim

    std::vector<PlayerSlot*>& plys = plylist->GetVec();

    for(size_t i = 0; i < plys.size(); i++){

        // Subslots... //
        PlayerSlot* curply = plys[i];

        while(curply){

            DEBUG_BREAK;
            //if (curply->GetNetworkedInputID() == GetID()) {
            if (curply->GetNetworkedInputID() == 0) {

                // Store the data and set us as this slot's thing //
                PlayerSlot* curplayer = curply;

                this->StartSendingInput(guard, curplayer);

                Logger::Get()->Info("Pong input linked to local player");
                return;
            }

            curply = curply->GetSplit();
        }
    }


    Logger::Get()->Error("Pong input thing failed to create local");
}
// ------------------------------------ //
void Pong::PongNInputter::_OnInputChanged(){
    GUARD_LOCK();
    
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
    GUARD_LOCK();

    packet << (int)CtrlGroup << ControlStates;
}

void Pong::PongNInputter::OnLoadCustomFullDataFrompacket(sf::Packet &packet){
    GUARD_LOCK();

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
    GUARD_LOCK();

    packet << (sf::Int8)ChangedKeys;
}

void Pong::PongNInputter::OnLoadCustomUpdateDataFrompacket(sf::Packet &packet){
    GUARD_LOCK();

    if(!(packet >> *reinterpret_cast<sf::Int8*>(&ChangedKeys))){

        throw InvalidArgument("invalid pong control packet");
    }
}
// ------------------------------------ //
bool Pong::PongNInputter::ReceiveInput(int32_t key, int modifiers, bool down){
    // Reject if not local input //
    if(!CreatedByUs)
        return false;

    return _HandleKeyThing(key, down);
}

void Pong::PongNInputter::ReceiveBlockedInput(int32_t key, int modifiers, bool down){
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
bool Pong::PongNInputter::_HandleKeyThing(int32_t key, bool down){
    // This might be the case if input hasn't been chosen //
    if(CtrlGroup == PLAYERCONTROLS_NONE)
        return false;


    auto* owner = static_cast<GameInputController*>(ConnectedTo);

    // Get the map which contains the keys that we are monitoring //
    std::map<int32_t, CONTROLKEYACTION>& mapref = 
        owner->MapControlsToKeyGrouping(CtrlGroup);
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
        default:
        {
            // Not our key //
            return false;
        }
    }

    // Set/unset the changed bit //
    if(down){

        ChangedKeys |= 1 << targetbit;

    } else {

        ChangedKeys &= ~(1 << targetbit);
    }

    // Our thing has changed! //
    DEBUG_BREAK;
    //if(ChangedKeys != ControlStates)
    //    OnUpdateInputStates();


    // It was our key which we handled //
    return true;
}

void Pong::PongNInputter::StartSendingInput(Lock &guard, PlayerSlot* target){

    if(ControlledSlot && ControlledSlot != target){

        GUARD_LOCK_OTHER_NAME(ControlledSlot, guard2);
        ControlledSlot->SetInputThatSendsControls(guard2, this);
    }

    ControlledSlot = target;
    // The SetInputThatSendsControls should be called by our caller //
}
// ------------------------------------ //
void Pong::PongNInputter::UpdateSettings(PLAYERCONTROLS newcontrols){
    GUARD_LOCK();

    if(!CreatedByUs)
        return;

    CtrlGroup = newcontrols;
}

