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
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkClientInterface::NetworkClientInterface() : MaxConnectTries(DEFAULT_MAXCONNECT_TRIES), ConnectTriesCount(0){

}

DLLEXPORT Leviathan::NetworkClientInterface::~NetworkClientInterface(){

}
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

DLLEXPORT void Leviathan::NetworkClientInterface::DisconnectFromServer(ObjectLock &guard, const wstring &reason){
	VerifyLock(guard);

	// Return if no connection //
	if(!ServerConnection){

		Logger::Get()->Info(L"NetworkClientInterface: DisconnectFromServer: not connected to any servers");
		return;
	}

	// Send disconnect message to server //
	_OnNewConnectionStatusMessage(L"Disconnected from "+ServerConnection->GenerateFormatedAddressString()+L", reason: "+reason);

	// Close connection //
	NetworkHandler::Get()->SafelyCloseConnectionTo(ServerConnection.get());
	ServerConnection.reset();

	_OnDisconnectFromServer(reason);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkClientInterface::_HandleClientRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connectiontosendresult){
	DEBUG_BREAK;
	return false;
}

DLLEXPORT bool Leviathan::NetworkClientInterface::_HandleClientResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection, bool &dontmarkasreceived){
	DEBUG_BREAK;
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

				// Do some checks based on request type //
				switch(tmpsendthing->OriginalRequest->GetType()){
				case NETWORKREQUESTTYPE_JOINSERVER:
					{
						if(ConnectTriesCount < MaxConnectTries){

							Logger::Get()->Write(L"\t> Retrying connect");
							_SendConnectRequest(guard);

						} else {

							Logger::Get()->Write(L"\t> Maximum connect tries reached");
							DisconnectFromServer(guard, L"Connection timed out after "+Convert::ToWstring(ConnectTriesCount)+L" tries");
						}
					}
					break;
				default:
					Logger::Get()->Write(L"\t> Unknown request type, probably not important, dropping request");
				}

				// We need to loop again, because our iterator is now invalid //
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
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::_OnNotifierDisconnected(BaseNotifiableAll* parenttoremove){
	DEBUG_BREAK;
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
	_OnNewConnectionStatusMessage(L"Trying to connect to server on "+ServerConnection->GenerateFormatedAddressString()+L", attempt "+
		Convert::ToWstring(ConnectTriesCount));
}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_ProcessCompletedRequest(shared_ptr<SentNetworkThing> tmpsendthing, ObjectLock &guard){
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
					// \todo check what was the actual accepted thing
					_ProperlyConnectedToServer(guard);
				}
				break;
			}
		}
		break;
	default:
		Logger::Get()->Info(L"NetworkClientInterface: WE MADE AN INVALID REQUEST");
		DEBUG_BREAK;
	}
}
// ------------------------------------ //
void Leviathan::NetworkClientInterface::_ProperlyConnectedToServer(ObjectLock &guard){
	VerifyLock(guard);

	// Set the variables //
	ConnectedToServer = true;
	
	// Send connect message //
	_OnNewConnectionStatusMessage(L"Established a connection with "+ServerConnection->GenerateFormatedAddressString());

	// Call the callback //
	_OnProperlyConnected();
}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnProperlyConnected(){
	// By default synchronize values and then pass to the pure virtual function for joining the match properly //
	SyncedVariables::Get()->AddAnotherToSyncWith(ServerConnection.get());

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
		if(!maderequest->GetFutureForThis().get()){
			// Terminate the connection //
			DEBUG_BREAK;
			return;
		}
		
		// We are now syncing the variables //
		iptr->_OnNewConnectionStatusMessage(L"Syncing variables, TODO: how many ");

		// Queue task that checks when it is done //
		Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new ConditionalTask(
			boost::bind<void>([](NetworkClientInterface* iptr) -> void
		{
			// Set the status to almost done //
			iptr->_OnNewConnectionStatusMessage(L"Finalizing connection");

			// We are now completely connected from the engines point of view so let the application know //
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
DLLEXPORT void Leviathan::NetworkClientInterface::_OnDisconnectFromServer(const wstring &reasonstring){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnStartConnectToServer(){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnFailedToConnectToServer(const wstring &reason){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnSuccessfullyConnectedToServer(){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnNewConnectionStatusMessage(const wstring &message){

}




