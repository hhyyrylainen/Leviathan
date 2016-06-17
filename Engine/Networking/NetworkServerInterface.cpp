#include "Include.h"
// ------------------------------------ //
#include "NetworkServerInterface.h"

#include "NetworkRequest.h"
#include "ConnectionInfo.h"
#include "Gameplay/CommandHandler.h"
#include "SyncedVariables.h"
#include "NetworkedInputHandler.h"
#include "Networking/AINetworkCache.h"
#include "Entities/GameWorld.h"
#include "../TimeIncludes.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkServerInterface::NetworkServerInterface(
    int maxplayers, const std::string &servername, 
	NETWORKRESPONSE_SERVERJOINRESTRICT restricttype
    /*= NETWORKRESPONSE_SERVERJOINRESTRICT_NONE*/, int additionalflags /*= 0*/) :
    MaxPlayers(maxplayers), ServerName(servername), JoinRestrict(restricttype),
    ServerStatus(NETWORKRESPONSE_SERVERSTATUS_STARTING),
    ExtraServerFlags(additionalflags),
    _CommandHandler(new CommandHandler(this))
{

}

DLLEXPORT Leviathan::NetworkServerInterface::~NetworkServerInterface(){

    Lock lock(PlayerListLocked);
    
	// Release the memory //
	for(auto iter = ServerPlayers.begin(); iter != ServerPlayers.end(); ){

		delete (*iter);
		// Can't report errors at this point, but maybe we should //
        cout << "Warning! server quitting while playerlist has data in it" << endl;
        
		iter = ServerPlayers.erase(iter);
	}

    GUARD_LOCK();
    
	PotentialInputHandler.reset();
	SAFE_DELETE(_CommandHandler);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::CloseDownServer(){
	GUARD_LOCK();

	// Prevent new players //
	ServerStatus = NETWORKRESPONSE_SERVERSTATUS_SHUTDOWN;
	AllowJoin = false;

    Lock uniqueplylock(PlayerListLocked);
    
	for(auto iter = ServerPlayers.begin(); iter != ServerPlayers.end(); ){

		// Kick them //
		(*iter)->OnKicked("Server closing");

		delete (*iter);
		iter = ServerPlayers.erase(iter);
	}
}
// ------------------------------------ //
DLLEXPORT ConnectedPlayer* Leviathan::NetworkServerInterface::GetPlayerForConnection(ConnectionInfo* connection){
	GUARD_LOCK();
	// Search through the connections //
    Lock plylock(PlayerListLocked);
    
	for(size_t i = 0; i < ServerPlayers.size(); i++){
		// Check with the pointer //
		if(ServerPlayers[i]->IsConnectionYoursPtrCompare(connection))
			return ServerPlayers[i];
	}

	// No matching one found //
	return NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::RespondToServerStatusRequest(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connectiontouse)
{
	// Gather info for a response //
	shared_ptr<NetworkResponse> response(new NetworkResponse(request->GetExpectedResponseID(),
            PACKET_TIMEOUT_STYLE_TIMEDMS, 2000));

	// Generate a valid response //
    {
        Lock plylock(PlayerListLocked);
        
        response->GenerateServerStatusResponse(new NetworkResponseDataForServerStatus(ServerName,
                AllowJoin, JoinRestrict, ServerPlayers.size(), MaxPlayers, ActiveBots.size(),
                ServerStatus, ExtraServerFlags));

    }

	// Log this //
	Logger::Get()->Info("NetworkServerInterface: Responding to server status request, "
        "to potential client on: "+connectiontouse->GenerateFormatedAddressString());

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
DLLEXPORT bool Leviathan::NetworkServerInterface::_HandleServerRequest(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connectiontosendresult)
{
	// Try to handle input packet if we have the proper handler //
	if(PotentialInputHandler && PotentialInputHandler->HandleInputPacket(request, connectiontosendresult)){
		
		return true;
	}

	switch(request->GetType()){
	case NETWORKREQUESTTYPE_REQUESTEXECUTION:
		{
			// Get the matching player //
			auto ply = GetPlayerForConnection(connectiontosendresult);

			// Drop it if no matching players //
			if(!ply)
				return true;

			// Send a response to the sender //
			shared_ptr<NetworkResponse> fineresponse(new NetworkResponse(request->GetExpectedResponseID(), 
				PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 40));

			fineresponse->GenerateEmptyResponse();

			connectiontosendresult->SendPacketToConnection(fineresponse, 4);

			// Extract the command //
			auto data = request->GetCommandExecutionRequestData();

			if(!data)
				return true;

			{
				// We need to lock the player object for the adding process //
				GUARD_LOCK_OTHER_NAME(ply, lock2);

				// Execute it //
				_CommandHandler->QueueCommand(data->Command, ply, lock2);
			}

			return true;
		}
	case NETWORKREQUESTTYPE_SERVERSTATUS:
		{
			RespondToServerStatusRequest(request, connectiontosendresult);
			// Handled //
			return true;
		}
	case NETWORKREQUESTTYPE_JOINSERVER:
		{
			// Call handling function //
			Logger::Get()->Info("NetworkServerInterface: player on "+connectiontosendresult->
                GenerateFormatedAddressString()+"is trying to connect");
            
			_HandleServerJoinRequest(request, connectiontosendresult);
			return true;
		}
	default:
		// Let somebody else do this
		return false;
	}

	// We didn't know how to handle this packet //
	return false;
}

DLLEXPORT bool Leviathan::NetworkServerInterface::_HandleServerResponseOnly(shared_ptr<NetworkResponse> message,
    ConnectionInfo* connection, bool &dontmarkasreceived)
{
	// Try to handle input packet if we have the proper handler //
	if(PotentialInputHandler && PotentialInputHandler->HandleInputPacket(message, connection)){

		return true;
	}

	switch(message->GetType()){
        case NETWORKRESPONSETYPE_SERVERHEARTBEAT:
		{
			// Notify the matching player object about a heartbeat //
			ConnectedPlayer* ply = GetPlayerForConnection(connection);

			if(!ply){

				Logger::Get()->Warning("NetworkServerInterface: received a heartbeat packet from a "
                    "non-existing player");
				return true;
			}

			ply->HeartbeatReceived();
			
			// Avoid spamming packets back //
			//dontmarkasreceived = true;

			return true;
		}
        default:
            return false;
	}

	return false;
}

DLLEXPORT void Leviathan::NetworkServerInterface::_HandleServerJoinRequest(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connection)
{
	GUARD_LOCK();

	if(!AllowJoin){

		shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(),
                PACKET_TIMEOUT_STYLE_TIMEDMS, 2000));

		// Set data //
		tmpresponse->GenerateServerDisallowResponse(new NetworkResponseDataForServerDisallow(
                NETWORKRESPONSE_INVALIDREASON_SERVERNOTACCEPTINGPLAYERS, 
			"Server is not accepting any players at this time"));

		connection->SendPacketToConnection(tmpresponse, 1);
		return;
	}

	// Check is the player already connected //
    {
        Lock plylock(PlayerListLocked);
        
        for(auto iter = ServerPlayers.begin(); iter != ServerPlayers.end(); ++iter){
            // Check does it match //
            if((*iter)->IsConnectionYours(connection)){
                // Already connected //
                std::shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(),
                        PACKET_TIMEOUT_STYLE_TIMEDMS, 2000));

                // Set data //
                tmpresponse->GenerateServerDisallowResponse(new NetworkResponseDataForServerDisallow(
                        NETWORKRESPONSE_INVALIDREASON_SERVERALREADYCONNECTEDTOYOU, 
                        "You are already connected to this server, disconnect first"));

                connection->SendPacketToConnection(tmpresponse, 1);

                return;
            }
        }
    }

	// Call this here, so this can potentially kick players for reserved slots //
	PlayerPreconnect(connection, request);

	// Check if we can fit a new player //
    {

        Lock plylock(PlayerListLocked);
        
        if((int)(ServerPlayers.size()+1) > MaxPlayers){

            std::shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(),
                    PACKET_TIMEOUT_STYLE_TIMEDMS, 2000));

            std::string plys = Convert::ToString(ServerPlayers.size());

            // Set data //
            tmpresponse->GenerateServerDisallowResponse(new NetworkResponseDataForServerDisallow(
                    NETWORKRESPONSE_INVALIDREASON_SERVERFULL, "Server is at maximum capacity, "+
                    plys+"/"+plys));

            connection->SendPacketToConnection(tmpresponse, 1);
            return;
        }

    }
	// Connection security check //
	

	// Do something with join restrict things //


	// Check if the program wants to veto this join //
	std::string disallowmessage;

	if(!AllowPlayerConnectVeto(request, connection, disallowmessage)){

		shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(),
                PACKET_TIMEOUT_STYLE_TIMEDMS, 2000));

		// Set data //
		tmpresponse->GenerateServerDisallowResponse(new NetworkResponseDataForServerDisallow(
                NETWORKRESPONSE_INVALIDREASON_SERVERCUSTOM, disallowmessage));

		connection->SendPacketToConnection(tmpresponse, 1);
		return;
	}

	// Player joined! //
	int newid = ++CurrentPlayerID;
    {
        // The player list needs to be locked while we add stuff and read //
        Lock plylock(PlayerListLocked);
    
        ServerPlayers.push_back(new ConnectedPlayer(connection, this, newid));
        
        _OnReportPlayerConnected(ServerPlayers.back(), connection, guard);
    }
    
	Logger::Get()->Info("NetworkServerInterface: accepted a new player, ID: "+
        Convert::ToString(newid));

	// Send connection notification back to the client //
	shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(),
            PACKET_TIMEOUT_STYLE_TIMEDMS, 2000));

	// Set data //
	tmpresponse->GenerateServerAllowResponse(new NetworkResponseDataForServerAllow(
            NETWORKRESPONSE_SERVERACCEPTED_TYPE_CONNECT_ACCEPTED, "Allowed, ID: "+
            Convert::ToString(newid)));

	connection->SendPacketToConnection(tmpresponse, 3);
}
// ------------------ Default callbacks ------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::_OnPlayerConnected(Lock &guard,
    ConnectedPlayer* newplayer)
{

}

