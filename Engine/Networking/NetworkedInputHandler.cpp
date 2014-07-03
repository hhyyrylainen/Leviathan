#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKEDINPUTHANDLER
#include "NetworkedInputHandler.h"
#endif
#include "NetworkedInput.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
#include "ConnectionInfo.h"
#include "Threading/ThreadingManager.h"
#include "NetworkServerInterface.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkedInputHandler::NetworkedInputHandler(NetworkInputFactory* objectcreater, NetworkClientInterface* isclient) : 
	LastInputSourceID(2500), IsOnTheServer(false), ClientInterface(isclient), ServerInterface(NULL), _NetworkInputFactory(objectcreater)
{

}

DLLEXPORT Leviathan::NetworkedInputHandler::NetworkedInputHandler(NetworkInputFactory* objectcreater, NetworkServerInterface* isserver) : 
	LastInputSourceID(2500), IsOnTheServer(true), ClientInterface(NULL), ServerInterface(isserver), _NetworkInputFactory(objectcreater)
{

}

DLLEXPORT Leviathan::NetworkedInputHandler::~NetworkedInputHandler(){
	GUARD_LOCK_THIS_OBJECT();

	auto end = GlobalOrLocalListeners.end();
	for(auto iter = GlobalOrLocalListeners.begin(); iter != end; ++iter){

		_NetworkInputFactory->NoLongerNeeded(*(*iter).get());
	}


	_NetworkInputFactory = NULL;
	GlobalOrLocalListeners.clear();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkedInputHandler::HandleInputPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	GUARD_LOCK_THIS_OBJECT();

	switch(request->GetType()){
	case NETWORKREQUESTTYPE_CONNECTINPUT:
		{
			// Clients won't receive these //
			if(!IsOnTheServer) 
				return false;

			_HandleConnectRequestPacket(request, connection);
			return true;
		}
	}

	// Type didn't match anything that we should be concerned with //
	return false;
}

DLLEXPORT bool Leviathan::NetworkedInputHandler::HandleInputPacket(shared_ptr<NetworkResponse> response, ConnectionInfo* connection){
	GUARD_LOCK_THIS_OBJECT();


	switch(response->GetType()){
	case NETWORKRESPONSETYPE_CREATENETWORKEDINPUT:
		{
			// Server in turn ignores this one //
			if(IsOnTheServer) 
				return false;

			DEBUG_BREAK;

			return true;
		}
		break;
	case NETWORKRESPONSETYPE_UPDATENETWORKEDINPUT:
		{
			// Process it //
			if(!_HandleInputUpdateResponse(response, connection)){

				// This packet wasn't properly authenticated //
				return true;
			}


			// Everybody receives these, but only the server has to distribute these around //
			if(IsOnTheServer){

				// Distribute it around //
				ServerInterface->SendToAllButOnePlayer(response, connection);
			}

			return true;
		}
		break;


	}


	// Type didn't match anything that we should be concerned with //
	return false;
}
// ------------------------------------ //
void Leviathan::NetworkedInputHandler::_HandleConnectRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	{
		GUARD_LOCK_THIS_OBJECT();

		// Verify that it is allowed //
		bool allowed = true;

		// If the packet is valid it should contain this //
		RequestConnectInputData* data = request->GetConnectInputRequestData();

		if(!data)
			goto notallowedfailedlabel;




		// We need to partially load the data here //
		int ownerid, inputid;

		NetworkedInput::LoadHeaderDataFromPacket(data->DataForObject, ownerid, inputid);


		// Create a temporary object from the packet //
		auto ournewobject = _NetworkInputFactory->CreateNewInstanceForReplication(inputid, ownerid);

		if(!ournewobject)
			goto notallowedfailedlabel;

		// Check is it allowed //
		allowed = _NetworkInputFactory->DoesServerAllowCreate(ournewobject.get(), connection);


		if(!allowed)
			goto notallowedfailedlabel;


		// It got accepted so finish adding the data //
		ournewobject->OnLoadCustomFullDataFrompacket(data->DataForObject);

		// Add it to us //
		LinkReceiver(ournewobject.get());
		ournewobject->NowOwnedBy(this);
		ournewobject->SetNetworkReceivedState();


		GlobalOrLocalListeners.push_back(shared_ptr<NetworkedInput>(ournewobject.release()));

		_NetworkInputFactory->ReplicationFinalized(GlobalOrLocalListeners.back().get());

		// Notify that we accepted it //
		shared_ptr<NetworkResponse> tmpresp(new NetworkResponse(request->GetExpectedResponseID(), 
			PACKAGE_TIMEOUT_STYLE_TIMEDMS, 1500));


		tmpresp->GenerateServerAllowResponse(new NetworkResponseDataForServerAllow(NETWORKRESPONSE_SERVERACCEPTED_TYPE_CONNECT_ACCEPTED));

		connection->SendPacketToConnection(tmpresp, 4);


		// Send messages to other clients //
		
		// First create the packet //
		shared_ptr<NetworkResponse> tmprespall = shared_ptr<NetworkResponse>(new NetworkResponse(-1, PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 15));

		tmprespall->GenerateCreateNetworkedInputResponse(new NetworkResponseDataForCreateNetworkedInput(*GlobalOrLocalListeners.back()));

		// Using threading here to not use too much time processing the create request //

		// \todo Guarantee that the interface will be available when this is ran
		ThreadingManager::Get()->QueueTask(new QueuedTask(boost::bind<void>(
			[](shared_ptr<NetworkResponse> response, NetworkServerInterface* server, ConnectionInfo* skipme) -> void
		{

			// Then tell the interface to send it to all but one connection //
			server->SendToAllButOnePlayer(response, skipme);

			Logger::Get()->Info(L"NetworkedInputHandler: finished distributing create response around");

		}, tmprespall, ServerInterface, connection)));




		return;
	}

