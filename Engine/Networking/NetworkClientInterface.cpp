#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKCLIENTINTERFACE
#include "NetworkClientInterface.h"
#endif
#include "NetworkHandler.h"
#include "ConnectionInfo.h"
#include "SyncedVariables.h"
#include "Engine.h"
#include "boost/thread/future.hpp"
#include "Exceptions/ExceptionInvalidState.h"
#include "NetworkRequest.h"
#include "Common/Misc.h"
#include "Iterators/StringIterator.h"
#include "NetworkedInputHandler.h"
#include "Application/Application.h"
#include "Entities/GameWorld.h"
#include "Threading/ThreadingManager.h"
using namespace Leviathan;
// ------------------------------------ //




// ------------------ NetworkClientInterface ------------------ //
DLLEXPORT Leviathan::NetworkClientInterface::NetworkClientInterface() :
    MaxConnectTries(DEFAULT_MAXCONNECT_TRIES), ConnectTriesCount(0), ConnectedToServer(false), UsingHeartbeats(false),
    SecondsWithoutConnection(0.f), OurPlayerID(-1), PotentialInputHandler(NULL)
{
	Staticaccess = this;
}

DLLEXPORT Leviathan::NetworkClientInterface::~NetworkClientInterface(){

	PotentialInputHandler.reset();

	Staticaccess = NULL;
}

DLLEXPORT NetworkClientInterface* Leviathan::NetworkClientInterface::GetIfExists(){
	return Staticaccess;
}

NetworkClientInterface* Leviathan::NetworkClientInterface::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkClientInterface::JoinServer(shared_ptr<ConnectionInfo> connectiontouse){
	GUARD_LOCK_THIS_OBJECT();

	// Fail if already connected //
	if(ServerConnection){

		Logger::Get()->Error(L"NetworkClientInterface: JoinServer: trying to join a server while connected to another");
		DisconnectFromServer(guard, L"Trying to connect to another");
		return false;
	}


	// Store the connection //
	ServerConnection = connectiontouse;

	ConnectToNotifier(ServerConnection.get());

	ConnectTriesCount = 0;

	// Send connect request //
	_SendConnectRequest(guard);

	return true;
}

