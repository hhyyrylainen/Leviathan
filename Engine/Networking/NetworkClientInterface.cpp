// ------------------------------------ //
#include "NetworkClientInterface.h"

#include "Application/Application.h"
#include "Connection.h"
#include "Engine.h"
#include "Entities/GameWorld.h"
#include "Exceptions.h"
#include "Iterators/StringIterator.h"
#include "NetworkHandler.h"
#include "NetworkRequest.h"
#include "Networking/NetworkCache.h"
#include "Networking/SentNetworkThing.h"
#include "SyncedVariables.h"
#include "Threading/ThreadingManager.h"
#include "TimeIncludes.h"
#include "Utility/Convert.h"

#include "Engine.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT NetworkClientInterface::NetworkClientInterface() :
    NetworkInterface(NETWORKED_TYPE::Client)
{}

DLLEXPORT NetworkClientInterface::~NetworkClientInterface() {}
// ------------------------------------ //
DLLEXPORT void NetworkClientInterface::HandleRequestPacket(
    const std::shared_ptr<NetworkRequest>& request, Connection& connection)
{
    if(_HandleDefaultRequest(request, connection))
        return;

    // switch (request->GetType()) {
    // }

    if(_CustomHandleRequestPacket(request, connection))
        return;

    LOG_ERROR(
        "NetworkClientInterface: failed to handle request of type: " + request->GetTypeStr());
}

DLLEXPORT void NetworkClientInterface::HandleResponseOnlyPacket(
    const std::shared_ptr<NetworkResponse>& message, Connection& connection)
{
    LEVIATHAN_ASSERT(message, "_HandleClientResponseOnly message is null");

    if(_HandleDefaultResponseOnly(message, connection))
        return;

    switch(message->GetType()) {
    case NETWORK_RESPONSE_TYPE::StartWorldReceive: {

        auto data = static_cast<ResponseStartWorldReceive*>(message.get());

        _HandleWorldJoinResponse(data->WorldID, data->WorldType, data->ExtraOptions);
        return;
    }
    case NETWORK_RESPONSE_TYPE::ServerHeartbeat: {
        // We got a heartbeat //
        _OnHeartbeat();
        // Avoid spamming keep alive packets //
        // dontmarkasreceived = true;
        return;
    }
    case NETWORK_RESPONSE_TYPE::StartHeartbeats: {
        // We need to start sending heartbeats //
        _OnStartHeartbeats();
        return;
    }
    case NETWORK_RESPONSE_TYPE::CacheUpdated: {
        auto data = static_cast<ResponseCacheUpdated*>(message.get());

        if(!Owner->GetCache()->HandleUpdatePacket(data)) {

            LOG_WARNING("NetworkClientInterface: applying AI cache "
                        "update failed");
        }

        return;
    }
    case NETWORK_RESPONSE_TYPE::EntityCreation: {
        auto data = static_cast<ResponseEntityCreation*>(message.get());

        auto world = _GetWorldForEntityMessage(data->WorldID);

        // TODO: this needs to be queued if we haven't received the world yet
        if(!world) {
            LOG_WARNING("NetworkClientInterface: no world found for EntityCreation. TODO: "
                        "queue this message");
            return;
        }

        ObjectID created = world->HandleEntityPacket(*data);

        if(created != NULL_OBJECT)
            _OnEntityReceived(world, created);
        return;
    }
    case NETWORK_RESPONSE_TYPE::EntityDestruction: {
        auto data = static_cast<ResponseEntityDestruction*>(message.get());

        auto world = _GetWorldForEntityMessage(data->WorldID);

        // TODO: this needs to be queued if we haven't received the world yet
        if(!world) {
            LOG_WARNING("NetworkClientInterface: no world found for EntityDestruction. TODO: "
                        "queue this message");
            return;
        }

        world->HandleEntityPacket(*data);
        return;
    }
    case NETWORK_RESPONSE_TYPE::EntityUpdate: {
        auto data = static_cast<ResponseEntityUpdate*>(message.get());

        auto world = _GetWorldForEntityMessage(data->WorldID);

        // TODO: this needs to be queued if we haven't received the world yet
        if(!world) {
            LOG_WARNING("NetworkClientInterface: no world found for EntityUpdate. TODO: "
                        "queue this message");
            return;
        }

        world->HandleEntityPacket(std::move(*data), connection);
        return;
    }
    case NETWORK_RESPONSE_TYPE::EntityLocalControlStatus: {
        auto data = static_cast<ResponseEntityLocalControlStatus*>(message.get());

        auto world = _GetWorldForEntityMessage(data->WorldID);

        // TODO: this needs to be queued if we haven't received the world yet
        if(!world) {
            LOG_WARNING(
                "NetworkClientInterface: no world found for EntityLocalControlStatus. TODO: "
                "queue this message");
            return;
        }

        world->HandleEntityPacket(*data);
        _OnLocalControlChanged(world);
        return;
    }
    default: break;
    }

    if(_CustomHandleResponseOnlyPacket(message, connection))
        return;

    LOG_ERROR("NetworkClientInterface: failed to handle response only of type: " +
              message->GetTypeStr());
}
// ------------------------------------ //
DLLEXPORT bool NetworkClientInterface::JoinServer(std::shared_ptr<Connection> connectiontouse)
{
    // Fail if already connected //
    if(ServerConnection) {

        LOG_ERROR("NetworkClientInterface: JoinServer: trying to join a server "
                  "while connected to another");
        DisconnectFromServer("Trying to connect to another server");
        return false;
    }

    // Store the connection //
    ServerConnection = connectiontouse;

    KeepAliveQueued = false;
    UsingHeartbeats = false;

    ConnectState = CLIENT_CONNECTION_STATE::WaitingForOpening;

    // Send message //
    _OnNewConnectionStatusMessage("Trying to connect to a server on " +
                                  ServerConnection->GenerateFormatedAddressString());

    return true;
}

