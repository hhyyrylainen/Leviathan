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

	}
	// We didn't know how to handle this packet //
	return false;
}

DLLEXPORT bool Leviathan::NetworkServerInterface::_HandleServerResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection, bool &dontmarkasreceived){
	// We don't know any handling for this //
	return false;
}
// ------------------ Default callbacks ------------------ //
DLLEXPORT bool Leviathan::NetworkServerInterface::PlayerAllowedToConnect(ConnectionInfo* connection, shared_ptr<NetworkRequest> joinrequest){

	return true;
}

DLLEXPORT void Leviathan::NetworkServerInterface::_OnPlayerConnected(ConnectedPlayer* newplayer){

}

DLLEXPORT void Leviathan::NetworkServerInterface::_OnPlayerDisconnect(ConnectedPlayer* newplayer){

}

DLLEXPORT bool Leviathan::NetworkServerInterface::PlayerPotentiallyKicked(ConnectedPlayer* player){
	return true;
}
