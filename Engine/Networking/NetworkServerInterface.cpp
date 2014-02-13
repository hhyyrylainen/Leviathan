#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKSERVERINTERFACE
#include "NetworkServerInterface.h"
#endif
#include "NetworkRequest.h"
#include "ConnectionInfo.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkServerInterface::NetworkServerInterface(int maxplayers, const wstring &servername, 
	NETWORKRESPONSE_SERVERJOINRESTRICT restricttype /*= NETWORKRESPONSE_SERVERJOINRESTRICT_NONE*/, int additionalflags /*= 0*/) : MaxPlayers(maxplayers),
		ServerName(servername), JoinRestrict(restricttype), ExtraServerFlags(additionalflags), AllowJoin(false), 
		ServerStatus(NETWORKRESPONSE_SERVERSTATUS_STARTING)
{

}

DLLEXPORT Leviathan::NetworkServerInterface::~NetworkServerInterface(){

}
// ------------------------------------ //
DLLEXPORT ConnectedPlayer* Leviathan::NetworkServerInterface::GetPlayerForConnection(ConnectionInfo* connection){
	DEBUG_BREAK;

	return NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::RespondToServerStatusRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connectiontouse){
	// Gather info for a response //
	shared_ptr<NetworkResponse> response(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 2000));

	// Generate a valid response //
	response->GenerateServerStatusResponse(new NetworkResponseDataForServerStatus(ServerName, AllowJoin, JoinRestrict, PlayerList.size(), MaxPlayers, 
		ActiveBots.size(), ServerStatus, ExtraServerFlags));

	// Log this //
	Logger::Get()->Info(L"NetworkServerInterface: Responding to server status request, to potential client on: "+
		connectiontouse->GenerateFormatedAddressString());

	// Send it //
	connectiontouse->SendPacketToConnection(response, 2);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::SetServerStatus(NETWORKRESPONSE_SERVERSTATUS newstatus){
	ServerStatus = newstatus;
}

DLLEXPORT void Leviathan::NetworkServerInterface::SetServerAllowPlayers(bool allowingplayers){
	AllowJoin = allowingplayers;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkServerInterface::_HandleServerRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connectiontosendresult){
	switch(request->GetType()){
	case NETWORKREQUESTTYPE_SERVERSTATUS:
		{
			RespondToServerStatusRequest(request, connectiontosendresult);
			// Handled //
			return true;
		}
	case NETWORKREQUESTTYPE_JOINSERVER:
		{
			// Call handling function //
			Logger::Get()->Info(L"NetworkServerInterface: player on "+connectiontosendresult->GenerateFormatedAddressString()+L"is trying to connect");
			_HandleServerJoinRequest(request, connectiontosendresult);
			return true;
		}
	}
	// We didn't know how to handle this packet //
	return false;
}

DLLEXPORT bool Leviathan::NetworkServerInterface::_HandleServerResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection, bool &dontmarkasreceived){
	// We don't know any handling for this //
	return false;
}

DLLEXPORT void Leviathan::NetworkServerInterface::_HandleServerJoinRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	ObjectLock guard(*this);

	if(!AllowJoin){

		shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 2000));

		// Set data //
		tmpresponse->GenerateServerDisallowResponse(new NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON_SERVERNOTACCEPTINGPLAYERS, 
			L"Server is not accepting any players at this time"));

		connection->SendPacketToConnection(tmpresponse, 1);
		return;
	}

	// Call this here, so this can potentially kick players for reserved slots //
	PlayerPreconnect(connection, request);

	// Check if we can fit a new player //
	if((int)(PlayerList.size()+1) > MaxPlayers){

		shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 2000));

		wstring plys = Convert::ToWstring(PlayerList.size());

		// Set data //
		tmpresponse->GenerateServerDisallowResponse(new NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON_SERVERFULL, L"Server"
			L" is at maximum capacity, "+plys+L"/"+plys));

		connection->SendPacketToConnection(tmpresponse, 1);
		return;
	}


	// Connection security check //
	

	// Do something with join restrict things //


	// Check if the program wants to veto this join //
	wstring disallowmessage;

	if(!AllowPlayerConnectVeto(request, connection, disallowmessage)){

		shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 2000));

		// Set data //
		tmpresponse->GenerateServerDisallowResponse(new NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON_SERVERCUSTOM, disallowmessage));

		connection->SendPacketToConnection(tmpresponse, 1);
		return;
	}

	// Player joined! //

	PlayerList.push_back(new ConnectedPlayer(connection, this));

	_OnPlayerConnected(PlayerList.back());
}
// ------------------ Default callbacks ------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::_OnPlayerConnected(ConnectedPlayer* newplayer){

}

DLLEXPORT void Leviathan::NetworkServerInterface::_OnPlayerDisconnect(ConnectedPlayer* newplayer){

}

DLLEXPORT bool Leviathan::NetworkServerInterface::PlayerPotentiallyKicked(ConnectedPlayer* player){
	return true;
}

DLLEXPORT bool Leviathan::NetworkServerInterface::AllowPlayerConnectVeto(shared_ptr<NetworkRequest> request, ConnectionInfo* connection, wstring &message){
	return true;
}

DLLEXPORT void Leviathan::NetworkServerInterface::PlayerPreconnect(ConnectionInfo* connection, shared_ptr<NetworkRequest> joinrequest){

}
// ------------------ ConnectedPlayer ------------------ //
Leviathan::ConnectedPlayer::ConnectedPlayer(ConnectionInfo* unsafeconnection, NetworkServerInterface* owninginstance) : 
	CorrenspondingConnection(unsafeconnection), Owner(owninginstance)
{
	// Register us //
	this->ConnectToNotifier(unsafeconnection);
}

void Leviathan::ConnectedPlayer::_OnNotifierDisconnected(BaseNotifierAll* parenttoremove){

	Logger::Get()->Info(L"ConnectedPlayer: player connection closed");
}
