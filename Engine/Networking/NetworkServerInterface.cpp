// ------------------------------------ //
#include "NetworkServerInterface.h"

#include "NetworkRequest.h"
#include "Connection.h"
#include "Gameplay/CommandHandler.h"
#include "SyncedVariables.h"
#include "NetworkedInputHandler.h"
#include "Networking/NetworkCache.h"
#include "Entities/GameWorld.h"
#include "../TimeIncludes.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkServerInterface::NetworkServerInterface(
    int maxplayers, const std::string &servername, 
	SERVER_JOIN_RESTRICT restricttype, int additionalflags /*= 0*/) :
    NetworkInterface(NETWORKED_TYPE::Server),
    MaxPlayers(maxplayers), ServerName(servername), JoinRestrict(restricttype),
    ExtraServerFlags(additionalflags),
    _CommandHandler(new CommandHandler(this))
{

}

DLLEXPORT Leviathan::NetworkServerInterface::~NetworkServerInterface(){

    Lock lock(PlayerListLocked);
    
	// Release the memory //
	for(auto iter = ServerPlayers.begin(); iter != ServerPlayers.end(); ){

		// Can't report errors at this point, but maybe we should //
        cout << "Warning! server quitting while player list has data in it" << endl;
        
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
	ServerStatus = SERVER_STATUS::Shutdown;
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
DLLEXPORT std::shared_ptr<ConnectedPlayer> 
Leviathan::NetworkServerInterface::GetPlayerForConnection(
    const std::shared_ptr<Connection> &connection)
{
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
    Connection* connectiontouse)
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
    Connection* connectiontosendresult)
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
    Connection* connection, bool &dontmarkasreceived)
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
    Connection* connection)
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
    Connection* connection, std::string &message)
{
	return true;
}

DLLEXPORT void Leviathan::NetworkServerInterface::PlayerPreconnect(Connection* connection,
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

void Leviathan::NetworkServerInterface::_OnReportPlayerConnected(ConnectedPlayer* plyptr, Connection* connection,
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
    Connection* skipme, int resendcount /*= 4*/)
{
	GUARD_LOCK();

    Lock plylock(PlayerListLocked);
    
	// Loop the players and send to their connections //
	auto end = ServerPlayers.end();
	for(auto iter = ServerPlayers.begin(); iter != end; ++iter){

		Connection* curconnection = (*iter)->GetConnection();

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

