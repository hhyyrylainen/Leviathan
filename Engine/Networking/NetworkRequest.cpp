#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKREQUEST
#include "NetworkRequest.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(NETWORKREQUESTTYPE type, int timeout /*= 1000*/, PACKET_TIMEOUT_STYLE style 
	/*= PACKAGE_TIMEOUT_STYLE_TIMEDMS*/) : ResponseID(IDFactory::GetID()), TypeOfRequest(type), TimeOutValue(timeout), TimeOutStyle(style)
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

DLLEXPORT int Leviathan::NetworkRequest::GetTimeOutValue(){
	return TimeOutValue;
}

DLLEXPORT PACKET_TIMEOUT_STYLE Leviathan::NetworkRequest::GetTimeOutType(){
	return TimeOutStyle;
}
// ------------------------------------ //

// ------------------------------------ //


