#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKREQUEST
#include "NetworkRequest.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(NETWORKREQUESTTYPE type, int timeout /*= 1000*/) : ResponseID(IDFactory::GetID()), 
	TypeOfRequest(type), TimeOutMS(timeout)
{

}

DLLEXPORT Leviathan::NetworkRequest::~NetworkRequest(){

}
// ------------------------------------ //
DLLEXPORT sf::Packet Leviathan::NetworkRequest::GeneratePacketForRequest(){

	sf::Packet packet;

	packet << TypeOfRequest;
	packet << ResponseID;


	return packet;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::NetworkRequest::GetExpectedResponseID(){
	return ResponseID;
}

DLLEXPORT int Leviathan::NetworkRequest::GetTimeoutMilliseconds(){
	return TimeOutMS;
}
// ------------------------------------ //

// ------------------------------------ //


