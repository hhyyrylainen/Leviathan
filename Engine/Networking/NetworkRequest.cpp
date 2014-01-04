#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKREQUEST
#include "NetworkRequest.h"
#endif
#include "Exceptions/ExceptionInvalidArgument.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(NETWORKREQUESTTYPE type, int timeout /*= 1000*/, PACKET_TIMEOUT_STYLE style 
	/*= PACKAGE_TIMEOUT_STYLE_TIMEDMS*/) : ResponseID(IDFactory::GetID()), TypeOfRequest(type), TimeOutValue(timeout), TimeOutStyle(style),
	RequestData(NULL)
{
	// We need to make sure the type is correct for this kind of packet //
	assert(type == NETWORKREQUESTTYPE_IDENTIFICATION && "trying to create a request which requires extra data without providing any extra data!");
}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(sf::Packet &frompacket){
	// Get the heading data //
	if(!(frompacket >> ResponseID)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"frompacket", L"");
	}
	int tmpval;
	if(!(frompacket >> tmpval)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"frompacket", L"");
	}
	TypeOfRequest = static_cast<NETWORKREQUESTTYPE>(tmpval);

	// Try to create the additional data if required for this type //
	switch(TypeOfRequest){
	//case :
		//{
			//RequestData = new (frompacket);
		//}
		//break;
	default:
		{
			// Nothing to get //
			RequestData = NULL;
		}
		break;
	}
}

DLLEXPORT Leviathan::NetworkRequest::~NetworkRequest(){
	SAFE_DELETE(RequestData);
}
// ------------------------------------ //
DLLEXPORT sf::Packet Leviathan::NetworkRequest::GeneratePacketForRequest(){

	sf::Packet packet;
	// Set the heading data //
	packet << ResponseID << TypeOfRequest;

	// Add data, if needed for the request //
	if(RequestData){
		RequestData->AddDataToPacket(packet);
	}

	// Return the finished packet //
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

DLLEXPORT NETWORKREQUESTTYPE Leviathan::NetworkRequest::GetType(){
	return TypeOfRequest;
}
// ------------------------------------ //

// ------------------------------------ //