DLLEXPORT void NetworkClientInterface::DisconnectFromServer(
    const std::string& reason, bool connectiontimedout)
{
    // Return if no connection //
    if(!ServerConnection) {

        LOG_WARNING("NetworkClientInterface: DisconnectFromServer: not connected "
                    "to any servers");
        return;
    }

    LOG_INFO("Client: disconnected from server, reason: " + reason);

    // Discard waiting requests //
    OurSentRequests.clear();

    // Send disconnect message to server //
    _OnNewConnectionStatusMessage("Disconnected from " +
                                  ServerConnection->GenerateFormatedAddressString() +
                                  ", reason: " + reason);

    // TODO: clear synced variables
    // Owner->GetSyncedVariables()->Clear();

    // Close connection //
    LOG_WRITE("TODO: send disconnect reason to server");
    Owner->CloseConnection(*ServerConnection);

    if(ServerConnection) {
        ServerConnection->SendCloseConnectionPacket();
    }

    ServerConnection.reset();

    ConnectState = CLIENT_CONNECTION_STATE::Closed;
    OurPlayerID = -1;

    _OnDisconnectFromServer(reason, connectiontimedout ? false : true);
}

DLLEXPORT std::vector<std::shared_ptr<Leviathan::Connection>>&
    NetworkClientInterface::GetClientConnections()
{
    LOG_FATAL("Calling GetClientConnections on a client interface");
    throw Exception("Calling GetClientConnections on a client interface");
}
// ------------------------------------ //
void NetworkClientInterface::_TickServerConnectionState()
{
    switch(ConnectState) {
    case CLIENT_CONNECTION_STATE::None: return;
    case CLIENT_CONNECTION_STATE::Closed: {
        // Everything should have been handled already
        ConnectState = CLIENT_CONNECTION_STATE::None;
        return;
    }
    case CLIENT_CONNECTION_STATE::WaitingForOpening: {
        if(!Owner->IsConnectionValid(*ServerConnection)) {

            LOG_WARNING("Connection to server failed");
            DisconnectFromServer("Server Refused Connection", true);
            LEVIATHAN_ASSERT(ConnectState != CLIENT_CONNECTION_STATE::WaitingForOpening,
                "DisconnectFromServer didn't reset client connect state");
            return;
        }

        if(ServerConnection->GetState() == CONNECTION_STATE::Authenticated) {

            LOG_INFO("NetworkClientInterface: sending JoinServer request");

            auto sentthing = ServerConnection->SendPacketToConnection(
                std::make_shared<RequestJoinServer>(-1), RECEIVE_GUARANTEE::Critical);

            // Monitor result
            OurSentRequests.push_back(sentthing);

            ConnectState = CLIENT_CONNECTION_STATE::SentJoinRequest;
        }

        return;
    }
    case CLIENT_CONNECTION_STATE::SentJoinRequest: return;
    case CLIENT_CONNECTION_STATE::Connected: break;
    }

    // TODO: cause a disconnect if no server connection
    LEVIATHAN_ASSERT(ServerConnection, "ServerConnection null when ConnectState isn't None");

    // Check did The connection close //
    if(!ServerConnection->IsValidForSend()) {
        LOG_WARNING("NetworkClientInterface: Connection to server died");
        DisconnectFromServer("Lost connection to server", false);
        return;
    }

    if(KeepAliveQueued) {

        // Send a keep alive //
        ServerConnection->SendKeepAlivePacket();

        KeepAliveQueued = false;
    }

    // Send heartbeats //
    _UpdateHeartbeats();
}