DLLEXPORT void Leviathan::NetworkServerInterface::_OnPlayerDisconnect(Lock &guard,
    ConnectedPlayer* newplayer)
{

}

DLLEXPORT bool Leviathan::NetworkServerInterface::PlayerPotentiallyKicked(ConnectedPlayer* player){
	return true;
}

DLLEXPORT bool Leviathan::NetworkServerInterface::AllowPlayerConnectVeto(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connection, std::string &message)
{
	return true;
}

DLLEXPORT void Leviathan::NetworkServerInterface::PlayerPreconnect(ConnectionInfo* connection,
    std::shared_ptr<NetworkRequest> joinrequest)
{

}

DLLEXPORT void Leviathan::NetworkServerInterface::RegisterCustomCommandHandlers(CommandHandler* addhere){

}
// ------------------------------------ //
void Leviathan::NetworkServerInterface::_OnReportCloseConnection(ConnectedPlayer* plyptr, Lock &guard)
{
	VerifyLock(guard);

	Logger::Get()->Info("NetworkServerInterface: player (TODO: get name) has closed their "
        "connection");

    _OnPlayerDisconnect(guard, plyptr);
}

void Leviathan::NetworkServerInterface::_OnReportPlayerConnected(ConnectedPlayer* plyptr, ConnectionInfo* connection,
    Lock &guard)
{
	VerifyLock(guard);

	Logger::Get()->Info("NetworkServerInterface: player (TODO: get name) has connected");

    if(SyncedVariables::Get())
        SyncedVariables::Get()->AddAnotherToSyncWith(connection);

    if(AINetworkCache::Get())
        AINetworkCache::Get()->RegisterNewConnection(connection);

    _OnPlayerConnected(guard, plyptr);
}

