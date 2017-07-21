#include "PongIncludes.h"
// ------------------------------------ //
#include "PongServerNetworking.h"

#include "Networking/NetworkRequest.h"
#include "Networking/Connection.h"
#include "Networking/NetworkResponse.h"
#include "Gameplay/CustomCommandHandler.h"
#include "PongServer.h"
#include "PlayerSlot.h"
using namespace Pong;
using namespace std;
// ------------------------------------ //
Pong::PongServerNetworking::PongServerNetworking() :
    NetworkServerInterface(8, "Local pong game",
        Leviathan::SERVER_JOIN_RESTRICT::None),
    ServerStatusIs(PONG_JOINGAMERESPONSE_TYPE_LOBBY)
{
    RegisterCustomCommandHandlers(_CommandHandler.get());
}

Pong::PongServerNetworking::~PongServerNetworking(){

}
// ------------------------------------ //
void Pong::PongServerNetworking::RegisterCustomCommandHandlers(CommandHandler* addhere){
    addhere->RegisterCustomCommandHandler(make_shared<PongCommandHandler>(this));
}
// ------------------------------------ //
void Pong::PongServerNetworking::_OnPlayerDisconnect(Leviathan::ConnectedPlayer* newplayer){

    auto veccy = BasePongParts::Get()->GetPlayers()->GetVec();

    auto end = veccy.end();
    for(auto iter = veccy.begin(); iter != end; ++iter){

        if((*iter)->GetConnectedPlayer() == newplayer){

            (*iter)->SlotLeavePlayer();
            BasePongParts::Get()->GetPlayers()->NotifyUpdatedValue();

            // \todo If the game is in progress update the level to block of the slot
            return;
        }
    }
}
// ------------------------------------ //
void Pong::PongServerNetworking::SetStatus(PONG_JOINGAMERESPONSE_TYPE status){

    ServerStatusIs = status;

    DEBUG_BREAK;
    //// Create an update packet //
    //shared_ptr<NetworkResponse> response(new NetworkResponse(-1,
    //        Leviathan::PACKET_TIMEOUT_STYLE_TIMEDMS, 800));
    //response->GenerateCustomResponse(new PongServerChangeStateResponse(ServerStatusIs));


    //// Send it to all of the current players //
    //SendToAllPlayers(response);
}





