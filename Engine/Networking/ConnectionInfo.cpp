#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CONNECTIONINFO
#include "ConnectionInfo.h"
#endif
#include "Utility/Iterators/WstringIterator.h"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "NetworkHandler.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ConnectionInfo::ConnectionInfo(shared_ptr<wstring> hostname, USHORT port /*= sf::Socket::AnyPort*/) : PortNumber(port), HostName(hostname){
	// We need to split the port number from the address //
	WstringIterator itr(hostname.get(), false);

	auto result = itr.GetUntilNextCharacterOrAll(L':');

	HostName = shared_ptr<wstring>(result.release());

	// We should be fine skipping a letter //
	result = itr.GetUntilEnd();

	TargetPortNumber = Convert::WstringToInt(*result.get());
}

DLLEXPORT Leviathan::ConnectionInfo::~ConnectionInfo(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConnectionInfo::Init(){
	// This might do something //
	TargetHost = sf::IpAddress(Convert::WstringToString(*HostName.get()));
	
	// We want to receive responses //
	if(_Socket.bind(PortNumber) != sf::Socket::Done){

		return false;
	}

	// Set the socket as non-blocking //
	_Socket.setBlocking(false);

	// Register us //
	NetworkHandler::Get()->_RegisterConnectionInfo(this);

	return true;
}

DLLEXPORT void Leviathan::ConnectionInfo::ReleaseSocket(){
	// Remove us from the queue //
	NetworkHandler::Get()->_UnregisterConnectionInfo(this);

	_Socket.unbind();
}
// ------------------------------------ //
DLLEXPORT shared_ptr<NetworkResponse> Leviathan::ConnectionInfo::SendRequestAndBlockUntilDone(shared_ptr<NetworkRequest> request, int maxtries /*= 2*/){
	// Send the request //
	shared_ptr<MadeNetworkRequest> sentrequest = SendPacketToConnection(request, maxtries);

	// Now we wait //
	sentrequest->WaitForMe->get_future().get();

	// Remove it //
	ObjectLock guard(*this);
	_PopMadeRequest(sentrequest, guard);

	return sentrequest->GotResponse;
}

DLLEXPORT shared_ptr<MadeNetworkRequest> Leviathan::ConnectionInfo::SendPacketToConnection(shared_ptr<NetworkRequest> request, int maxretries){
	ObjectLock guard(*this);
	// Generate a packet from the request //
	sf::Packet tosend = request->GeneratePacketForRequest();

	_Socket.send(tosend, TargetHost, TargetPortNumber);

	// Add to the sent packets //
	shared_ptr<MadeNetworkRequest> tmprequestinfo(new MadeNetworkRequest(request->GetExpectedResponseID(), request, shared_ptr<boost::promise<bool>>(new 
		boost::promise<bool>()), maxretries, request->GetTimeoutMilliseconds(), 1));

	WaitingRequests.push_back(tmprequestinfo);

	return tmprequestinfo;
}
// ------------------------------------ //
void Leviathan::ConnectionInfo::_PopMadeRequest(shared_ptr<MadeNetworkRequest> objectptr, ObjectLock &guard){
	VerifyLock(guard);

	for(auto iter = WaitingRequests.begin(); iter != WaitingRequests.end(); ++iter){

		if((*iter).get() == objectptr.get()){

			WaitingRequests.erase(iter);
			return;
		}
	}
}

void Leviathan::ConnectionInfo::_ResendRequest(shared_ptr<MadeNetworkRequest> toresend, ObjectLock &guard){
	VerifyLock(guard);
#ifdef _DEBUG
	Logger::Get()->Info(L"ConnectionInfo: resending request");
#endif // _DEBUG
	// Generate a packet from the request //
	sf::Packet tosend = toresend->OriginalRequest->GeneratePacketForRequest();

	_Socket.send(tosend, TargetHost, TargetPortNumber);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ConnectionInfo::UpdateListening(){

	sf::Packet receivedpacket;

	sf::IpAddress sender;
	USHORT sentport;

	{
		ObjectLock guard(*this);

		if(_Socket.receive(receivedpacket, sender, sentport) == sf::Socket::Done){
			// Process packet //
			Logger::Get()->Info(L"ConnectionInfo: received a packet!");
		}

	}

	// Timeout stuff (if possible) //
	__int64 timems = Misc::GetTimeMs64();

	ObjectLock guard(*this);

	for(auto iter = WaitingRequests.begin(); iter != WaitingRequests.end(); ){

		// Check for resend //
		if(timems-(*iter)->RequestStartTime > (*iter)->TimeOutMS){
			// Move to next try //
			if(++(*iter)->AttempNumber > (*iter)->MaxTries){

				Logger::Get()->Warning(L"ConnectionInfo: request reached maximum tries");
				// We want to notify all waiters that it failed //
				(*iter)->WaitForMe->set_value(false);
				iter = WaitingRequests.erase(iter);
				continue;
			}
			// Resend request //
			_ResendRequest((*iter), guard);
		}


		// Move to next //
		++iter;
	}
}
// ------------------ MadeNetworkRequest ------------------ //
Leviathan::MadeNetworkRequest::MadeNetworkRequest(int expectedresponseid, shared_ptr<NetworkRequest> request, shared_ptr<boost::promise<bool>> waitobject, int maxtries, 
	int timeout, int attempnumber /*= 1*/) : ExpectedResponseID(expectedresponseid), WaitForMe(waitobject), ResponseReceiveTime(0), RequestStartTime(Misc::GetTimeMs64()),
	MaxTries(maxtries), AttempNumber(attempnumber), TimeOutMS(timeout), OriginalRequest(request)
{

}