void Leviathan::NetworkServerInterface::_OnPlayerConnectionCloseResources(Lock &guard,
    ConnectedPlayer* ply)
{

    // Close common interfaces that might be using this player //
    
    // Stop syncing values with this client //
    if(SyncedVariables::Get()){

        // The connection should already be going to be closed soon... //
        SyncedVariables::Get()->RemoveConnectionWithAnother(ply->GetConnection(), true);
    }

    if(AINetworkCache::Get())
        AINetworkCache::Get()->RemoveConnection(ply->GetConnection());

    Logger::Get()->Info("NetworkServerInterface: player \""+ply->GetNickname()+
        "\" unconnected from common resources");
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::UpdateServerStatus(){
	{
        // Lock the list while we are doing stuff //
        Lock plylock(PlayerListLocked);
        
        // Check for closed connections //
        auto end = ServerPlayers.end();
        for(auto iter = ServerPlayers.begin(); iter != end; ){
            
            if((*iter)->IsConnectionClosed()){

                // The player has disconnected //
                GUARD_LOCK();
                _OnReportCloseConnection(*iter, guard);

                iter = ServerPlayers.erase(iter);
                // The end iterator is now also invalid //
                end = ServerPlayers.end();
                continue;
            }

            (*iter)->UpdateHeartbeats();
            ++iter;
        }
	}

	// Update the command handling //
	_CommandHandler->UpdateStatus();

	// Update networked input handling //
	if(PotentialInputHandler)
		PotentialInputHandler->UpdateInputStatus();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkServerInterface::RegisterNetworkedInput(shared_ptr<NetworkedInputHandler> handler){

	PotentialInputHandler = handler;
	return true;
}

DLLEXPORT NetworkedInputHandler* Leviathan::NetworkServerInterface::GetNetworkedInput(){
	return PotentialInputHandler.get();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::SendToAllButOnePlayer(shared_ptr<NetworkResponse> response,
    ConnectionInfo* skipme, int resendcount /*= 4*/)
{
	GUARD_LOCK();

    Lock plylock(PlayerListLocked);
    
	// Loop the players and send to their connections //
	auto end = ServerPlayers.end();
	for(auto iter = ServerPlayers.begin(); iter != end; ++iter){

		ConnectionInfo* curconnection = (*iter)->GetConnection();

		if(curconnection != skipme){

			curconnection->SendPacketToConnection(response, resendcount);
		}
	}
}

DLLEXPORT void Leviathan::NetworkServerInterface::SendToAllPlayers(
    shared_ptr<NetworkResponse> response, int resendcount /*= 4*/)
{
    Lock plylock(PlayerListLocked);

	// Loop the players and send to their connections //
	auto end = ServerPlayers.end();
	for(auto iter = ServerPlayers.begin(); iter != end; ++iter){

		(*iter)->GetConnection()->SendPacketToConnection(response, resendcount);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkServerInterface::VerifyWorldIsSyncedWithPlayers(shared_ptr<GameWorld> world){
	GUARD_LOCK();

    Lock plylock(PlayerListLocked);
    
	// We can safely add all players as they will only be added if they aren't there already //
	auto end = ServerPlayers.end();
	for(auto iter = ServerPlayers.begin(); iter != end; ++iter){

        GUARD_LOCK_OTHER_NAME((*iter), guard2);
		if(world->ConnectToNotifiable(guard, *iter, guard2)){

			// Added a new one //
			Logger::Get()->Info("NetworkServerInterface: a new player is now synced with a world");
		}
	}
}
// ------------------------------------ //

int Leviathan::NetworkServerInterface::CurrentPlayerID = 1000;
// ------------------ ConnectedPlayer ------------------ //
Leviathan::ConnectedPlayer::ConnectedPlayer(ConnectionInfo* unsafeconnection, NetworkServerInterface* owninginstance,
    int plyid) : 
	CorrespondingConnection(unsafeconnection), Owner(owninginstance), ConnectionStatus(true), UsingHeartbeats(false),
    IsControlLost(false), SecondsWithoutConnection(0.f), ID(plyid)
{
	// Register us //
	this->ConnectToNotifier(unsafeconnection);
}

DLLEXPORT Leviathan::ConnectedPlayer::~ConnectedPlayer(){
	GUARD_LOCK();
	_OnReleaseParentCommanders(guard);
}
// ------------------------------------ //
void Leviathan::ConnectedPlayer::_OnNotifierDisconnected(Lock &guard,
    BaseNotifierAll* parenttoremove, Lock &parentlock)
{

    GUARD_LOCK_OTHER_NAME(Owner, guard2);
    Owner->_OnPlayerConnectionCloseResources(guard2, this);

    // Set as closing //
    ConnectionStatus = false;

    CorrespondingConnection = NULL;

	Logger::Get()->Info("ConnectedPlayer: connection marked as closed");
}

DLLEXPORT bool Leviathan::ConnectedPlayer::IsConnectionYours(ConnectionInfo* checkconnection){
	GUARD_LOCK();

	return CorrespondingConnection->GenerateFormatedAddressString() ==
        checkconnection->GenerateFormatedAddressString();
}

DLLEXPORT bool Leviathan::ConnectedPlayer::IsConnectionYoursPtrCompare(ConnectionInfo* checkconnection){
	return CorrespondingConnection == checkconnection;
}

DLLEXPORT bool Leviathan::ConnectedPlayer::IsConnectionClosed() const{
	return !ConnectionStatus;
}

DLLEXPORT void Leviathan::ConnectedPlayer::OnKicked(const std::string &reason){
	{
		// Send a close connection packet //
		GUARD_LOCK();

		auto connection = NetworkHandler::Get()->GetSafePointerToConnection(CorrespondingConnection);

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
	GUARD_LOCK();

	// Send a start packet //
	auto connection = NetworkHandler::Get()->GetSafePointerToConnection(CorrespondingConnection);

	if(!connection){

		ConnectionStatus = false;
		return;
	}

	// Create the packet and THEN send it //
	shared_ptr<NetworkResponse> response(new NetworkResponse(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000));
	response->GenerateStartHeartbeatsResponse();

	connection->SendPacketToConnection(response, 7);

	// Reset our variables //
	UsingHeartbeats = true;

	LastReceivedHeartbeat = Time::GetThreadSafeSteadyTimePoint();
	LastSentHeartbeat = LastReceivedHeartbeat;
	SecondsWithoutConnection = 0.f;
}

DLLEXPORT void Leviathan::ConnectedPlayer::HeartbeatReceived(){
	// Reset all timers //
	GUARD_LOCK();

	LastReceivedHeartbeat = Time::GetThreadSafeSteadyTimePoint();

	// Re-acquire controls, if lost in the first place //
	if(IsControlLost){

		
		IsControlLost = false;
	}

}

DLLEXPORT void Leviathan::ConnectedPlayer::UpdateHeartbeats(){
	// Skip if not used //
	if(!UsingHeartbeats)
		return;

	GUARD_LOCK();

	// Check do we need to send one //
	auto timenow = Time::GetThreadSafeSteadyTimePoint();

	if(timenow >= LastSentHeartbeat+MillisecondDuration(SERVER_HEARTBEATS_MILLISECOND)){

		auto connection = NetworkHandler::Get()->GetSafePointerToConnection(CorrespondingConnection);

		if(!connection){

			ConnectionStatus = false;
			return;
		}

		// Send one //
		shared_ptr<NetworkResponse> response(new NetworkResponse(-1, PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 30));
		response->GenerateHeartbeatResponse();

		connection->SendPacketToConnection(response, 1);

		LastSentHeartbeat = timenow;
	}

	// Update the time without a response //
	SecondsWithoutConnection = SecondDuration(timenow-LastReceivedHeartbeat).count();

	// Do something if the time is too high //
	if(SecondsWithoutConnection >= 2.f){


		IsControlLost = true;
	}
}
// ------------------------------------ //
DLLEXPORT const string& Leviathan::ConnectedPlayer::GetUniqueName(){
	return UniqueName;
}

DLLEXPORT const string& Leviathan::ConnectedPlayer::GetNickname(){
	return DisplayName;
}

DLLEXPORT COMMANDSENDER_PERMISSIONMODE Leviathan::ConnectedPlayer::GetPermissionMode(){
	return COMMANDSENDER_PERMISSIONMODE_NORMAL;
}

DLLEXPORT bool Leviathan::ConnectedPlayer::_OnSendPrivateMessage(const string &message){
	
	Logger::Get()->Write("Probably should implement a ChatManager");
	return false;
}

DLLEXPORT ConnectionInfo* Leviathan::ConnectedPlayer::GetConnection(){
	return CorrespondingConnection;
}

DLLEXPORT int Leviathan::ConnectedPlayer::GetID() const{
	return ID;
}
// ------------------------------------ //
DLLEXPORT ObjectID Leviathan::ConnectedPlayer::GetPositionInWorld(GameWorld* world, Lock &guard)
    const
{
	// Not found for that world //
	return 0;
}
