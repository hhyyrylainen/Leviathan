// ------------------------------------ //
#include "NetworkedInput.h"

#include "Exceptions.h"
#include "NetworkedInputHandler.h"
#include "NetworkRequest.h"
#include "Threading/ThreadingManager.h"
#include "ConnectionInfo.h"
#include "NetworkClientInterface.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkedInput::NetworkedInput(int ownerid, int networkid) :
    OwnerID(ownerid), InputID(networkid), OwningHandler(NULL), CurrentState(NETWOKREDINPUT_STATE_READY)
{

}

DLLEXPORT Leviathan::NetworkedInput::NetworkedInput(sf::Packet &packet){
	LoadDataFromFullPacket(packet);
}

DLLEXPORT Leviathan::NetworkedInput::~NetworkedInput(){

    // Set the correct state for destructors that run after this //
    CurrentState = NETWORKEDINPUT_STATE_DESTRUCTED;
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

		throw InvalidArgument("invalid packet format");
	}
	if(!(packet >> InputID)){

		throw InvalidArgument("invalid packet format");
	}

	// And then the custom data //
	OnLoadCustomFullDataFrompacket(packet);
}

DLLEXPORT void Leviathan::NetworkedInput::LoadHeaderDataFromPacket(sf::Packet &packet, int &ownerid, int &inputid)
    
{
	// First our common information //
	if(!(packet >> ownerid)){

		throw InvalidArgument("invalid packet format");
	}
    
	if(!(packet >> inputid)){

		throw InvalidArgument("invalid packet format");
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::AddChangesToPacket(sf::Packet &packet){

	// Common data cannot change, only custom data has changed //
	OnAddUpdateCustomDataToPacket(packet);

}

DLLEXPORT void Leviathan::NetworkedInput::LoadUpdatesFromPacket(sf::Packet &packet){

	// Common data cannot change, only custom data has changed //
	OnLoadCustomUpdateDataFrompacket(packet);

	// Notify local update //
	_OnInputChanged();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::OnUpdateInputStates(){
	// Server may not do this //
	if(OwningHandler->IsOnTheServer)
		return;

	// Send updates through the network //
	shared_ptr<NetworkResponse> response(new NetworkResponse(-1, PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 5));
	response->GenerateUpdateNetworkedInputResponse(new NetworkResponseDataForUpdateNetworkedInput(*this));

	// Send to the server for it to then distribute it around //
	OwningHandler->ClientInterface->GetServerConnection()->SendPacketToConnection(response, 5);


	// Notify local update //
	_OnInputChanged();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkedInput::ConnectToServersideInput(){

	assert(!OwningHandler->IsOnTheServer && "Cannot connect to the server on the server process");

	// Send a connect request //
	CurrentState = NETWORKEDINPUT_STATE_WAITING;

	// Create the request //
	shared_ptr<NetworkRequest> request(new NetworkRequest(new RequestConnectInputData(*this), 1500));


	// Send the request //
	auto connection = OwningHandler->ClientInterface->GetServerConnection();

	if(!connection)
		return false;

	shared_ptr<SentNetworkThing> netthing = connection->SendPacketToConnection(request, 12);

	// Queue a task for checking the result later //
	ThreadingManager::Get()->QueueTask(new ConditionalDelayedTask(std::bind<void>(
		[](shared_ptr<SentNetworkThing> requestthing, NetworkedInput* inputobj) -> void
	{
		if(!requestthing->GetStatus() || !requestthing->GotResponse){

			// Destroy it //
			Logger::Get()->Warning("NetworkedInput: closed due to the server not responding to connect request");
			goto doactualdeletereleasethingforfaillabel;
		}

		// Check the response //
		if(requestthing->GotResponse->GetType() != NETWORKRESPONSETYPE_SERVERALLOW){

			// It failed because server denied it //
			Logger::Get()->Warning("NetworkedInput: closed due to the server denying our connect request");
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

	}, netthing, this), std::bind<bool>(
		[](shared_ptr<SentNetworkThing> requestthing, NetworkedInput* inputobj) -> bool{
		// Check has it arrived //
		if(requestthing->IsFinalized())
			return true;

		// More waiting //
		return false;
	}, netthing, this), MillisecondDuration(10)));

	


	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::TerminateConnection(){

    // We don't need to bother if the server hasn't been contacted //
    if(CurrentState == NETWOKREDINPUT_STATE_READY)
        return;
    
	auto response = std::make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000);

    response->GenerateDisconnectInputResponse(
        new NetworkResponseDataForDisconnectInput(InputID, OwnerID));
    
	// Send the request //
	auto connection = OwningHandler->ClientInterface->GetServerConnection();

	if(!connection)
		return;

	connection->SendPacketToConnection(response, 12);
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

DLLEXPORT int Leviathan::NetworkedInput::GetID() const{
	return InputID;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkedInput::SetNetworkReceivedState(){
	CurrentState = NETWORKEDINPUT_STATE_CONNECTED;
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

