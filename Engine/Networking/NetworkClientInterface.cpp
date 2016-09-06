// ------------------------------------ //
#include "NetworkClientInterface.h"

#include "NetworkHandler.h"
#include "Connection.h"
#include "SyncedVariables.h"
#include "Engine.h"
#include "Exceptions.h"
#include "NetworkRequest.h"
#include "../TimeIncludes.h"
#include "Iterators/StringIterator.h"
#include "Application/Application.h"
#include "Entities/GameWorld.h"
#include "Threading/ThreadingManager.h"
#include "Networking/NetworkCache.h"
#include "../Utility/Convert.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

// ------------------ NetworkClientInterface ------------------ //
DLLEXPORT Leviathan::NetworkClientInterface::NetworkClientInterface() : 
    NetworkInterface(NETWORKED_TYPE::Client)
{
}

DLLEXPORT Leviathan::NetworkClientInterface::~NetworkClientInterface(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::HandleRequestPacket(
    std::shared_ptr<NetworkRequest> request, Connection &connection) 
{

    if (_HandleDefaultRequest(request, connection))
        return;

    // switch (request->GetType()) {
    // }

    if (_CustomHandleRequestPacket(request, connection))
        return;

    LOG_ERROR("NetworkClientInterface: failed to handle request of type: " +
        Convert::ToString(static_cast<int>(request->GetType())));
}

DLLEXPORT void Leviathan::NetworkClientInterface::HandleResponseOnlyPacket(
    std::shared_ptr<NetworkResponse> message, Connection &connection, 
    bool &dontmarkasreceived) 
{
    LEVIATHAN_ASSERT(message, "_HandleClientResponseOnly message is null");

    if (_HandleDefaultResponseOnly(message, connection, dontmarkasreceived))
        return;

    switch (message->GetType()) {
    case NETWORK_RESPONSE_TYPE::ServerHeartbeat:
    {
        // We got a heartbeat //
        _OnHeartbeat();
        // Avoid spamming keep alive packets //
        //dontmarkasreceived = true;
        return;
    }
    case NETWORK_RESPONSE_TYPE::StartHeartbeats:
    {
        // We need to start sending heartbeats //
        _OnStartHeartbeats();
        return;
    }
    case NETWORK_RESPONSE_TYPE::CacheUpdated:
    {
        auto data = static_cast<ResponseCacheUpdated*>(message.get());

        if (!Owner->GetCache()->HandleUpdatePacket(data)) {

            LOG_WARNING("NetworkClientInterface: applying AI cache "
                "update failed");
        }

        return;
    }
    default:
        break;
    }

    if (_CustomHandleResponseOnlyPacket(message, connection, dontmarkasreceived))
        return;

    LOG_ERROR("NetworkClientInterface: failed to handle response of type: " +
        Convert::ToString(static_cast<int>(message->GetType())));
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkClientInterface::JoinServer(
    shared_ptr<Connection> connectiontouse)
{
    GUARD_LOCK();

    // Fail if already connected //
    if(ServerConnection){

        Logger::Get()->Error("NetworkClientInterface: JoinServer: trying to join a server "
            "while connected to another");
        DisconnectFromServer(guard, "Trying to connect to another server");
        return false;
    }

    // Store the connection //
    ServerConnection = connectiontouse;

    KeepAliveQueued = false;
    UsingHeartbeats = false;

    ConnectState = CLIENT_CONNECTION_STATE::WaitingForOpening;

    // Send message //
    _OnNewConnectionStatusMessage("Trying to connect to a server on "+ServerConnection->
        GenerateFormatedAddressString());

    return true;
}

DLLEXPORT void Leviathan::NetworkClientInterface::DisconnectFromServer(Lock &guard,
    const std::string &reason, bool connectiontimedout)
{
    // Return if no connection //
    if(!ServerConnection){

        LOG_WARNING("NetworkClientInterface: DisconnectFromServer: not connected "
            "to any servers");
        return;
    }

    // Discard waiting requests //
    OurSentRequests.clear();

    // Send disconnect message to server //
    _OnNewConnectionStatusMessage("Disconnected from " +
        ServerConnection->GenerateFormatedAddressString() + ", reason: " +reason);

    // TODO: clear synced variables
    //Owner->GetSyncedVariables()->Clear();

    // Close connection //
    LOG_WRITE("TODO: send disconnect reason to server");
    Owner->CloseConnection(*ServerConnection);
    ServerConnection.reset();

    ConnectState = CLIENT_CONNECTION_STATE::Closed;
    OurPlayerID = -1;

    _OnDisconnectFromServer(reason, connectiontimedout ? false: true);
}

DLLEXPORT std::vector<std::shared_ptr<Leviathan::Connection>>& 
Leviathan::NetworkClientInterface::GetClientConnections() 
{
    LOG_FATAL("Calling GetClientConnections on a client interface");
    throw Exception("Calling GetClientConnections on a client interface");
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::TickIt(){
    
    GUARD_LOCK();

    switch(ConnectState){
    case CLIENT_CONNECTION_STATE::None:
        return;
    case CLIENT_CONNECTION_STATE::Closed:
    {
        // Everything should have been handled already
        ConnectState = CLIENT_CONNECTION_STATE::None;
        return;
    }
    case CLIENT_CONNECTION_STATE::WaitingForOpening:
    {
        if(!Owner->IsConnectionValid(*ServerConnection)){

            DisconnectFromServer(guard, "Server Refused Connection", true);
            return;
        }

        
        return;
    }
    default:
        break;
    }

    LEVIATHAN_ASSERT(ServerConnection, "ServerConnection null when ConnectState isn't None");

    // Check did The connection close //

    if(KeepAliveQueued){

        // Send a keep alive //
        if(ServerConnection){

            ServerConnection->SendKeepAlivePacket();
            
        } else {

            LOG_WARNING("NetworkClientInterface: cannot send keep alive because "
                "ServerConnection is empty");
        }
        
        KeepAliveQueued = false;
    }

checksentrequestsbeginlabel:


    // Check status of requests //
    for(auto iter = OurSentRequests.begin(); iter != OurSentRequests.end(); ){

        // Check can we handle it //
        if((*iter)->IsFinalized()){
            // Handle the request //
            if(!(*iter)->GetStatus() || !(*iter)->GotResponse){
                // It failed //

                Logger::Get()->Warning("NetworkClientInterface: request to server failed, "
                    "possibly retrying:");

                // Store a copy and delete from the vector //
                shared_ptr<SentNetworkThing> tmpsendthing = (*iter);
                iter = OurSentRequests.erase(iter);

                _ProcessFailedRequest(guard, tmpsendthing, tmpsendthing->GotResponse);

                // We need to loop again, because our iterator is now invalid, because
                // quite often failed things are retried
                goto checksentrequestsbeginlabel;
            }

            Logger::Get()->Info("Received a response to client request");

            // Handle it //
            _ProcessCompletedRequest(guard, *iter, (*iter)->GotResponse);

            // This is now received/handled //
            iter = OurSentRequests.erase(iter);
            continue;
        }

        // Can't handle, continue looping //
        ++iter;
    }

    // Send heartbeats //
    _UpdateHeartbeats(guard);

}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_ProcessCompletedRequest(
    Lock &guard, std::shared_ptr<SentNetworkThing> tmpsendthing, 
    std::shared_ptr<NetworkResponse> response)
{
    LEVIATHAN_ASSERT(tmpsendthing->UserCodeRequestStore,
        "ClientInterface processing request with no UserCodeRequestStore");

    // Handle it //
    switch(tmpsendthing->UserCodeRequestStore->GetType()){
    case NETWORK_REQUEST_TYPE::JoinServer:
        {
            // Check what we got back //
            switch(response->GetType()){
            default:
            case NETWORK_RESPONSE_TYPE::ServerDisallow:
                {
                    // We need to do something to fix this
                    DEBUG_BREAK;
                }
                break;
            case NETWORK_RESPONSE_TYPE::ServerAllow:
                {
                    // Properly joined //
                    {
                        // Get our ID from the message //
                        auto resdata = static_cast<ResponseServerAllow*>(response.get());

                        // We need to parse our ID from the response //
                        StringIterator itr(resdata->Message);

                        auto numberthing =
                            itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

                        // Invalid format //
                        if(!numberthing || numberthing->empty())
                            goto networkresponseserverallowinvalidreponseformatthinglabel;

                        OurPlayerID = Convert::StringTo<int>(*numberthing);

                        if(OurPlayerID < 0)
                            goto networkresponseserverallowinvalidreponseformatthinglabel;

                        Logger::Get()->Info("NetworkClientInterface: our player ID is now: "+
                            Convert::ToString(OurPlayerID));

                        _ProperlyConnectedToServer(guard);
                        return;
                    }
networkresponseserverallowinvalidreponseformatthinglabel:

                    Logger::Get()->Error("NetworkClientInterface: server join response has "
                        "invalid format, disconnecting");
                    DisconnectFromServer(guard, "Server sent an invalid response to join "
                        "request", true);
                }
                break;
            }
        }
        break;
    case NETWORK_REQUEST_TYPE::RequestCommandExecution:
        {
            // It doesn't matter what we got back since the only important thing is
            // that the packet made it there
        }
        break;
    default:
        LOG_ERROR("NetworkClientInterface: WE MADE AN INVALID REQUEST, unknown type");
        DEBUG_BREAK;
    }
}

void Leviathan::NetworkClientInterface::_ProcessFailedRequest(
    Lock &guard, std::shared_ptr<SentNetworkThing> tmpsendthing, 
    std::shared_ptr<NetworkResponse> response)
{
    LEVIATHAN_ASSERT(tmpsendthing->UserCodeRequestStore,
        "ClientInterface processing request with no UserCodeRequestStore");

    if(!ServerConnection || !ServerConnection->IsOpen()){

        LOG_WRITE("\t> Connection has been closed (Perhaps the request was Critical)");
        return;
    }

    // First do some checks based on the request type //
    switch(tmpsendthing->UserCodeRequestStore->GetType()){
    
    default:
        LOG_WRITE("\t> Unknown request type, probably not important, "
            "not doing anything about it failing");
    }
}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_ProperlyConnectedToServer(Lock &guard){
    VerifyLock(guard);

    // Set the variables //
    ConnectState = CLIENT_CONNECTION_STATE::Connected;

    // TODO: clear synced variables
    //Owner->GetSyncedVariables()->Clear();


    // Send connect message //
    _OnNewConnectionStatusMessage("Established a connection with " +
        ServerConnection->GenerateFormatedAddressString());

    // Call the callback //
    _OnProperlyConnected();
}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnProperlyConnected(){

    // Send the request //
    DEBUG_BREAK;
    //shared_ptr<NetworkRequest> tmprequest(
    //    new NetworkRequest(NETWORKREQUESTTYPE_GETALLSYNCVALUES));

    // Prepare for sync //
    //Owner->GetSyncedVariables()->PrepareForFullSync();
    //SyncedVariables::Get()->PrepareForFullSync();

    //auto receivedata = ServerConnection->SendPacketToConnection(tmprequest, 3);

    // Add a on completion function //
    DEBUG_BREAK;
 //   std::bind<void>([](shared_ptr<SentNetworkThing> maderequest,
 //               NetworkClientInterface* iptr) -> void
    //{
    //	// Check the status //
    //	if(!maderequest->GetStatus() || !maderequest->GotResponse){
    //		// Terminate the connection //
    //		DEBUG_BREAK;
    //		return;
    //	}

    //	// This will have the maximum number of variables we are going to receive //
    //	NetworkResponseDataForServerAllow* tmpresponse = maderequest->GotResponse->
 //           GetResponseDataForServerAllowResponse();
    //	
    //	if(!tmpresponse){

    //		Logger::Get()->Warning("NetworkClientInterface: connect sync: variable sync "
 //               "request returned and invalid response, expected ServerAllow, unknown count "
 //               "of synced variables");
    //	} else {

    //		// Set the maximum number of things //
    //		size_t toreceive = Convert::StringTo<size_t>(tmpresponse->Message);

    //		SyncedVariables::Get()->SetExpectedNumberOfVariablesReceived(toreceive);

    //		Logger::Get()->Info("NetworkClientInterface: sync variables: now expecting "+
 //               Convert::ToString(toreceive)+" variables");
    //	}


    //	
    //	// We are now syncing the variables //
    //	iptr->_OnNewConnectionStatusMessage("Syncing variables, waiting for response...");

    //	// Queue task that checks when it is done //
    //	Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(
 //               new ConditionalTask(std::bind<void>([](NetworkClientInterface* iptr) -> void
    //	{
    //		// Set the status to almost done //
    //		iptr->_OnNewConnectionStatusMessage("Finalizing connection");

    //		// We are now completely connected from the engine's point of view so let the
 //           // application know
    //		iptr->_OnStartApplicationConnect();

    //	}, iptr), std::bind<bool>([]() -> bool
    //	{
    //		return SyncedVariables::Get()->IsSyncDone();
    //	}))));


    //}, receivedata, this)
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::OnUpdateFullSynchronizationState(
    size_t variablesgot, size_t expectedvariables)
{

    _OnNewConnectionStatusMessage("Syncing variables, "+Convert::ToString(variablesgot)+
        "/"+Convert::ToString(expectedvariables));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::CloseDown(){

    _OnCloseDown();

    GUARD_LOCK();

    if(ServerConnection){


        DisconnectFromServer(guard, "Game closing");
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::SendCommandStringToServer(
    const string &messagestr)
{
    // Make sure that we are connected to a server //
    if(ConnectState != CLIENT_CONNECTION_STATE::Connected){

        throw InvalidState("cannot send command because we aren't connected to a server");
    }

    // Check the length //
    if(messagestr.length() >= MAX_SERVERCOMMAND_LENGTH){

        throw InvalidArgument("server command is too long");
    }

    // Create a packet //
    RequestRequestCommandExecution request(messagestr);

    // Send it //
    auto sendthing = ServerConnection->SendPacketToConnection(request, 
        RECEIVE_GUARANTEE::Critical);

    // The packet may not fail so we need to monitor the response //
    OurSentRequests.push_back(sendthing);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkClientInterface::IsConnected() const{
    return ConnectState != CLIENT_CONNECTION_STATE::None &&
        ConnectState != CLIENT_CONNECTION_STATE::Closed;
}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_OnStartHeartbeats(){
    GUARD_LOCK();

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

void Leviathan::NetworkClientInterface::_OnHeartbeat(){
    GUARD_LOCK();
    // Reset the times //
    LastReceivedHeartbeat = Time::GetThreadSafeSteadyTimePoint();
    SecondsWithoutConnection = 0.f;
}

void Leviathan::NetworkClientInterface::_UpdateHeartbeats(Lock &guard){
    // Skip if not in use //
    if(!UsingHeartbeats)
        return;

    if(!ServerConnection)
        return;

    // Check do we need to send one //
    auto timenow = Time::GetThreadSafeSteadyTimePoint();

    if(timenow >= LastSentHeartbeat+MillisecondDuration(HEARTBEATS_MILLISECOND)){

        // Send one //
        ResponseNone response(NETWORK_RESPONSE_TYPE::ServerHeartbeat);

        ServerConnection->SendPacketToConnection(response, RECEIVE_GUARANTEE::None);

        LastSentHeartbeat = timenow;
    }

    // Update the time without a response //
    SecondsWithoutConnection = SecondDuration(timenow-LastSentHeartbeat).count();

    // Do something if the time is too high //
    if(SecondsWithoutConnection >= 2.f){


        DEBUG_BREAK;
    }
}
// ------------------------------------ //
DLLEXPORT int Leviathan::NetworkClientInterface::GetOurID() const{
    return OurPlayerID;
}

DLLEXPORT std::shared_ptr<Connection>
    Leviathan::NetworkClientInterface::GetServerConnection()
{
    return ServerConnection;
}
// ------------------------------------ //
DLLEXPORT void NetworkClientInterface::MarkForNotifyReceivedStates(){

    GUARD_LOCK();
    KeepAliveQueued = true;
}
// ------------------------------------ //



