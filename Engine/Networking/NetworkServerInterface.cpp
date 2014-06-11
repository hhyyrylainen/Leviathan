#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKSERVERINTERFACE
#include "NetworkServerInterface.h"
#endif
#include "NetworkRequest.h"
#include "ConnectionInfo.h"
#include "Common\Misc.h"
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
	GUARD_LOCK_THIS_OBJECT();

	// Prevent new players //
	ServerStatus = NETWORKRESPONSE_SERVERSTATUS_SHUTDOWN;
	AllowJoin = false;

	for(auto iter = PlayerList.begin(); iter != PlayerList.end(); ){

		// Kick them //
		(*iter)->OnKicked(L"Server closing");

		delete (*iter);
		iter = PlayerList.erase(iter);
	}
}
// ------------------------------------ //
DLLEXPORT ConnectedPlayer* Leviathan::NetworkServerInterface::GetPlayerForConnection(ConnectionInfo* connection){
	GUARD_LOCK_THIS_OBJECT();
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

DLLEXPORT bool Leviathan::NetworkServerInterface::_HandleServerResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection, 
	bool &dontmarkasreceived)
{
	switch(message->GetType()){
	case NETWORKRESPONSETYPE_SERVERHEARTBEAT:
		{
			// Notify the matching player object about a heartbeat //
			ConnectedPlayer* ply = GetPlayerForConnection(connection);

			if(!ply){

				Logger::Get()->Warning(L"NetworkServerInterface: received a heartbeat packet from a nonexisting player");
				return true;
			}

			ply->HeartbeatReceived();

			return true;
		}
	}

	return false;
}

DLLEXPORT void Leviathan::NetworkServerInterface::_HandleServerJoinRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	GUARD_LOCK_THIS_OBJECT();

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
	GUARD_LOCK_THIS_OBJECT();
	// Check for closed connections //

	for(auto iter = PlayerList.begin(); iter != PlayerList.end(); ){
		// Check is it the player //
		if((*iter)->IsConnectionClosed()){
			// The player has disconnected //
			iter = _OnReportCloseConnection(iter, guard);

		} else {

			(*iter)->UpdateHeartbeats();

			++iter;
		}
	}
}
// ------------------ ConnectedPlayer ------------------ //
Leviathan::ConnectedPlayer::ConnectedPlayer(ConnectionInfo* unsafeconnection, NetworkServerInterface* owninginstance) : 
	CorrenspondingConnection(unsafeconnection), Owner(owninginstance), ConnectionStatus(true), UsingHeartbeats(false), IsControlLost(false),
	SecondsWithoutConnection(0.f)
{
	// Register us //
	this->ConnectToNotifier(unsafeconnection);
}

void Leviathan::ConnectedPlayer::_OnNotifierDisconnected(BaseNotifierAll* parenttoremove){
	{
		GUARD_LOCK_THIS_OBJECT();

		// Set as closing //
		ConnectionStatus = false;

		CorrenspondingConnection = NULL;
	}

	Logger::Get()->Info(L"ConnectedPlayer: connection marked as closed");
}

DLLEXPORT bool Leviathan::ConnectedPlayer::IsConnectionYours(ConnectionInfo* checkconnection){
	GUARD_LOCK_THIS_OBJECT();

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

DLLEXPORT void Leviathan::ConnectedPlayer::OnKicked(const wstring &reason){
	{
		// Send a close connection packet //
		GUARD_LOCK_THIS_OBJECT();

		auto connection = NetworkHandler::Get()->GetSafePointerToConnection(CorrenspondingConnection);

		if(connection){

			// \todo Add the reason here
			connection->SendCloseConnectionPacket();
		}


		// No longer connected //
		ConnectionStatus = false;
	}

	// Broadcast a kick message on the server here //

}
// ------------------------------------ //
DLLEXPORT void Leviathan::ConnectedPlayer::StartHeartbeats(){
	GUARD_LOCK_THIS_OBJECT();

	// Send a start packet //
	auto connection = NetworkHandler::Get()->GetSafePointerToConnection(CorrenspondingConnection);

	if(!connection){

		ConnectionStatus = false;
		return;
	}

	// Create the packet and THEN send it //
	shared_ptr<NetworkResponse> response(new NetworkResponse(-1, PACKAGE_TIMEOUT_STYLE_TIMEDMS, 1000));
	response->GenerateStartHeartbeatsResponse();

	connection->SendPacketToConnection(response, 7);

	// Reset our variables //
	UsingHeartbeats = true;

	LastReceivedHeartbeat = Misc::GetThreadSafeSteadyTimePoint();
	LastSentHeartbeat = LastReceivedHeartbeat;
	SecondsWithoutConnection = 0.f;
}

DLLEXPORT void Leviathan::ConnectedPlayer::HeartbeatReceived(){
	// Reset all timers //
	GUARD_LOCK_THIS_OBJECT();

	LastReceivedHeartbeat = Misc::GetThreadSafeSteadyTimePoint();

	// Re-acquire controls, if lost in the first place //
	if(IsControlLost){

		DEBUG_BREAK;
		IsControlLost = false;
	}

}

DLLEXPORT void Leviathan::ConnectedPlayer::UpdateHeartbeats(){
	// Skip if not used //
	if(!UsingHeartbeats)
		return;

	GUARD_LOCK_THIS_OBJECT();

	// Check do we need to send one //
	auto timenow = Misc::GetThreadSafeSteadyTimePoint();

	if(timenow >= LastSentHeartbeat+MillisecondDuration(SERVER_HEARTBEATS_MILLISECOND)){

		auto connection = NetworkHandler::Get()->GetSafePointerToConnection(CorrenspondingConnection);

		if(!connection){

			ConnectionStatus = false;
			return;
		}

		// Send one //
		shared_ptr<NetworkResponse> response(new NetworkResponse(-1, PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 5));
		response->GenerateHeartbeatResponse();

		connection->SendPacketToConnection(response, 1);

		LastSentHeartbeat = timenow;
	}

	// Update the time without a response //
	SecondsWithoutConnection = SecondDuration(timenow-LastSentHeartbeat).count();

	// Do something if the time is too high //
	if(SecondsWithoutConnection >= 2.f){


		IsControlLost = true;
		DEBUG_BREAK;
	}
}
