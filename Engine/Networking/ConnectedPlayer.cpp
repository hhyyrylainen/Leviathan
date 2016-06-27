// ------------------------------------ //
#include "ConnectedPlayer.h"

using namespace Leviathan;
// ------------------------------------ //
ConnectedPlayer::ConnectedPlayer(Connection* unsafeconnection, NetworkServerInterface* owninginstance,
    int plyid) : 
	CorrespondingConnection(unsafeconnection), Owner(owninginstance), ConnectionStatus(true), UsingHeartbeats(false),
    IsControlLost(false), SecondsWithoutConnection(0.f), ID(plyid)
{
	// Register us //
	this->ConnectToNotifier(unsafeconnection);
}

DLLEXPORT ConnectedPlayer::~ConnectedPlayer(){
	GUARD_LOCK();
	_OnReleaseParentCommanders(guard);
}
// ------------------------------------ //
DLLEXPORT bool ConnectedPlayer::IsConnectionYours(Connection* checkconnection){
	GUARD_LOCK();

	return CorrespondingConnection->GenerateFormatedAddressString() ==
        checkconnection->GenerateFormatedAddressString();
}

DLLEXPORT bool ConnectedPlayer::IsConnectionYoursPtrCompare(Connection* checkconnection){
	return CorrespondingConnection == checkconnection;
}

DLLEXPORT bool ConnectedPlayer::IsConnectionClosed() const{
	return !ConnectionStatus;
}

DLLEXPORT void ConnectedPlayer::OnKicked(const std::string &reason){
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
DLLEXPORT void ConnectedPlayer::StartHeartbeats(){
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

DLLEXPORT void ConnectedPlayer::HeartbeatReceived(){
	// Reset all timers //
	GUARD_LOCK();

	LastReceivedHeartbeat = Time::GetThreadSafeSteadyTimePoint();

	// Re-acquire controls, if lost in the first place //
	if(IsControlLost){

		
		IsControlLost = false;
	}

}

DLLEXPORT void ConnectedPlayer::UpdateHeartbeats(){
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
DLLEXPORT const string& ConnectedPlayer::GetUniqueName(){
	return UniqueName;
}

DLLEXPORT const string& ConnectedPlayer::GetNickname(){
	return DisplayName;
}

DLLEXPORT COMMANDSENDER_PERMISSIONMODE ConnectedPlayer::GetPermissionMode(){
	return COMMANDSENDER_PERMISSIONMODE_NORMAL;
}

DLLEXPORT bool ConnectedPlayer::_OnSendPrivateMessage(const string &message){
	
	Logger::Get()->Write("Probably should implement a ChatManager");
	return false;
}

DLLEXPORT Connection* ConnectedPlayer::GetConnection(){
	return CorrespondingConnection;
}

DLLEXPORT int ConnectedPlayer::GetID() const{
	return ID;
}
// ------------------------------------ //
DLLEXPORT ObjectID ConnectedPlayer::GetPositionInWorld(GameWorld* world, Lock &guard)
    const
{
	// Not found for that world //
	return 0;
}

