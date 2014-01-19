#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKRESPONSE
#include "NetworkResponse.h"
#endif
#include "Exceptions/ExceptionInvalidArgument.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkResponse::NetworkResponse(int inresponseto, PACKET_TIMEOUT_STYLE timeout, int timeoutvalue) : 
	ResponseType(NETWORKRESPONSETYPE_NONE), ResponseID(inresponseto), TimeOutStyle(timeout), TimeOutValue(timeoutvalue), ResponseData(NULL)
{

}

DLLEXPORT Leviathan::NetworkResponse::NetworkResponse(sf::Packet &receivedresponse){
	// First thing is the response ID //
	if(!(receivedresponse >> ResponseID)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"receivedresponse", L"");
	}

	// Second is the type, based on which we handle the rest of the data //
	int tmpval;
	if(!(receivedresponse >> tmpval)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"receivedresponse", L"");
	}
	ResponseType = static_cast<NETWORKRESPONSETYPE>(tmpval);

	// Process based on the type //
	switch(ResponseType){
	case NETWORKRESPONSETYPE_NONE: case NETWORKRESPONSETYPE_KEEPALIVE: case NETWORKRESPONSETYPE_CLOSECONNECTION: case NETWORKRESPONSETYPE_REMOTECONSOLEOPENED:
		{
			// There is no data in the packet //
			ResponseData = NULL;
			return;
		}
	case NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS:
		{
			ResponseData = new NetworkResponseDataForIdentificationString(receivedresponse);
		}
		break;
	default:
		{
			throw ExceptionInvalidArgument(L"packet has invalid type", 0, __WFUNCTION__, L"receivedresponse", Convert::ToWstring(ResponseType));
		}
		break;
	}
}

DLLEXPORT Leviathan::NetworkResponse::~NetworkResponse(){
	SAFE_DELETE(ResponseData);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkResponse::GenerateIdentificationStringResponse(NetworkResponseDataForIdentificationString* newddata){
	ResponseType = NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateKeepAliveResponse(){
	ResponseType = NETWORKRESPONSETYPE_KEEPALIVE;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateCloseConnectionResponse(){
	ResponseType = NETWORKRESPONSETYPE_CLOSECONNECTION;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateRemoteConsoleOpenedResponse(){
	ResponseType = NETWORKRESPONSETYPE_REMOTECONSOLEOPENED;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateEmptyResponse(){
	ResponseType = NETWORKRESPONSETYPE_NONE;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}
// ------------------------------------ //
DLLEXPORT sf::Packet Leviathan::NetworkResponse::GeneratePacketForResponse(){

	sf::Packet generatedpacket;
	// First thing is the header //
	generatedpacket << ResponseID << (int)ResponseType;

	// We don't need to enforce the type here //
	//if(ResponseType != NETWORKRESPONSETYPE_NONE){
	if(ResponseData){
		// Add the data //
		ResponseData->AddDataToPacket(generatedpacket);
	}

	// Return the finished packet //
	return generatedpacket;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::NetworkResponse::GetTimeOutValue(){
	return TimeOutValue;
}

DLLEXPORT PACKET_TIMEOUT_STYLE Leviathan::NetworkResponse::GetTimeOutType(){
	return TimeOutStyle;
}

DLLEXPORT int Leviathan::NetworkResponse::GetResponseID(){
	return ResponseID;
}

DLLEXPORT NETWORKRESPONSETYPE Leviathan::NetworkResponse::GetTypeOfResponse(){
	return ResponseType;
}
// ------------------------------------ //
DLLEXPORT NetworkResponseDataForIdentificationString* Leviathan::NetworkResponse::GetResponseDataForIdentificationString(){
	if(ResponseType == NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS && ResponseData)
		return static_cast<NetworkResponseDataForIdentificationString*>(ResponseData);
	return NULL;
}
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForIdentificationString::NetworkResponseDataForIdentificationString(sf::Packet &frompacket){
	// Extract the data from the packet //
	if(!(frompacket >> UserReadableData)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}
	if(!(frompacket >> GameName)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}
	if(!(frompacket >> GameVersionString)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}
	if(!(frompacket >> LeviathanVersionString)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT Leviathan::NetworkResponseDataForIdentificationString::NetworkResponseDataForIdentificationString(const wstring &userreadableidentification, 
	const wstring &gamename, const wstring &gameversion, const wstring &leviathanversion) : UserReadableData(userreadableidentification), 
	GameName(gamename), GameVersionString(gameversion), LeviathanVersionString(leviathanversion)
{

}

DLLEXPORT void Leviathan::NetworkResponseDataForIdentificationString::AddDataToPacket(sf::Packet &packet){
	packet << UserReadableData << GameName << GameVersionString << LeviathanVersionString;
}
