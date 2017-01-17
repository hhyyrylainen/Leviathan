// ------------------------------------ //
#ifndef PONG_PLAYERSLOT
#include "PlayerSlot.h"
#endif
#include "Iterators/StringIterator.h"
#include "Networking/NetworkServerInterface.h"
#include "Utility/ComplainOnce.h"
#include "Entities/Components.h"

#include "CommonPong.h"

#ifdef PONG_VERSION
#include "PongGame.h"
#include "PongNetHandler.h"
#endif //PONG_VERSION

#include "GameInputController.h"
using namespace Pong;
using namespace Leviathan;
// ------------------------------------ //
Pong::PlayerSlot::PlayerSlot(int slotnumber, PlayerList* owner) :
    Slot(slotnumber), PlayerType(PLAYERTYPE_CLOSED), PlayerNumber(0), ControlType(PLAYERCONTROLS_NONE),
    ControlIdentifier(0), PlayerControllerID(0), Colour(Float4::GetColourWhite()), Score(0), 
    MoveState(0), PaddleObject(0), GoalAreaObject(0), TrackObject(0), SplitSlot(nullptr),
    ParentSlot(nullptr), InputObj(nullptr), SlotsPlayer(NULL), PlayerID(-1), NetworkedInputID(-1), 
    Parent(owner)
{

}

Pong::PlayerSlot::~PlayerSlot(){
    _ResetNetworkInput();
    SAFE_DELETE(SplitSlot);
}
// ------------------------------------ //
void Pong::PlayerSlot::Init(PLAYERTYPE type /*= PLAYERTYPE_EMPTY*/, int PlayerNumber /*= 0*/,
    PLAYERCONTROLS controltype /*= PLAYERCONTROLS_NONE*/, int ctrlidentifier /*= 0*/, int playercontrollerid /*= -1*/,
    const Float4 &playercolour /*= Float4::GetColourWhite()*/)
{
    PlayerType = type;
    PlayerNumber = PlayerNumber;
    ControlType = controltype;
    ControlIdentifier = ctrlidentifier;
    Colour = playercolour;
    PlayerControllerID = playercontrollerid;
}
// ------------------------------------ //
void Pong::PlayerSlot::SetPlayer(PLAYERTYPE type, int identifier){
    PlayerType = type;
    PlayerNumber = identifier;
}

Pong::PLAYERTYPE Pong::PlayerSlot::GetPlayerType(){
    return PlayerType;
}

int Pong::PlayerSlot::GetPlayerNumber(){
    return PlayerNumber;
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
    GUARD_LOCK();

    // if this is player 0 or 3 flip inputs //
    if(Slot == 0 || Slot == 2){

        switch(actiontoperform){
        case CONTROLKEYACTION_LEFT: actiontoperform = CONTROLKEYACTION_POWERUPUP; break;
        case CONTROLKEYACTION_POWERUPDOWN: actiontoperform = CONTROLKEYACTION_RIGHT; break;
        case CONTROLKEYACTION_RIGHT: actiontoperform = CONTROLKEYACTION_POWERUPDOWN; break;
        case  CONTROLKEYACTION_POWERUPUP: actiontoperform = CONTROLKEYACTION_LEFT; break;
        }
    }

    // set control state //
    if(active){

        if(actiontoperform == CONTROLKEYACTION_LEFT){
            MoveState = 1;
        } else if(actiontoperform == CONTROLKEYACTION_RIGHT){
            MoveState = -1;
        }

    } else {

        if(actiontoperform == CONTROLKEYACTION_LEFT && MoveState == 1){
            MoveState = 0;
        } else if(actiontoperform == CONTROLKEYACTION_RIGHT && MoveState == -1){
            MoveState = 0;
        }

    }

    try{
        DEBUG_BREAK;
        //auto& track = BasePongParts::Get()->GetGameWorld()->GetComponent<TrackController>(
        //    TrackObject);

        //// Set the track speed based on move direction //
        //track.ChangeSpeed = MoveState*INPUT_TRACK_ADVANCESPEED;
        //track.Marked = true;
    
    } catch(const NotFound&){

        Logger::Get()->Warning("PlayerSlot has a TrackObject that has no TrackController "
            "component, object: "+Convert::ToString(TrackObject));
    }
}

void Pong::PlayerSlot::InputDisabled(){

    // Reset control state //
    MoveState = 0;

    if(TrackObject == 0)
        return;
    
    // Set apply force to zero //
    try{
        DEBUG_BREAK;
        //auto& track = BasePongParts::Get()->GetGameWorld()->GetComponent<TrackController>(
        //    TrackObject);

        //// Set the track speed based on move direction //
        //track.ChangeSpeed = 0;
        //track.Marked = true;
    
    } catch(const NotFound&){

    }
}

int Pong::PlayerSlot::GetScore(){
    return Score;
}
// ------------------------------------ //
void Pong::PlayerSlot::AddEmptySubSlot(){
    SplitSlot = new PlayerSlot(this->Slot, Parent);
    // Set this as the parent //
    SplitSlot->ParentSlot = this;


}