notallowedfailedlabel:

	// Notify about we disallowing this connection //
	shared_ptr<NetworkResponse> tmpresp(new NetworkResponse(NETWORKRESPONSETYPE_SERVERDISALLOW, 
		PACKAGE_TIMEOUT_STYLE_TIMEDMS, 1));

	tmpresp->GenerateServerDisallowResponse(new 
		NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON_NOT_AUTHORIZED, L"Not allowed to create input with that ID"));

	connection->SendPacketToConnection(tmpresp, 1);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInputHandler::UpdateInputStatus(){
	GUARD_LOCK_THIS_OBJECT();

	// Remove invalid things //
	auto end = GlobalOrLocalListeners.end();
	for(auto iter = GlobalOrLocalListeners.begin(); iter != end; ++iter){

		// This checks for all invalid states //
		auto tmpstate = (*iter)->GetState();
		if(tmpstate == NETWORKEDINPUT_STATE_CLOSED || tmpstate == NETWORKEDINPUT_STATE_FAILED)
			_NetworkInputFactory->NoLongerNeeded(*(*iter).get());
	}



}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInputHandler::GetNextInputIDNumber(boost::function<void (int)> onsuccess, boost::function<void ()> onfailure){
	GUARD_LOCK_THIS_OBJECT();

	DEBUG_BREAK;
}

DLLEXPORT int Leviathan::NetworkedInputHandler::GetNextInputIDNumberOnServer(){
	GUARD_LOCK_THIS_OBJECT();

	assert(IsOnTheServer && "cannot call this function on the client");


	return ++LastInputSourceID;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkedInputHandler::RegisterNewLocalGlobalReflectingInputSource(shared_ptr<NetworkedInput> iobject){

	assert(IsOnTheServer != true && "cannot call this function on the server");

	GUARD_LOCK_THIS_OBJECT();


	// Start connection to the server //
	iobject->NowOwnedBy(this);

	iobject->InitializeLocal();

	if(!iobject->ConnectToServersideInput()){

		Logger::Get()->Error(L"NetworkedInputHandler: register local input source failed because connecting it to the server didn't start properly");
		return false;
	}

	// Store our local copy //
	GlobalOrLocalListeners.push_back(iobject);

	LinkReceiver(iobject.get());

	return true;
}
// ------------------------------------ //
void Leviathan::NetworkedInputHandler::_OnChildUnlink(InputReceiver* child){
	GUARD_LOCK_THIS_OBJECT();

	for(size_t i = 0; i < ConnectedReceivers.size(); i++){
		if(ConnectedReceivers[i] == child){
			// call disconnect function first //
			ConnectedReceivers[i]->_OnDisconnect(this);
			ConnectedReceivers.erase(ConnectedReceivers.begin()+i);

			// Remove from the other place, too //
			auto end = GlobalOrLocalListeners.end();
			for(auto iter = GlobalOrLocalListeners.begin(); iter != end; ++iter){

				// Discard the one matching the pointer //
				if(child == static_cast<InputReceiver*>((*iter).get())){
					_NetworkInputFactory->NoLongerNeeded(*(*iter).get());
					break;
				}
			}
			return;
		}
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInputHandler::QueueDeleteInput(NetworkedInput* inputobj){
	DEBUG_BREAK;
}
// ------------------------------------ //
bool Leviathan::NetworkedInputHandler::_HandleInputUpdateResponse(shared_ptr<NetworkResponse> response, ConnectionInfo* connection){


	NetworkResponseDataForUpdateNetworkedInput* data = response->GetResponseDataForUpdateNetworkedInputResponse();


	GUARD_LOCK_THIS_OBJECT();

	// Find the right input object //

	NetworkedInput* target = NULL;

	auto end = GlobalOrLocalListeners.end();
	for(auto iter = GlobalOrLocalListeners.begin(); iter != end; ++iter){

		// Find the right one //
		if((*iter)->GetID() == data->InputID){

			// Found the target //
			target = (*iter).get();
			break;
		}
	}


	// If we didn't find it this *should* be a bogus request //
	if(!target)
		return false;


	// Check is it allowed //
	if(!_NetworkInputFactory->IsConnectionAllowedToUpdate(target, connection)){

		// Some player is trying to fake someone else's input //
		return false;
	}

	
	// Now we can update it //
	target->LoadUpdatesFromPacket(data->UpdateData);

	return true;
}

