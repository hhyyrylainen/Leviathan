#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONGNETHANDLER
#include "PongNetHandler.h"
#endif
#include "Networking\NetworkResponse.h"
#include "PongPackets.h"
#include "Networking\NetworkRequest.h"
#include "Networking\ConnectionInfo.h"
using namespace Pong;
// ------------------------------------ //
Pong::PongNetHandler::PongNetHandler() : OnAServer(false), ServerStatusIs(PONG_JOINGAMERESPONSE_TYPE_LOBBY){

}

Pong::PongNetHandler::~PongNetHandler(){

}
// ------------------------------------ //
void Pong::PongNetHandler::HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived){
	// Try default handling //
	if(_HandleDefaultResponseOnly(message, connection, dontmarkasreceived))
		return;
	// Try client handling //
	if(_HandleClientResponseOnly(message, connection, dontmarkasreceived))
		return;

	// Check for custom packets //
	if(message->GetType() == Leviathan::NETWORKRESPONSETYPE_CUSTOM){
		// Handle the game specific //
		GameSpecificPacketData* data = message->GetResponseDataForGameSpecific()->ActualPacketData.get();

		// Check is it invalid type //
		if(data->IsRequest)
			return;

		switch(data->TypeIDNumber){
		case PONG_PACKET_JOINGAME_RESPONSE:
			{
				assert(0 && "shouldn't get this packet");
			}
			break;
		}

		return;
	}

	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}

void Pong::PongNetHandler::HandleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	// Try default //
	if(_HandleDefaultRequest(request, connection))
		return;
	// Try client handling //
	if(_HandleClientRequest(request, connection))
		return;


	// Check for custom packets //
	if(request->GetType() == Leviathan::NETWORKREQUESTTYPE_CUSTOM){
		// Handle the game specific //
		GameSpecificPacketData* data = request->GetCustomRequestData()->ActualPacketData.get();

		// Check is it invalid type //
		if(!data->IsRequest)
			return;

		switch(data->TypeIDNumber){
		case PONG_PACKET_JOINGAME_REQUEST:
			{
				assert(0 && "shouldn't get this packet");
			}
			break;
		}
		return;
	}

	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}

DLLEXPORT void Pong::PongNetHandler::TickIt(){
	UpdateClientStatus();
}
// ------------------------------------ //
void Pong::PongNetHandler::_OnStartApplicationConnect(){
	Logger::Get()->Info(L"Pong ready to join the lobby or the game");


	// Send our custom join request packet //
	shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(new Leviathan::CustomRequestData(new PongJoinGameRequest())));

	shared_ptr<SentNetworkThing> waitforthis = ServerConnection->SendPacketToConnection(tmprequest, 5);


	// Start waiting for a response //
	Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new Leviathan::ConditionalTask(
		boost::bind<void>([](shared_ptr<SentNetworkThing> packetobject, PongNetHandler* instance) -> void
	{
		// Check did it succeed //
		if(!packetobject->GetFutureForThis().get() || !packetobject->GotResponse){

			// It failed //
			instance->DisconnectFromServer(L"Failed to receive response to join lobby/match request");
			return;
		}

		// Check where we got (lobby, match, end screen) //
		Leviathan::NetworkResponseDataForCustom* directdata = packetobject->GotResponse->GetResponseDataForGameSpecific();

		// Check is it invalid for this request //
		if(!directdata || !directdata->ActualPacketData || !directdata->ActualPacketData->ResponseBaseData || 
			directdata->ActualPacketData->TypeIDNumber != PONG_PACKET_JOINGAME_RESPONSE)
		{
			// It failed //
			instance->DisconnectFromServer(L"Received an invalid response to join game request");
			return;
		}


		// Now connected //
		PongJoinGameResponse* tmpresponseobj = dynamic_cast<PongJoinGameResponse*>(directdata->ActualPacketData->ResponseBaseData);
		assert(tmpresponseobj && "the packet factory is being wonky");

		switch(tmpresponseobj->RType){
		case PONG_JOINGAMERESPONSE_TYPE_LOBBY:
			{
				// Show the lobby screen //
				instance->_OnNewConnectionStatusMessage(L"Server join completed, entering lobby...");
				
				// Send event to enable the lobby screen //
				EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"LobbyScreenState", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
					new NamedVariableList(L"State", new VariableBlock(string("On")))))));

				return;
			}
		case PONG_JOINGAMERESPONSE_TYPE_MATCH:
			{
				// Show the during game GUI //
				DEBUG_BREAK;
				return;
			}
		case PONG_JOINGAMERESPONSE_TYPE_GAMEEND:
			{
				// Show the match scores screen //
				DEBUG_BREAK;
				return;
			}
		default:
			// It failed //
			instance->DisconnectFromServer(L"Received an invalid match status after a join game request");
		}

	}, waitforthis, this), boost::bind<bool>([](shared_ptr<SentNetworkThing> packetobject) -> bool
	{
		// Check is it sent //
		return packetobject->GetFutureForThis().has_value();

	}, waitforthis))));
}
// ------------------------------------ //
void Pong::PongNetHandler::_OnNewConnectionStatusMessage(const wstring &message){
	EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
		new NamedVariableList(L"Message", new VariableBlock(Convert::WstringToString(message)))))));
}