bool Pong::PlayerSlot::IsVerticalSlot(){
    return Slot == 0 || Slot == 2;
}

float Pong::PlayerSlot::GetTrackProgress(){

    try{
        DEBUG_BREAK;
        //auto& track = BasePongParts::Get()->GetGameWorld()->GetComponent<TrackController>(
        //    TrackObject);

        //return track.NodeProgress;
    
    } catch(const NotFound&){

    }

    return 0.f;
}

bool Pong::PlayerSlot::DoesPlayerNumberMatchThisOrParent(int number){
    if(number == PlayerNumber)
        return true;
    // Recurse to parent, if one exists //
    if(ParentSlot)
        return ParentSlot->DoesPlayerNumberMatchThisOrParent(number);

    return false;
}
// ------------------------------------ //
void Pong::PlayerSlot::AddDataToPacket(sf::Packet &packet){
    GUARD_LOCK();

    // Write all our data to the packet //
    packet << Slot << (int)PlayerType << PlayerNumber << NetworkedInputID << ControlIdentifier
           << (int)ControlType << PlayerControllerID 
           << Colour;
    packet << PaddleObject << GoalAreaObject << TrackObject;

    packet << PlayerID << Score << (bool)(SplitSlot != NULL);

    if(SplitSlot){
        // Add our sub slot data //
        SplitSlot->AddDataToPacket(packet);
    }
}

void Pong::PlayerSlot::UpdateDataFromPacket(sf::Packet &packet, Lock &listlock){

    GUARD_LOCK();
    
    int tmpival;

    // Get our data from it //
    packet >> Slot >> tmpival;

    PlayerType = static_cast<PLAYERTYPE>(tmpival);

    packet >> PlayerNumber  >> NetworkedInputID  >> ControlIdentifier  >> tmpival;

    ControlType = static_cast<PLAYERCONTROLS>(tmpival);

    packet >> PlayerControllerID;
    packet >> Colour;

    packet >> PaddleObject >> GoalAreaObject >> TrackObject;
    
    packet >> PlayerID >> Score;
    

    bool wantedsplit;

    packet >> wantedsplit;

    if(!packet)
        throw InvalidArgument("packet format for PlayerSlot is invalid");

    // Check do we need to change split //
    if(wantedsplit && !SplitSlot){
        AddEmptySubSlot();

    } else if(!wantedsplit && SplitSlot){
        SAFE_DELETE(SplitSlot);
    }

    // Update split value //
    if(wantedsplit){

        SplitSlot->UpdateDataFromPacket(packet, listlock);
    }

#ifdef PONG_VERSION

    // Update everything related to input //
    if(PlayerID == PongGame::Get()->GetInterface().GetOurID()){

        // Create new one only if there isn't one already created //
        if(!InputObj){

            // Skip if it is an AI slot //
            if(ControlType != PLAYERCONTROLS_AI){

                Logger::Get()->Info("Creating input for our player id, NetworkedInputID: "+Convert::ToString(
                        NetworkedInputID));
            
                // Hook a networked input receiver to the server //
                DEBUG_BREAK;
                //PongGame::Get()->GetInputController()->RegisterNewLocalGlobalReflectingInputSource(
                //    PongGame::GetInputFactory()->CreateNewInstanceForLocalStart(guard,
                //        NetworkedInputID, true));
            }

        } else {

            if(ControlType == PLAYERCONTROLS_AI){
                // Destroy out input if there is an AI //
                _ResetNetworkInput(guard);

            } else {
                
                // Update the existing one //
                InputObj->UpdateSettings(ControlType);
                Logger::Get()->Info("Updated our control type: "+Convert::ToString(ControlType));
            }
        }
    }


#endif //PONG_VERSION


}

int Pong::PlayerSlot::GetPlayerControllerID(){
    return PlayerControllerID;
}

void Pong::PlayerSlot::SlotJoinPlayer(Leviathan::ConnectedPlayer* ply, int uniqnumber){
    SlotsPlayer = ply;

    PlayerID = SlotsPlayer->GetID();
    PlayerNumber = uniqnumber;

    PlayerType = PLAYERTYPE_HUMAN;
}

void Pong::PlayerSlot::AddServerAI(int uniquenumber, int aitype /*= 2*/){

    // TODO: properly kick //
    SlotsPlayer = NULL;

    PlayerNumber = uniquenumber;
    PlayerType = PLAYERTYPE_COMPUTER;
    
    ControlType = PLAYERCONTROLS_AI;
    ControlIdentifier = aitype;
}

void Pong::PlayerSlot::SlotLeavePlayer(){
    SlotsPlayer = NULL;
    ControlType = PLAYERCONTROLS_NONE;
    PlayerType = PLAYERTYPE_EMPTY;
}