DLLEXPORT void NetworkClientInterface::TickIt()
{
    _TickServerConnectionState();

sentrequestloopbegin:

    // Check status of requests //
    for(auto iter = OurSentRequests.begin(); iter != OurSentRequests.end();) {

        // Check can we handle it //
        if((*iter)->IsFinalized()) {

            // Store a copy and delete from the vector //
            auto tmpsendthing = *iter;
            iter = OurSentRequests.erase(iter);

            // Handle the request //
            if(!(*iter)->GetStatus() || !(*iter)->GotResponse) {
                // It failed //

                Logger::Get()->Warning("NetworkClientInterface: request to server failed, "
                                       "possibly retrying:");

                _ProcessFailedRequest(tmpsendthing, tmpsendthing->GotResponse);
            } else {

                Logger::Get()->Info("Received a response to client request");

                // Handle it //
                _ProcessCompletedRequest(tmpsendthing, tmpsendthing->GotResponse);
            }

            // We need to loop again, because our iterator is now invalid, because
            // quite often failed things are retried
            goto sentrequestloopbegin;
        }

        // Can't handle, continue looping //
        ++iter;
    }
}
// ------------------------------------ //
void NetworkClientInterface::_ProcessCompletedRequest(
    std::shared_ptr<SentRequest> tmpsendthing, std::shared_ptr<NetworkResponse> response)
{
    LEVIATHAN_ASSERT(tmpsendthing->SentRequestData,
        "ClientInterface processing request with no SentRequestData");

    // Handle it //
    switch(tmpsendthing->SentRequestData->GetType()) {
    case NETWORK_REQUEST_TYPE::JoinServer: {
        // Check what we got back //
        switch(response->GetType()) {
        default:
        case NETWORK_RESPONSE_TYPE::ServerDisallow: {
            // We need to do something to fix this
            auto resdata = static_cast<ResponseServerDisallow*>(response.get());

            LOG_ERROR("NetworkClientInterface: Server didn't allow our request "
                      ", code: " + // std::to_string(resdata->Reason) +
                      std::to_string(static_cast<int>(resdata->Reason)) +
                      " message: " + resdata->Message);

            LOG_INFO("TODO: check can we recover from that failure");

            // Fallback to the generic failure handling
            _ProcessFailedRequest(tmpsendthing, response);

            break;
        }
        case NETWORK_RESPONSE_TYPE::ServerAllow: {
            // Properly joined //
            {
                // Get our ID from the message //
                auto resdata = static_cast<ResponseServerAllow*>(response.get());

                // We need to parse our ID from the response //
                StringIterator itr(resdata->Message);

                auto numberthing = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

                // Invalid format //
                if(!numberthing || numberthing->empty())
                    goto networkresponseserverallowinvalidreponseformatthinglabel;

                OurPlayerID = Convert::StringTo<int>(*numberthing);

                if(OurPlayerID < 0)
                    goto networkresponseserverallowinvalidreponseformatthinglabel;

                Logger::Get()->Info("NetworkClientInterface: our player ID is now: " +
                                    Convert::ToString(OurPlayerID));

                _ProperlyConnectedToServer();
                return;
            }
        networkresponseserverallowinvalidreponseformatthinglabel:

            Logger::Get()->Error("NetworkClientInterface: server join response has "
                                 "invalid format, disconnecting");
            DisconnectFromServer("Server sent an invalid response to join "
                                 "request",
                true);
            break;
        }
        }
        break;
    }
    case NETWORK_REQUEST_TYPE::RequestCommandExecution: {
        // It doesn't matter what we got back since the only important thing is
        // that the packet made it there
        break;
    }
    default:
        LOG_ERROR("NetworkClientInterface: WE MADE AN INVALID REQUEST, unknown type");
        DEBUG_BREAK;
    }
}

