#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKEDINPUTHANDLER
#include "NetworkedInputHandler.h"
#endif
#include "NetworkedInput.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkedInputHandler::NetworkedInputHandler(NetworkInputFactory* objectcreater, NetworkClientInterface* isclient) : 
	LastInputSourceID(2500), IsOnTheServer(false), ClientInterface(isclient), ServerInterface(NULL)
{

}

DLLEXPORT Leviathan::NetworkedInputHandler::NetworkedInputHandler(NetworkInputFactory* objectcreater, NetworkServerInterface* isserver) : 
	LastInputSourceID(2500), IsOnTheServer(true), ClientInterface(NULL), ServerInterface(isserver)
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
			DEBUG_BREAK;

			return true;
		}

	}

	// Type didn't match anything that we should be concerned with //
	return false;
}

DLLEXPORT bool Leviathan::NetworkedInputHandler::HandleInputPacket(shared_ptr<NetworkResponse> response, ConnectionInfo* connection){
	GUARD_LOCK_THIS_OBJECT();


	// Type didn't match anything that we should be concerned with //
	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInputHandler::UpdateInputStatus(){
	GUARD_LOCK_THIS_OBJECT();

	// Remove invalid things //
	auto end = GlobalOrLocalListeners.end();
	for(auto iter = GlobalOrLocalListeners.begin(); iter != end; ++iter){

		// This checks for all invalid states //
		if(NETWORKEDINPUT_STATE_TODISCARD & (*iter)->GetState())
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

