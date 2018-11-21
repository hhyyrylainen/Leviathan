// ------------------------------------ //
#include "ConnectedPlayer.h"

#include "Connection.h"

using namespace Leviathan;
// ------------------------------------ //
ConnectedPlayer::ConnectedPlayer(std::shared_ptr<Connection> connection,
    NetworkServerInterface* owninginstance, int plyid) :
    CorrespondingConnection(connection),
    Owner(owninginstance), UsingHeartbeats(false), IsControlLost(false),
    SecondsWithoutConnection(0.f), ID(plyid)
{}

DLLEXPORT ConnectedPlayer::~ConnectedPlayer()
{

    _OnReleaseParentCommanders();
}
// ------------------------------------ //
DLLEXPORT bool ConnectedPlayer::IsConnectionClosed() const
{
    return CorrespondingConnection ? !CorrespondingConnection->IsValidForSend() : true;
}
// ------------------------------------ //
DLLEXPORT void ConnectedPlayer::OnKicked(const std::string& reason)
{

    // Send a close connection packet //

    if(CorrespondingConnection) {

        // \todo Add the reason here
        CorrespondingConnection->SendCloseConnectionPacket();
    }


    // Broadcast a kick message on the server here //
}
// ------------------------------------ //
DLLEXPORT void ConnectedPlayer::StartHeartbeats()
{

    // Send a start packet //

    DEBUG_BREAK;
    // ResponseStartHeartbeats;

    // Reset our variables //
    UsingHeartbeats = true;

    LastReceivedHeartbeat = Time::GetThreadSafeSteadyTimePoint();
    LastSentHeartbeat = LastReceivedHeartbeat;
    SecondsWithoutConnection = 0.f;
}

DLLEXPORT void ConnectedPlayer::HeartbeatReceived()
{
    // Reset all timers //

    LastReceivedHeartbeat = Time::GetThreadSafeSteadyTimePoint();

    // Re-acquire controls, if lost in the first place //
    if(IsControlLost) {


        IsControlLost = false;
    }
}

DLLEXPORT void ConnectedPlayer::UpdateHeartbeats()
{
    // Skip if not used //
    if(!UsingHeartbeats)
        return;

    // Check do we need to send one //
    auto timenow = Time::GetThreadSafeSteadyTimePoint();

    if(timenow >= LastSentHeartbeat + MillisecondDuration(HEARTBEATS_MILLISECOND)) {

        // Send one //
        // ResponseHeartbeat
        // CorrespondingConnection

        LastSentHeartbeat = timenow;
    }

    // Update the time without a response //
    SecondsWithoutConnection = SecondDuration(timenow - LastReceivedHeartbeat).count();

    // Do something if the time is too high //
    if(SecondsWithoutConnection >= 2.f) {


        IsControlLost = true;
    }
}
// ------------------------------------ //
DLLEXPORT COMMANDSENDER_PERMISSIONMODE ConnectedPlayer::GetPermissionMode()
{
    return COMMANDSENDER_PERMISSIONMODE_NORMAL;
}

DLLEXPORT bool ConnectedPlayer::_OnSendPrivateMessage(const std::string& message)
{

    Logger::Get()->Write("Probably should implement a ChatManager");
    return false;
}
// ------------------------------------ //
DLLEXPORT ObjectID ConnectedPlayer::GetPositionInWorld(GameWorld* world) const
{
    // Not found for that world //
    return 0;
}
