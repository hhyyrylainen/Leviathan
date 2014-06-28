#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKEDINPUT
#include "NetworkedInput.h"
#endif
#include "Exceptions/ExceptionInvalidArgument.h"
#include "NetworkedInputHandler.h"
#include "NetworkRequest.h"
#include "Threading/ThreadingManager.h"
#include "ConnectionInfo.h"
#include "NetworkClientInterface.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkedInput::NetworkedInput(int ownerid, int networkid) : OwnerID(ownerid), InputID(networkid), OwningHandler(NULL),
	CurrentState(NETWOKREDINPUT_STATE_READY)
{

}

DLLEXPORT Leviathan::NetworkedInput::NetworkedInput(sf::Packet &packet){
	LoadDataFromFullPacket(packet);
}

DLLEXPORT Leviathan::NetworkedInput::~NetworkedInput(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::AddFullDataToPacket(sf::Packet &packet){
	// First our common information //
	packet << OwnerID << InputID;

	// And then the custom data //
	OnAddFullCustomDataToPacket(packet);
}

DLLEXPORT void Leviathan::NetworkedInput::LoadDataFromFullPacket(sf::Packet &packet){

	// First our common information //
	if(!(packet >> OwnerID)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"packet", L"");
	}
	if(!(packet >> InputID)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"packet", L"");
	}

	// And then the custom data //
	OnLoadCustomFullDataFrompacket(packet);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::AddChangesToPacket(sf::Packet &packet){

	// Common data cannot change, only custom data has changed //
	OnAddUpdateCustomDataToPacket(packet);

}

DLLEXPORT void Leviathan::NetworkedInput::LoadUpdatesFromPacket(sf::Packet &packet){

	// Common data cannot change, only custom data has changed //
	OnLoadCustomUpdateDataFrompacket(packet);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::OnUpdateInputStates(){
	// Send updates through the network //
	DEBUG_BREAK;

	// Notify local update //
	_OnInputChanged();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkedInput::ConnectToServersideInput(){

	assert(!OwningHandler->IsOnTheServer && "Cannot connect to the server on the server process");

	// Send a connect request //
	CurrentState = NETWORKEDINPUT_STATE_WAITING;

	sf::Packet tmpcollect;

	// Add our data in it //
	AddFullDataToPacket(tmpcollect);

	// Create the request //
	shared_ptr<NetworkRequest> request(new NetworkRequest(new RequestConnectInputData(tmpcollect), 1500));


	// Send the request //
	auto connection = OwningHandler->ClientInterface->GetServerConnection();

	if(!connection)
		return false;

	shared_ptr<SentNetworkThing> netthing = connection->SendPacketToConnection(request, 12);

	// Queue a task for checking the result later //
	ThreadingManager::Get()->QueueTask(new ConditionalDelayedTask(boost::bind<void>(
		[](shared_ptr<SentNetworkThing> requestthing, NetworkedInput* inputobj) -> void
	{
		if(!requestthing->GetFutureForThis().get() || !requestthing->GotResponse){

			// Destroy it //
			Logger::Get()->Warning(L"NetworkedInput: closed due to the server not responding to connect request");
			goto doactualdeletereleasethingforfaillabel;
		}

		// Check the response //
		if(requestthing->GotResponse->GetType() != NETWORKRESPONSETYPE_SERVERALLOW){

			// It failed because server denied it //
			Logger::Get()->Warning(L"NetworkedInput: closed due to the server denying our connect request");
			goto doactualdeletereleasethingforfaillabel;
		}

		// It succeeded so no further action is required //
		return;

doactualdeletereleasethingforfaillabel:

		auto tmp = dynamic_cast<NetworkClientInterface*>(NetworkHandler::GetInterface());

		if(tmp){
			auto otherpotential = tmp->GetNetworkedInput();
			if(otherpotential)
				otherpotential->QueueDeleteInput(inputobj);
		}

	}, netthing, this), boost::bind<bool>(
		[](shared_ptr<SentNetworkThing> requestthing, NetworkedInput* inputobj) -> bool{
		// Check has it arrived //
		if(requestthing->GetFutureForThis().has_value())
			return true;

		// More waiting //
		return false;
	}, netthing, this), MillisecondDuration(10)));

	


	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::NowOwnedBy(NetworkedInputHandler* owner){
	OwningHandler = owner;
}

DLLEXPORT void Leviathan::NetworkedInput::OnParentNoLongerAvailable(){
	OwningHandler = NULL;
}
// ------------------------------------ //
DLLEXPORT NETWORKEDINPUT_STATE Leviathan::NetworkedInput::GetState() const{
	return CurrentState;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::_OnRemovedConnection(){

}

void Leviathan::NetworkedInput::_OnInputChanged(){

}
// ------------------------------------ //
void Leviathan::NetworkedInput::OnLoadCustomFullDataFrompacket(sf::Packet &packet){

}

void Leviathan::NetworkedInput::OnLoadCustomUpdateDataFrompacket(sf::Packet &packet){

}
// ------------------------------------ //
void Leviathan::NetworkedInput::OnAddFullCustomDataToPacket(sf::Packet &packet){

}

void Leviathan::NetworkedInput::OnAddUpdateCustomDataToPacket(sf::Packet &packet){

}
