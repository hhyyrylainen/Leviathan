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
	// Release the memory //
	for(auto iter = PlayerList.begin(); iter != PlayerList.end(); ){

		delete (*iter);
		// Can't report errors at this point //
		//Logger::Get()->Warning(L"NetworkServerInterface: destructor has still active players, kicking must have failed");
		iter = PlayerList.erase(iter);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::CloseDownServer(){
	ObjectLock guard(*this);

	// Prevent new players //
	ServerStatus = NETWORKRESPONSE_SERVERSTATUS_SHUTDOWN;
	AllowJoin = false;

	for(auto iter = PlayerList.begin(); iter != PlayerList.end(); ){

		// Kick them //


		delete (*iter);
		iter = PlayerList.erase(iter);
	}
}
// ------------------------------------ //
DLLEXPORT ConnectedPlayer* Leviathan::NetworkServerInterface::GetPlayerForConnection(ConnectionInfo* connection){
	ObjectLock guard(*this);
	// Search through the connections //
	for(size_t i = 0; i < PlayerList.size(); i++){
		// Check with the pointer //
		if(PlayerList[i]->IsConnectionYoursPtrCompare(connection))
			return PlayerList[i];
	}

	// No matching one found //
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

	// Check is the player already connected //
	for(auto iter = PlayerList.begin(); iter != PlayerList.end(); ++iter){
		// Check does it match //
		if((*iter)->IsConnectionYours(connection)){
			// Already connected //
			shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 2000));

			// Set data //
			tmpresponse->GenerateServerDisallowResponse(new NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON_SERVERALREADYCONNECTEDTOYOU, 
				L"You are already connected to this server, disconnect first"));

			connection->SendPacketToConnection(tmpresponse, 1);

			return;
		}
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

	Logger::Get()->Info(L"NetworkServerInterface: accepted a new player");

	// Send connection notification back to the client //
	shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 2000));

	// Set data //
	tmpresponse->GenerateServerAllowResponse(new NetworkResponseDataForServerAllow(NETWORKRESPONSE_SERVERACCEPTED_TYPE_CONNECT_ACCEPTED));

	connection->SendPacketToConnection(tmpresponse, 3);
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

std::vector<ConnectedPlayer*>::iterator Leviathan::NetworkServerInterface::_OnReportCloseConnection(const std::vector<ConnectedPlayer*>::iterator 
	&iter, ObjectLock &guard)
{
	VerifyLock(guard);

	if(iter == PlayerList.end()){

		Logger::Get()->Error(L"NetworkServerInterface: _OnReportCloseConnection: trying to use an invalid iterator");
		return PlayerList.end();
	}

	// The player has disconnected //
	Logger::Get()->Info(L"NetworkServerInterface: player (TODO: get name) has closed their connection");

	_OnPlayerDisconnect(*iter);

	delete (*iter);
	return PlayerList.erase(iter);
}

DLLEXPORT void Leviathan::NetworkServerInterface::UpdateServerStatus(){
	ObjectLock guard(*this);
	// Check for closed connections //

	for(auto iter = PlayerList.begin(); iter != PlayerList.end(); ){
		// Check is it the player //
		if((*iter)->IsConnectionClosed()){
			// The player has disconnected //
			iter = _OnReportCloseConnection(iter, guard);

		} else {
			++iter;
		}
	}
}
// ------------------ ConnectedPlayer ------------------ //
Leviathan::ConnectedPlayer::ConnectedPlayer(ConnectionInfo* unsafeconnection, NetworkServerInterface* owninginstance) : 
	CorrenspondingConnection(unsafeconnection), Owner(owninginstance), ConnectionStatus(true)
{
	// Register us //
	this->ConnectToNotifier(unsafeconnection);
}

void Leviathan::ConnectedPlayer::_OnNotifierDisconnected(BaseNotifierAll* parenttoremove){
	{
		ObjectLock guard(*this);

		// Set as closing //
		ConnectionStatus = false;
	}

	Logger::Get()->Info(L"ConnectedPlayer: connection marked as closed");
}

DLLEXPORT bool Leviathan::ConnectedPlayer::IsConnectionYours(ConnectionInfo* checkconnection){
	ObjectLock guard(*this);

	return CorrenspondingConnection->GenerateFormatedAddressString() == checkconnection->GenerateFormatedAddressString();
}

DLLEXPORT bool Leviathan::ConnectedPlayer::IsConnectionYoursPtrCompare(ConnectionInfo* checkconnection){
	return CorrenspondingConnection == checkconnection;
}

DLLEXPORT Leviathan::ConnectedPlayer::~ConnectedPlayer(){

}

DLLEXPORT bool Leviathan::ConnectedPlayer::IsConnectionClosed() const{
	return !ConnectionStatus;
}