void Pong::PlayerSlot::SetInputThatSendsControls(Lock &guard, PongNInputter* input){

    if(InputObj && input != InputObj){

        Logger::Get()->Info("PlayerSlot: destroying old networked input");
        _ResetNetworkInput(guard);
    }
    
    InputObj = input;
}

void PlayerSlot::InputDeleted(Lock &guard){

    InputObj = nullptr;
}

void Pong::PlayerSlot::_ResetNetworkInput(Lock &guard){

    if(InputObj){

        InputObj->StopSendingInput(this);
    }

    InputObj = NULL;
}

// ------------------ PlayerList ------------------ //
Pong::PlayerList::PlayerList(std::function<void (PlayerList*)> callback, size_t playercount
    /*= 4*/) :
    SyncedResource("PlayerList"), GamePlayers(4), CallbackFunc(callback)
{

    // Fill default player data //
    for(int i = 0; i < 4; i++){

        GamePlayers[i] = new PlayerSlot(i, this);
    }

}

Pong::PlayerList::~PlayerList(){
    SAFE_DELETE_VECTOR(GamePlayers);
}
// ------------------------------------ //
void Pong::PlayerList::UpdateCustomDataFromPacket(Lock &guard, sf::Packet &packet){
    
    sf::Int32 vecsize;

    if(!(packet >> vecsize)){

        throw InvalidArgument("packet format for PlayerSlot is invalid");
    }

    if(vecsize != static_cast<int>(GamePlayers.size())){
        // We need to resize //
        int difference = vecsize-GamePlayers.size();

        if(difference < 0){

            // Loop and delete items //
            int deleted = 0;
            while(deleted < difference){

                SAFE_DELETE(GamePlayers[GamePlayers.size()-1]);
                GamePlayers.pop_back();
                deleted++;
            }
        } else {
            // We need to add items //
            for(int i = 0; i < difference; i++){

                GamePlayers.push_back(new PlayerSlot(GamePlayers.size(), this));
            }
        }
    }

    // Update the data //
    auto end = GamePlayers.end();
    for(auto iter = GamePlayers.begin(); iter != end; ++iter){

        (*iter)->UpdateDataFromPacket(packet, guard);
    }
}

void Pong::PlayerList::SerializeCustomDataToPacket(Lock &guard, sf::Packet &packet){
    
    // First put the size //
    packet << static_cast<sf::Int32>(GamePlayers.size());

    // Loop through them and add them //
    for(auto iter = GamePlayers.begin(); iter != GamePlayers.end(); ++iter){

        // Serialize it //
        (*iter)->AddDataToPacket(packet);
    }
}

void Pong::PlayerList::OnValueUpdated(Lock &guard){
    
    // Call our callback //
    CallbackFunc(this);
}

PlayerSlot* Pong::PlayerList::GetSlot(size_t index){
    if(index >= GamePlayers.size())
        throw InvalidArgument("player index is out of range");

    return GamePlayers[index];
}
// ------------------------------------ //
void Pong::PlayerList::ReportPlayerInfoToLog() const{

    GUARD_LOCK();
    Logger::Get()->Write("PlayerList:::: size "+Convert::ToString(GamePlayers.size()));

    for(auto iter = GamePlayers.begin(); iter != GamePlayers.end(); ++iter){

        (*iter)->WriteInfoToLog(1);
    }
}


void Pong::PlayerSlot::WriteInfoToLog(int depth /*= 0*/) const{

    string prefix;
    if(depth >= 0)
        prefix.reserve(depth*4);

    for(int i = 0; i < depth; i++)
        prefix += "    ";

    Logger::Get()->Write(prefix+"----PlayerSlot "+Convert::ToString(Slot));

#define PUT_FIELD(x) Logger::Get()->Write(prefix+ #x +" "+Convert::ToString(x));

    PUT_FIELD(PlayerType);

    PUT_FIELD(PlayerNumber);

    PUT_FIELD(ControlType);

    PUT_FIELD(ControlIdentifier);

    PUT_FIELD(PlayerControllerID);

    PUT_FIELD(Colour);

    PUT_FIELD(Score);

    PUT_FIELD(MoveState);

    Logger::Get()->Write(prefix+"PaddleObject "+Convert::ToString(PaddleObject));

    Logger::Get()->Write(prefix + "GoalAreaObject "+Convert::ToString(GoalAreaObject));

    Logger::Get()->Write(prefix+"TrackObject "+Convert::ToString(TrackObject));

    Logger::Get()->Write(prefix+"InputObj "+Convert::ToString(InputObj));

    Logger::Get()->Write(prefix+"SlotsPlayer "+Convert::ToString(SlotsPlayer));

    PUT_FIELD(PlayerID);

    PUT_FIELD(NetworkedInputID);

    if(SplitSlot){

        Logger::Get()->Write(prefix+"====Split:");

        SplitSlot->WriteInfoToLog(depth+1);
    }
        
    Logger::Get()->Write(prefix+"----");
}
