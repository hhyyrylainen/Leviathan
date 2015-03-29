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
DLLEXPORT Leviathan::NetworkedInputHandler::NetworkedInputHandler(NetworkInputFactory* objectcreater,
    NetworkClientInterface* isclient) : 
	LastInputSourceID(2500), IsOnTheServer(false), ClientInterface(isclient), ServerInterface(NULL),
    _NetworkInputFactory(objectcreater)
{
    Staticinstance = this;
}

DLLEXPORT Leviathan::NetworkedInputHandler::NetworkedInputHandler(NetworkInputFactory* objectcreater,
    NetworkServerInterface* isserver) : 
	LastInputSourceID(2500), IsOnTheServer(true), ClientInterface(NULL), ServerInterface(isserver),
    _NetworkInputFactory(objectcreater)
{
    Staticinstance = this;
}

DLLEXPORT Leviathan::NetworkedInputHandler::~NetworkedInputHandler(){
    
    Staticinstance = NULL;
}

NetworkedInputHandler* Leviathan::NetworkedInputHandler::Staticinstance = NULL;

DLLEXPORT NetworkedInputHandler* Leviathan::NetworkedInputHandler::Get(){
    
    return Staticinstance;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInputHandler::Release(){
    
	GUARD_LOCK_THIS_OBJECT();

	auto end = GlobalOrLocalListeners.end();
	for(auto iter = GlobalOrLocalListeners.begin(); iter != end; ++iter){

        // Apparently there can be null pointers in the vector, skip them //
        NetworkedInput* ptr = (*iter).get();

        if(ptr)
            _NetworkInputFactory->NoLongerNeeded(*ptr);
	}


	GlobalOrLocalListeners.clear();

    // The listeners might want to destruct stuff, so set this to NULL after releasing them //
	_NetworkInputFactory = NULL;    
    
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkedInputHandler::HandleInputPacket(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connection)
{
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

DLLEXPORT bool Leviathan::NetworkedInputHandler::HandleInputPacket(shared_ptr<NetworkResponse> response,
    ConnectionInfo* connection)
{
	GUARD_LOCK_THIS_OBJECT();


	switch(response->GetType()){
        case NETWORKRESPONSETYPE_CREATENETWORKEDINPUT:
		{
			// Server in turn ignores this one //
			if(IsOnTheServer) 
				return false;
#ifndef NETWORK_USE_SNAPSHOTS
            if(!_HandleInputCreateResponse(response, connection)){

                Logger::Get()->Error("NetworkedInputHandler: failed to create replicated input on a client");
                return true;
            }
#endif //NETWORK_USE_SNAPSHOTS
            
			return true;
		}
		break;
        case NETWORKRESPONSETYPE_UPDATENETWORKEDINPUT:
		{
			// Process it //
			if(!_HandleInputUpdateResponse(response, connection)){

				// This packet wasn't properly authenticated //
                Logger::Get()->Warning("NetworkedInputHandler: improperly authenticated response");
				return true;
			}

#ifdef NETWORK_USE_SNAPSHOTS
			// Everybody receives these, but only the server has to distribute these around //
            if(IsOnTheServer){

                // Distribute it around //
        
                // We duplicate the packet from the data to make it harder for people to send invalid packets to
                // other clients 
                shared_ptr<NetworkResponse> tmprespall = shared_ptr<NetworkResponse>(new NetworkResponse(-1,
                        PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 5));

                tmprespall->GenerateUpdateNetworkedInputResponse(new NetworkResponseDataForUpdateNetworkedInput(
                        *response->GetResponseDataForUpdateNetworkedInputResponse()));

                ServerInterface->SendToAllButOnePlayer(response, connection);
            }
#endif //NETWORK_USE_SNAPSHOTS

			return true;
		}
		break;


	}


	// Type didn't match anything that we should be concerned with //
	return false;
}
// ------------------------------------ //
void Leviathan::NetworkedInputHandler::_HandleConnectRequestPacket(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connection)
{
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


		tmpresp->GenerateServerAllowResponse(new NetworkResponseDataForServerAllow(
                NETWORKRESPONSE_SERVERACCEPTED_TYPE_CONNECT_ACCEPTED));

		connection->SendPacketToConnection(tmpresp, 4);


		// Send messages to other clients //
#ifndef NETWORK_USE_SNAPSHOTS
		// First create the packet //
		shared_ptr<NetworkResponse> tmprespall = shared_ptr<NetworkResponse>(new NetworkResponse(-1,
                PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 15));

		tmprespall->GenerateCreateNetworkedInputResponse(new NetworkResponseDataForCreateNetworkedInput(
                *GlobalOrLocalListeners.back()));

		// Using threading here to not use too much time processing the create request //

		// \todo Guarantee that the interface will be available when this is ran
		ThreadingManager::Get()->QueueTask(new QueuedTask(boost::bind<void>(
			[](shared_ptr<NetworkResponse> response, NetworkServerInterface* server, ConnectionInfo* skipme) -> void
		{

			// Then tell the interface to send it to all but one connection //
			server->SendToAllButOnePlayer(response, skipme);

			Logger::Get()->Info(L"NetworkedInputHandler: finished distributing create response around");

		}, tmprespall, ServerInterface, connection)));
#endif //NETWORK_USE_SNAPSHOTS



		return;
	}

notallowedfailedlabel:

	// Notify about we disallowing this connection //
	shared_ptr<NetworkResponse> tmpresp(new NetworkResponse(NETWORKRESPONSETYPE_SERVERDISALLOW, 
		PACKAGE_TIMEOUT_STYLE_TIMEDMS, 1));

	tmpresp->GenerateServerDisallowResponse(new 
		NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON_NOT_AUTHORIZED,
            L"Not allowed to create input with that ID"));

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
		if(tmpstate == NETWORKEDINPUT_STATE_CLOSED || tmpstate == NETWORKEDINPUT_STATE_FAILED){

			DeleteQueue.push_back(*iter);
			iter = GlobalOrLocalListeners.erase(iter);
		}
	}


	_HandleDeleteQueue(guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInputHandler::GetNextInputIDNumber(boost::function<void (int)> onsuccess,
    boost::function<void ()> onfailure)
{
	GUARD_LOCK_THIS_OBJECT();

	DEBUG_BREAK;
}

DLLEXPORT int Leviathan::NetworkedInputHandler::GetNextInputIDNumberOnServer(){
	GUARD_LOCK_THIS_OBJECT();

	assert(IsOnTheServer && "cannot call this function on the client");


	return ++LastInputSourceID;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkedInputHandler::RegisterNewLocalGlobalReflectingInputSource(shared_ptr<NetworkedInput>
    iobject)
{

	assert(IsOnTheServer != true && "cannot call this function on the server");

	GUARD_LOCK_THIS_OBJECT();


	// Start connection to the server //
	iobject->NowOwnedBy(this);

	iobject->InitializeLocal();

	if(!iobject->ConnectToServersideInput()){

		Logger::Get()->Error("NetworkedInputHandler: register local input source failed because connecting it to "
            "the server didn't start properly");
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
                    // Only unallocate if the factory is still around //
                    if(_NetworkInputFactory && (*iter).get())
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
	GUARD_LOCK_THIS_OBJECT();

	// Find and remove from the main list //
	auto end = GlobalOrLocalListeners.end();
	for(auto iter = GlobalOrLocalListeners.begin(); iter != end; ++iter){

		if((*iter).get() == inputobj){

			// Found the one //
			DeleteQueue.push_back(*iter);
			GlobalOrLocalListeners.erase(iter);
			return;
		}
	}

	// Not found //
}

void Leviathan::NetworkedInputHandler::_HandleDeleteQueue(ObjectLock &guard){
	VerifyLock(guard);

	auto end = DeleteQueue.begin();
	for(auto iter = DeleteQueue.begin(); iter != end; ++iter){

		(*iter)->TerminateConnection();
		_NetworkInputFactory->NoLongerNeeded(*(*iter).get());
	}

	// All are now ready to be deleted //
	DeleteQueue.clear();
}
// ------------------------------------ //
bool Leviathan::NetworkedInputHandler::_HandleInputUpdateResponse(shared_ptr<NetworkResponse> response,
    ConnectionInfo* connection)
{


	NetworkResponseDataForUpdateNetworkedInput* data = response->GetResponseDataForUpdateNetworkedInputResponse();

    if(!data)
        return false;


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
	if(!target){
        
        Logger::Get()->Warning("NetworkedInputHandler: couldn't find a target for update response, InputID: "+
            Convert::ToString(data->InputID));
		return false;
    }


	// Check is it allowed //
    if(IsOnTheServer){
        
        if(!_NetworkInputFactory->IsConnectionAllowedToUpdate(target, connection)){

            // Some player is trying to fake someone else's input //
            Logger::Get()->Warning("NetworkedInputHandler: connection not allowed to update InputID: "+
                Convert::ToString(data->InputID));
            return false;
        }
        
    } else {

        if(!connection)
            return false;
    }

	
	// Now we can update it //
	target->LoadUpdatesFromPacket(data->UpdateData);

	return true;
}
// ------------------------------------ //
bool Leviathan::NetworkedInputHandler::_HandleInputCreateResponse(shared_ptr<NetworkResponse> response,
    ConnectionInfo* connection)
{
    NetworkResponseDataForCreateNetworkedInput* data = response->GetResponseDataForCreateNetworkedInputResponse();

    if(!data)
        return false;


    // We need to partially load the data here //
    int ownerid, inputid;

    NetworkedInput::LoadHeaderDataFromPacket(data->DataForObject, ownerid, inputid);

    Logger::Get()->Info("NetworkedInputHandler: client replicating networked input, "+Convert::ToString(ownerid));

    // Create a temporary object from the packet //
    auto ournewobject = _NetworkInputFactory->CreateNewInstanceForReplication(inputid, ownerid);

    if(!ournewobject)
        return false;

    // Check is it allowed //
    // Check here is the connection the connection to the server //
    bool allowed = connection ? true: false;

    if(!allowed)
        return false;

    GUARD_LOCK_THIS_OBJECT();

    assert(!IsOnTheServer && "Don't call _HandleInputCreateResponse on the server");
    
    // It got accepted so finish adding the data //
    ournewobject->OnLoadCustomFullDataFrompacket(data->DataForObject);
    
    // Add it to us //
    LinkReceiver(ournewobject.get());
    ournewobject->NowOwnedBy(this);
    ournewobject->SetNetworkReceivedState();


    GlobalOrLocalListeners.push_back(shared_ptr<NetworkedInput>(ournewobject.release()));

    _NetworkInputFactory->ReplicationFinalized(GlobalOrLocalListeners.back().get());

	return true;
}