void NetworkClientInterface::_ProcessFailedRequest(
    std::shared_ptr<SentRequest> tmpsendthing, std::shared_ptr<NetworkResponse> response)
{
    LEVIATHAN_ASSERT(tmpsendthing->SentRequestData,
        "ClientInterface processing request with no SentRequestData");

    if(!ServerConnection || !ServerConnection->IsValidForSend()) {

        LOG_WRITE("\t> Connection has been closed (Perhaps the request was Critical)");
        return;
    }

    // First do some checks based on the request type //
    switch(tmpsendthing->SentRequestData->GetType()) {

    default: {
        LOG_ERROR("\t> Unknown request type; can't recover. Closing connection");
        DisconnectFromServer("A critical request to the server failed.");
        break;
    }
    }
}
// ------------------------------------ //
void NetworkClientInterface::_ProperlyConnectedToServer()
{
    // Set the variables //
    ConnectState = CLIENT_CONNECTION_STATE::Connected;

    // TODO: clear synced variables
    // Owner->GetSyncedVariables()->Clear();

    // Send connect message //
    _OnNewConnectionStatusMessage(
        "Established a connection with " + ServerConnection->GenerateFormatedAddressString());

    // Call the callback //
    _OnProperlyConnected();
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<SentRequest> NetworkClientInterface::DoJoinDefaultWorld()
{
    if(ConnectState != CLIENT_CONNECTION_STATE::Connected || !ServerConnection) {
        LOG_ERROR("NetworkClientInterface: DoJoinDefaultWorld: not connected to a server");
        return nullptr;
    }

    LOG_INFO("NetworkClientInterface: requesting to join default world on server");
    return ServerConnection->SendPacketToConnection(
        std::make_shared<RequestJoinGame>(), RECEIVE_GUARANTEE::Critical);
}

DLLEXPORT void NetworkClientInterface::_OnWorldJoined(std::shared_ptr<GameWorld> world)
{
    LOG_WARNING(
        "Joined world but OnWorldJoined is not overridden, the world won't be displayed");
}

DLLEXPORT void NetworkClientInterface::_HandleWorldJoinResponse(
    int32_t worldid, int32_t worldtype, const std::string& extraoptions)
{
    auto world = Engine::Get()->CreateWorld(GetWindowForWorldJoin(extraoptions), worldtype,
        GetPhysicsMaterialsForReceivedWorld(worldtype, extraoptions),
        WorldNetworkSettings::GetSettingsForClient(), worldid);

    if(!world) {

        LOG_ERROR("NetworkClientInterface: _HandleWorldJoinResponse: failed to create world");
        DisconnectFromServer("Cannot create requested world type");
        return;
    }

    world->SetServerForLocalControl(ServerConnection);

    OurReceivedWorld = world;
    _OnWorldJoined(OurReceivedWorld);
}

DLLEXPORT Window* NetworkClientInterface::GetWindowForWorldJoin(
    const std::string& extraoptions)
{
    return Engine::Get()->GetWindowEntity();
}

DLLEXPORT std::shared_ptr<PhysicsMaterialManager>
    NetworkClientInterface::GetPhysicsMaterialsForReceivedWorld(
        int32_t worldtype, const std::string& extraoptions)
{
    LOG_INFO("NetworkClientInterface: default GetPhysicsMaterialsForReceivedWorld called, "
             "world won't have physics");
    return nullptr;
}

DLLEXPORT GameWorld* NetworkClientInterface::_GetWorldForEntityMessage(int32_t worldid)
{
    if(OurReceivedWorld && OurReceivedWorld->GetID() == worldid)
        return OurReceivedWorld.get();

    LOG_WARNING("NetworkClientInterface: failed to find world with id for entity update "
                "message, id: " +
                std::to_string(worldid));
    return nullptr;
}

DLLEXPORT void NetworkClientInterface::_OnLocalControlChanged(GameWorld* world)
{
    LOG_WARNING(
        "NetworkClientInterface: base implementation of _OnLocalControlChanged called");
}

DLLEXPORT void NetworkClientInterface::_OnEntityReceived(GameWorld* world, ObjectID created) {}
// ------------------------------------ //
DLLEXPORT void NetworkClientInterface::OnUpdateFullSynchronizationState(
    size_t variablesgot, size_t expectedvariables)
{
    _OnNewConnectionStatusMessage("Syncing variables, " + Convert::ToString(variablesgot) +
                                  "/" + Convert::ToString(expectedvariables));
}
// ------------------------------------ //
DLLEXPORT void NetworkClientInterface::CloseDown()
{
    _OnCloseDown();

    if(ServerConnection) {

        DisconnectFromServer("Game closing");
    }
}
// ------------------------------------ //
DLLEXPORT void NetworkClientInterface::SendCommandStringToServer(const std::string& messagestr)
{
    // Make sure that we are connected to a server //
    if(ConnectState != CLIENT_CONNECTION_STATE::Connected) {

        throw InvalidState("cannot send command because we aren't connected to a server");
    }

    // Check the length //
    if(messagestr.length() >= MAX_SERVERCOMMAND_LENGTH) {

        throw InvalidArgument("server command is too long");
    }

    // Send it //
    auto sendthing = ServerConnection->SendPacketToConnection(
        std::make_shared<RequestRequestCommandExecution>(messagestr),
        RECEIVE_GUARANTEE::Critical);

    // The packet may not fail so we need to monitor the response //
    OurSentRequests.push_back(sendthing);
}
// ------------------------------------ //
DLLEXPORT bool NetworkClientInterface::IsConnected() const
{
    return ConnectState != CLIENT_CONNECTION_STATE::None &&
           ConnectState != CLIENT_CONNECTION_STATE::Closed;
}
// ------------------------------------ //
void NetworkClientInterface::_OnStartHeartbeats()
{
    // Ignore if already started /
    if(UsingHeartbeats)
        return;

    // Reset heartbeat variables //
    LastReceivedHeartbeat = Time::GetThreadSafeSteadyTimePoint();
    LastSentHeartbeat = LastReceivedHeartbeat;

    SecondsWithoutConnection = 0.f;

    // And start using them //
    UsingHeartbeats = true;
}

void NetworkClientInterface::_OnHeartbeat()
{
    // Reset the times //
    LastReceivedHeartbeat = Time::GetThreadSafeSteadyTimePoint();
    SecondsWithoutConnection = 0.f;
}

void NetworkClientInterface::_UpdateHeartbeats()
{
    // Skip if not in use //
    if(!UsingHeartbeats)
        return;

    if(!ServerConnection)
        return;

    // Check do we need to send one //
    auto timenow = Time::GetThreadSafeSteadyTimePoint();

    if(timenow >= LastSentHeartbeat + MillisecondDuration(HEARTBEATS_MILLISECOND)) {

        // Send one //
        ServerConnection->SendPacketToConnection(
            std::make_shared<ResponseNone>(NETWORK_RESPONSE_TYPE::ServerHeartbeat),
            RECEIVE_GUARANTEE::None);

        LastSentHeartbeat = timenow;
    }

    // Update the time without a response //
    SecondsWithoutConnection = SecondDuration(timenow - LastSentHeartbeat).count();

    // Do something if the time is too high //
    if(SecondsWithoutConnection >= 2.f) {

        DEBUG_BREAK;
    }
}
// ------------------------------------ //
DLLEXPORT int NetworkClientInterface::GetOurID() const
{
    return OurPlayerID;
}

DLLEXPORT std::shared_ptr<Connection> NetworkClientInterface::GetServerConnection()
{
    return ServerConnection;
}
// ------------------------------------ //
DLLEXPORT void NetworkClientInterface::MarkForNotifyReceivedStates()
{
    KeepAliveQueued = true;
}
// ------------------------------------ //