DLLEXPORT void Leviathan::NetworkClientInterface::DisconnectFromServer(ObjectLock &guard, const wstring &reason,
    bool connectiontimedout)
{
	VerifyLock(guard);

	// Return if no connection //
	if(!ServerConnection){

		Logger::Get()->Info(L"NetworkClientInterface: DisconnectFromServer: not connected to any servers");
		return;
	}

	// Send disconnect message to server //
	_OnNewConnectionStatusMessage(L"Disconnected from "+ServerConnection->GenerateFormatedAddressString()+L", reason: "
        +reason);

	SyncedVariables::Get()->RemoveConnectionWithAnother(ServerConnection.get());

	// Close connection //
	NetworkHandler::Get()->SafelyCloseConnectionTo(ServerConnection.get());
	ServerConnection.reset();

	ConnectedToServer = false;
	OurPlayerID = -1;

	_OnDisconnectFromServer(reason, connectiontimedout ? false: true);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkClientInterface::_HandleClientRequest(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connectiontosendresult)
{
	// Try to handle input packet if we have the proper handler //
	if(PotentialInputHandler && PotentialInputHandler->HandleInputPacket(request, connectiontosendresult)){

		return true;
	}


    switch(request->GetType()){
        case NETWORKREQUESTTYPE_WORLD_CLOCK_SYNC:
        {

            // TODO: only allow this to be received from the server
            auto packetdata = request->GetWorldClockSyncRequestData();

            if(!packetdata){
                // Invalid data //
                return true;
            }

            // Find a matching world and handle it //

            // TODO: a thing that fetches the world from the thing that automatically creates worlds on the client
            
            // Get a matching world //
            auto world = LeviathanApplication::Get()->GetGameWorld(packetdata->WorldID);

            if(!world){

                Logger::Get()->Error("NetworkClientInterface: handle response: couldn't find a matching "
                    "world for world clock sync message, WorldID: "+Convert::ToString(packetdata->WorldID));
                return true;
            }

            world->HandleClockSyncPacket(packetdata);

            // Response to the request //
            auto response = make_shared<NetworkResponse>(request->GetExpectedResponseID(),
                PACKAGE_TIMEOUT_STYLE_TIMEDMS, 1000);

            response->GenerateEmptyResponse();

            // TODO: do something about this response potentially failing and the whole sync process having to be
            // redone
            connectiontosendresult->SendPacketToConnection(response, 1);
            
            return true;
        }
    }


	return false;
}

DLLEXPORT bool Leviathan::NetworkClientInterface::_HandleClientResponseOnly(shared_ptr<NetworkResponse> message,
    ConnectionInfo* connection, bool &dontmarkasreceived)
{
	// Try to handle input packet if we have the proper handler //
	if(PotentialInputHandler && PotentialInputHandler->HandleInputPacket(message, connection)){

		return true;
	}

	switch(message->GetType()){
        case NETWORKRESPONSETYPE_SERVERHEARTBEAT:
		{
			// We got a heartbeat //
			_OnHeartbeat();
			// Avoid spamming keepalive packets //
			//dontmarkasreceived = true;
			return true;
		}
        case NETWORKRESPONSETYPE_STARTHEARTBEATS:
		{
			// We need to start sending heartbeats //
			_OnStartHeartbeats();
			return true;
		}
        case NETWORKRESPONSETYPE_INITIAL_ENTITY:
        {
            // We received a new entity! //
            // TODO: do a system that automatically creates worlds on the client //
            ThreadingManager::Get()->QueueTask(new QueuedTask(boost::bind<void>([](shared_ptr<NetworkResponse> message)
                        -> void
                {

                    auto packetdata = message->GetResponseDataForInitialEntity();

                    if(!packetdata){
                        // Invalid data //
                        return;
                    }

                    // Get a matching world //
                    auto world = LeviathanApplication::Get()->GetGameWorld(packetdata->WorldID);

                    if(!world){

                        Logger::Get()->Error("NetworkClientInterface: handle response: couldn't find a matching "
                            "world for initial entity message, WorldID: "+Convert::ToString(packetdata->WorldID));
                        return;
                    }

                    if(!world->HandleEntityInitialPacket(packetdata)){

                        Logger::Get()->Error("NetworkClientInterface: failed to create an entity from an initial "
                            "entity packet");
                    }

                }, message)));

            
            // It will be handled soon //
            return true;
        }
        default:
            return false;
	}


	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::UpdateClientStatus(){
	GUARD_LOCK_THIS_OBJECT();

checksentrequestsbeginlabel:


	// Check status of requests //
	for(auto iter = OurSentRequests.begin(); iter != OurSentRequests.end(); ){

		// Check can we handle it //
		if((*iter)->GetFutureForThis().has_value()){
			// Handle the request //
			if(!(*iter)->GetFutureForThis().get() || !(*iter)->GotResponse){
				// It failed //

				Logger::Get()->Warning(L"NetworkClientInterface: request to server failed, possibly retrying:");

				// Store a copy and delete from the vector //
				shared_ptr<SentNetworkThing> tmpsendthing = (*iter);
				iter = OurSentRequests.erase(iter);

				_ProcessFailedRequest(tmpsendthing, guard);

				// We need to loop again, because our iterator is now invalid, because quite often failed things
                // are retried
				goto checksentrequestsbeginlabel;
			}

			Logger::Get()->Info(L"Received a response to client request");

			// Handle it //
			_ProcessCompletedRequest(*iter, guard);

			// This is now received/handled //
			iter = OurSentRequests.erase(iter);
			continue;
		}

		// Can't handle, continue looping //
		++iter;
	}

	// Send heartbeats //
	_UpdateHeartbeats();

	// Update networked input handling //
	if(PotentialInputHandler)
		PotentialInputHandler->UpdateInputStatus();

}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::_OnNotifierDisconnected(BaseNotifierAll* parenttoremove){
	GUARD_LOCK_THIS_OBJECT();

	// Get the close reason from it //
	wstring closereason;

	if(closereason.empty())
		closereason = L"Other side requested close";

	// Disconnect if got to the connected state //
	if(ServerConnection){
		// Send disconnect message to server //
		_OnNewConnectionStatusMessage(L"Disconnected from "+ServerConnection->GenerateFormatedAddressString()+
            L", reason: "+closereason);

		// Call the disconnect callback //
		_OnDisconnectFromServer(closereason, false);

		ConnectedToServer = false;

		// Let go of the connection //
		ServerConnection.reset();
	}
}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_SendConnectRequest(ObjectLock &guard){
	VerifyLock(guard);

	// Increase connect number //
	ConnectTriesCount++;


	// Send connect request //
	shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(new JoinServerRequestData()));

	auto sentthing = ServerConnection->SendPacketToConnection(tmprequest, 1);

	// Store it //
	OurSentRequests.push_back(sentthing);

	// Send message //
	_OnNewConnectionStatusMessage(L"Trying to connect to a server on "+ServerConnection->
        GenerateFormatedAddressString()+L", attempt "+Convert::ToWstring(ConnectTriesCount));
}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_ProcessCompletedRequest(shared_ptr<SentNetworkThing> tmpsendthing,
    ObjectLock &guard)
{
	VerifyLock(guard);

	// Handle it //
	switch(tmpsendthing->OriginalRequest->GetType()){
	case NETWORKREQUESTTYPE_JOINSERVER:
		{
			// Check what we got back //
			switch(tmpsendthing->GotResponse->GetTypeOfResponse()){
			default:
			case NETWORKRESPONSETYPE_SERVERDISALLOW:
				{
					// We need to do something to fix this
					DEBUG_BREAK;
				}
				break;
			case NETWORKRESPONSETYPE_SERVERALLOW:
				{
					// Properly joined //
					{
						// Get our ID from the message //
						auto resdata = tmpsendthing->GotResponse->GetResponseDataForServerAllowResponse();

						if(!resdata)
							goto networkresponseserverallowinvalidreponseformatthinglabel;

						// We need to parse our ID from the response //
						StringIterator itr(resdata->Message);

						auto numberthing = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_NONE);

						// Invalid format //
						if(!numberthing || numberthing->empty())
							goto networkresponseserverallowinvalidreponseformatthinglabel;

						OurPlayerID = Convert::WstringTo<int>(*numberthing);

						if(OurPlayerID < 0)
							goto networkresponseserverallowinvalidreponseformatthinglabel;

						Logger::Get()->Info(L"NetworkClientInterface: our player ID is now: "+
                            Convert::ToWstring(OurPlayerID));

						_ProperlyConnectedToServer(guard);
						return;
					}
networkresponseserverallowinvalidreponseformatthinglabel:

					Logger::Get()->Error("NetworkClientInterface: server join response has invalid format, "
                        "disconnecting");
					DisconnectFromServer(guard, L"Server sent an invalid response to join request", true);
				}
				break;
			}
		}
		break;
	case NETWORKREQUESTTYPE_REQUESTEXECUTION:
		{
			// It doesn't matter what we got back since the only important thing is that the packet made it there //
		}
		break;
	default:
		Logger::Get()->Info(L"NetworkClientInterface: WE MADE AN INVALID REQUEST");
		DEBUG_BREAK;
	}
}

