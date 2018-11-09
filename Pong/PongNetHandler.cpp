#include "PongIncludes.h"
// ------------------------------------ //
#include "PongNetHandler.h"

#include "Networking/NetworkResponse.h"
#include "PongPackets.h"
#include "Networking/NetworkRequest.h"
#include "Networking/Connection.h"
#include "Engine.h"
#include "GUI/GuiManager.h"
#include "PongGame.h"
using namespace Pong;
// ------------------------------------ //
Pong::PongNetHandler::PongNetHandler(){

}

Pong::PongNetHandler::~PongNetHandler(){

}
// ------------------------------------ //
void Pong::PongNetHandler::_OnProperlyConnected(){
    Logger::Get()->Info("Pong ready to join the lobby or the game");

    DEBUG_BREAK;
    // Send our custom join request packet //
    //shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(new Leviathan::CustomRequestData(new PongJoinGameRequest())));

    //shared_ptr<SentNetworkThing> waitforthis = ServerConnection->SendPacketToConnection(tmprequest, 5);


    //// Start waiting for a response //
    //Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new Leviathan::ConditionalTask(
    //    boost::bind<void>([](shared_ptr<SentNetworkThing> packetobject, PongNetHandler* instance) -> void
    //{
    //    // Check did it succeed //
    //    if(!packetobject->GetStatus() || !packetobject->GotResponse){

    //        // It failed //
    //        instance->DisconnectFromServer("Failed to receive response to a join lobby/match request");
    //        return;
    //    }

    //    // Check where we got (lobby, match, end screen) //
    //    Leviathan::NetworkResponseDataForCustom* directdata = packetobject->GotResponse->GetResponseDataForGameSpecific();

    //    // Check is it invalid for this request //
    //    if(!directdata || !directdata->ActualPacketData || !directdata->ActualPacketData->ResponseBaseData || 
    //        directdata->ActualPacketData->TypeIDNumber != PONG_PACKET_JOINGAME_RESPONSE)
    //    {
    //        // It failed //
    //        instance->DisconnectFromServer("Received an invalid response to a join game request");
    //        return;
    //    }


    //    // Now connected //
    //    PongJoinGameResponse* tmpresponseobj = dynamic_cast<PongJoinGameResponse*>(directdata->ActualPacketData->ResponseBaseData);
    //    assert(tmpresponseobj && "the packet factory is being wonky");

    //    switch(tmpresponseobj->RType){
    //    case PONG_JOINGAMERESPONSE_TYPE_LOBBY:
    //        {
    //            // Show the lobby screen //
    //            instance->_OnNewConnectionStatusMessage("Server join completed, entering the lobby...");
    //            
    //            PongGame::Get()->VerifyCorrectState(tmpresponseobj->RType);

    //            return;
    //        }
    //    default:
    //        // It failed //
    //        instance->DisconnectFromServer("Received an invalid match status after a join game request");
    //    }

    //}, waitforthis, this), boost::bind<bool>([](shared_ptr<SentNetworkThing> packetobject) -> bool
    //{
    //    // Check is it sent //
    //    return packetobject->IsFinalized();

    //}, waitforthis))));
}
// ------------------------------------ //
void Pong::PongNetHandler::_OnNewConnectionStatusMessage(const string &message){
    Engine::Get()->GetEventHandler()->CallEvent(
        new Leviathan::GenericEvent("ConnectStatusMessage",
            Leviathan::NamedVars(shared_ptr<NamedVariableList>(
        new NamedVariableList("Message", new VariableBlock(message))))));
}
// ------------------------------------ //
void Pong::PongNetHandler::_OnDisconnectFromServer(const string &reasonstring, bool donebyus){

    // Ignore if the reason is us //
    if(donebyus){

        return;
    }
    
    Logger::Get()->Info("The server kicked us, showing the reason");

    // Disable the lobby screen //
    Engine::Get()->GetEventHandler()->CallEvent(new Leviathan::GenericEvent("LobbyScreenState",
            Leviathan::NamedVars(shared_ptr<NamedVariableList>(
        new NamedVariableList("State", new VariableBlock(string("Off")))))));


    Engine::Get()->GetEventHandler()->CallEvent(
        new Leviathan::GenericEvent("ConnectStatusMessage",
            Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                    new NamedVariableList("Message", new VariableBlock(
                            string("Server kicked us, reason: "+reasonstring)))))));

    // Enable the connection screen to display this message //
    DEBUG_BREAK;
    // Engine::Get()->GetWindowEntity()->GetGui()->SetCollectionState("ConnectionScreen", true);
}




