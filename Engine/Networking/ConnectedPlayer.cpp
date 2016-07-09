// ------------------------------------ //
#include "ConnectedPlayer.h"

#include "Connection.h"

using namespace Leviathan;
// ------------------------------------ //
ConnectedPlayer::ConnectedPlayer(Connection* unsafeconnection, NetworkServerInterface* owninginstance,
    int plyid) : 
	CorrespondingConnection(unsafeconnection), Owner(owninginstance), UsingHeartbeats(false),
    IsControlLost(false), SecondsWithoutConnection(0.f), ID(plyid)
{
}

DLLEXPORT ConnectedPlayer::~ConnectedPlayer(){
	GUARD_LOCK();
	_OnReleaseParentCommanders(guard);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConnectedPlayer::IsConnectionYours(Connection* checkconnection) {

    return CorrespondingConnection.get() == checkconnection;
}
// ------------------------------------ //
DLLEXPORT bool ConnectedPlayer::IsConnectionClosed() const{
	return CorrespondingConnection ? !CorrespondingConnection->IsOpen() : true;
}
// ------------------------------------ //
DLLEXPORT void ConnectedPlayer::OnKicked(const std::string &reason){
	{
		// Send a close connection packet //
		GUARD_LOCK();

		if(CorrespondingConnection){

			// \todo Add the reason here
            CorrespondingConnection->SendCloseConnectionPacket();
		}
	}

	// Broadcast a kick message on the server here //

}
// ------------------------------------ //
DLLEXPORT void ConnectedPlayer::StartHeartbeats(){
	GUARD_LOCK();

	// Send a start packet //

    DEBUG_BREAK;
    //ResponseStartHeartbeats;

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

	if(timenow >= LastSentHeartbeat+MillisecondDuration(HEARTBEATS_MILLISECOND)){

		// Send one //
        //ResponseHeartbeat
        //CorrespondingConnection

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
DLLEXPORT COMMANDSENDER_PERMISSIONMODE ConnectedPlayer::GetPermissionMode(){
	return COMMANDSENDER_PERMISSIONMODE_NORMAL;
}

DLLEXPORT bool ConnectedPlayer::_OnSendPrivateMessage(const std::string &message){
	
	Logger::Get()->Write("Probably should implement a ChatManager");
	return false;
}
// ------------------------------------ //
DLLEXPORT ObjectID ConnectedPlayer::GetPositionInWorld(GameWorld* world, Lock &guard)
    const
{
	// Not found for that world //
	return 0;
}


