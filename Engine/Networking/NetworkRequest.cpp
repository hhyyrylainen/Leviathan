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
#ifdef _DEBUG
	switch(TypeOfRequest){
		// With these cases not having extra data is valid //
	case NETWORKREQUESTTYPE_IDENTIFICATION: case NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE:
		return;
	default:
		assert(0 && "trying to create a request which requires extra data without providing any extra data!");
	}

#endif // _DEBUG
}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(RemoteConsoleOpenRequestDataTo* newddata, int timeout /*= 1000*/, PACKET_TIMEOUT_STYLE style 
	/*= PACKAGE_TIMEOUT_STYLE_TIMEDMS*/) : ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_OPENREMOTECONSOLETO), 
	TimeOutValue(timeout), TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(RemoteConsoleAccessRequestData* newddata, int timeout /*= 1000*/, PACKET_TIMEOUT_STYLE style 
	/*= PACKAGE_TIMEOUT_STYLE_TIMEDMS*/) : ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE), 
	TimeOutValue(timeout), TimeOutStyle(style), RequestData(newddata)
{

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
	case NETWORKREQUESTTYPE_OPENREMOTECONSOLETO:
		{
			RequestData = new RemoteConsoleOpenRequestDataTo(frompacket);
		}
		break;
	case NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE:
		{
			RequestData = new RemoteConsoleAccessRequestData(frompacket);
		}
		break;
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
DLLEXPORT RemoteConsoleOpenRequestDataTo* Leviathan::NetworkRequest::GetRemoteConsoleOpenToDataIfPossible(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_OPENREMOTECONSOLETO)
		return static_cast<RemoteConsoleOpenRequestDataTo*>(RequestData);
	return NULL;
}

DLLEXPORT RemoteConsoleAccessRequestData* Leviathan::NetworkRequest::GetRemoteConsoleAccessRequestDataIfPossible(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE)
		return static_cast<RemoteConsoleAccessRequestData*>(RequestData);
	return NULL;
}
// ------------------ RemoteConsoleOpenRequestDataTo ------------------ //
DLLEXPORT Leviathan::RemoteConsoleOpenRequestDataTo::RemoteConsoleOpenRequestDataTo(int token) : SessionToken(token){

}

DLLEXPORT Leviathan::RemoteConsoleOpenRequestDataTo::RemoteConsoleOpenRequestDataTo(sf::Packet &frompacket){
	if(!(frompacket >> SessionToken)){
		throw ExceptionInvalidArgument(L"invalid packet to RemoteConsoleOpenRequestDataTo", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::RemoteConsoleOpenRequestDataTo::AddDataToPacket(sf::Packet &packet){
	packet << SessionToken;
}
// ------------------ RemoteConsoleAccess ------------------ //
DLLEXPORT Leviathan::RemoteConsoleAccessRequestData::RemoteConsoleAccessRequestData(int token) : SessionToken(token){

}

DLLEXPORT Leviathan::RemoteConsoleAccessRequestData::RemoteConsoleAccessRequestData(sf::Packet &frompacket){
	if(!(frompacket >> SessionToken)){
		throw ExceptionInvalidArgument(L"invalid packet to RemoteConsoleOpenRequestDataTo", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::RemoteConsoleAccessRequestData::AddDataToPacket(sf::Packet &packet){
	packet << SessionToken;
}