void Leviathan::NetworkClientInterface::_ProcessFailedRequest(shared_ptr<SentNetworkThing> tmpsendthing,
    ObjectLock &guard)
{
	VerifyLock(guard);

	// First do some checks based on the request type //
	switch(tmpsendthing->OriginalRequest->GetType()){
	case NETWORKREQUESTTYPE_JOINSERVER:
		{
			if(ConnectTriesCount < MaxConnectTries){

				Logger::Get()->Write(L"\t> Retrying connect");
				_SendConnectRequest(guard);

			} else {

				Logger::Get()->Write(L"\t> Maximum connect tries reached");
				DisconnectFromServer(guard, L"Connection timed out after "+Convert::ToWstring(ConnectTriesCount)+
                    L" tries", true);
			}
		}
		break;
	case NETWORKREQUESTTYPE_REQUESTEXECUTION:
		{
			// This may never fail, connection needs to be terminated //
			DisconnectFromServer(guard, L"command/chat packet failed", true);
			Logger::Get()->Write(L"\t> Terminating connection since it shouldn't have been able to fail");
		}
		break;
	default:
		Logger::Get()->Write(L"\t> Unknown request type, probably not important, dropping request");
	}
}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_ProperlyConnectedToServer(ObjectLock &guard){
	VerifyLock(guard);

	// Set the variables //
	ConnectedToServer = true;

	// By default synchronize values and then pass to the pure virtual function for joining the match properly //
	SyncedVariables::Get()->AddAnotherToSyncWith(ServerConnection.get());

	// Send connect message //
	_OnNewConnectionStatusMessage(L"Established a connection with "+ServerConnection->GenerateFormatedAddressString());

	// Call the callback //
	_OnProperlyConnected();
}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnProperlyConnected(){

	// Send the request //
	shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(NETWORKREQUESTTYPE_GETALLSYNCVALUES));

	// Prepare for sync //
	SyncedVariables::Get()->PrepareForFullSync();

	auto receivedata = ServerConnection->SendPacketToConnection(tmprequest, 3);

	// Add a checking task //
	Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new ConditionalTask(
		boost::bind<void>([](shared_ptr<SentNetworkThing> maderequest, NetworkClientInterface* iptr) -> void
	{
		// Check the status //
		if(!maderequest->GetFutureForThis().get() || !maderequest->GotResponse){
			// Terminate the connection //
			DEBUG_BREAK;
			return;
		}

		// This will have the maximum number of variables we are going to receive //
		NetworkResponseDataForServerAllow* tmpresponse = maderequest->GotResponse->
            GetResponseDataForServerAllowResponse();
		
		if(!tmpresponse){

			Logger::Get()->Warning(L"NetworkClientInterface: connect sync: variable sync request returned and "
                L"invalid response, expected ServerAllow, unknown count of synced variables");
		} else {

			// Set the maximum number of things //
			size_t toreceive = Convert::WstringTo<size_t>(tmpresponse->Message);

			SyncedVariables::Get()->SetExpectedNumberOfVariablesReceived(toreceive);

			Logger::Get()->Info(L"NetworkClientInterface: sync variables: now expecting "+Convert::ToWstring(toreceive)+
                L" variables");
		}


		
		// We are now syncing the variables //
		iptr->_OnNewConnectionStatusMessage(L"Syncing variables, waiting for response...");

		// Queue task that checks when it is done //
		Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new ConditionalTask(
			boost::bind<void>([](NetworkClientInterface* iptr) -> void
		{
			// Set the status to almost done //
			iptr->_OnNewConnectionStatusMessage(L"Finalizing connection");

			// We are now completely connected from the engine's point of view so let the application know //
			iptr->_OnStartApplicationConnect();


		}, iptr), boost::bind<bool>([]() -> bool
		{
			return SyncedVariables::Get()->IsSyncDone();
		}))));


	}, receivedata, this), boost::bind<bool>([](shared_ptr<SentNetworkThing> maderequest) -> bool
	{
		return maderequest->GetFutureForThis().has_value();
	}, receivedata))));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::OnUpdateFullSynchronizationState(size_t variablesgot,
    size_t expectedvariables)
{

	_OnNewConnectionStatusMessage(L"Syncing variables, "+Convert::ToWstring(variablesgot)+L"/"+Convert::ToWstring(
            expectedvariables));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::OnCloseClient(){
	GUARD_LOCK_THIS_OBJECT();

	if(ServerConnection){


		DisconnectFromServer(guard, L"Game closing");
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::SendCommandStringToServer(const string &messagestr){
	// Make sure that we are connected to a server //
	if(!ConnectedToServer){

		throw ExceptionInvalidState(L"cannot send command because we aren't connected to a server", 0, __WFUNCTION__,
            L"not connected");
	}


	// Check the length //
	if(messagestr.length() >= MAX_SERVERCOMMAND_LENGTH){

		throw ExceptionInvalidArgument(L"server command is too long", messagestr.size(), __WFUNCTION__, L"messagestr",
            Convert::Utf8ToUtf16(messagestr));
	}

	// Create a packet //
	shared_ptr<NetworkRequest> sendrequest(new NetworkRequest(new RequestCommandExecutionData(messagestr)));

	// Send it //
	auto sendthing = ServerConnection->SendPacketToConnection(sendrequest, 10);


	// The packet may not fail so we need to monitor the response //
	OurSentRequests.push_back(sendthing);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkClientInterface::IsConnected() const{
	return ConnectedToServer;
}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_OnStartHeartbeats(){
	GUARD_LOCK_THIS_OBJECT();

	// Ignore if already started /
	if(UsingHeartbeats)
		return;

	// Reset heartbeat variables //
	LastReceivedHeartbeat = Misc::GetThreadSafeSteadyTimePoint();
	LastSentHeartbeat = LastReceivedHeartbeat;

	SecondsWithoutConnection = 0.f;

	// And start using them //
	UsingHeartbeats = true;
}

void Leviathan::NetworkClientInterface::_OnHeartbeat(){
	GUARD_LOCK_THIS_OBJECT();
	// Reset the times //
	LastReceivedHeartbeat = Misc::GetThreadSafeSteadyTimePoint();
	SecondsWithoutConnection = 0.f;
}

void Leviathan::NetworkClientInterface::_UpdateHeartbeats(){
	// Skip if not in use //
	if(!UsingHeartbeats)
		return;

	GUARD_LOCK_THIS_OBJECT();

	if(!ServerConnection)
		return;

	// Check do we need to send one //
	auto timenow = Misc::GetThreadSafeSteadyTimePoint();

	if(timenow >= LastSentHeartbeat+MillisecondDuration(CLIENT_HEARTBEATS_MILLISECOND)){

		// Send one //
		shared_ptr<NetworkResponse> response(new NetworkResponse(-1, PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 30));
		response->GenerateHeartbeatResponse();

		ServerConnection->SendPacketToConnection(response, 1);

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
DLLEXPORT bool Leviathan::NetworkClientInterface::RegisterNetworkedInput(shared_ptr<NetworkedInputHandler> handler){
	GUARD_LOCK_THIS_OBJECT();
    
	PotentialInputHandler = handler;
	return true;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::NetworkClientInterface::GetOurID() const{
	return OurPlayerID;
}

DLLEXPORT NetworkedInputHandler* Leviathan::NetworkClientInterface::GetNetworkedInput(){
	return PotentialInputHandler.get();
}

DLLEXPORT shared_ptr<ConnectionInfo> Leviathan::NetworkClientInterface::GetServerConnection(){
	return ServerConnection;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::_OnDisconnectFromServer(const wstring &reasonstring, bool donebyus){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnStartConnectToServer(){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnFailedToConnectToServer(const wstring &reason){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnSuccessfullyConnectedToServer(){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnNewConnectionStatusMessage(const wstring &message){

}



