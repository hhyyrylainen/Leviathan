#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKREQUEST
#include "NetworkRequest.h"
#endif
#include "Exceptions/ExceptionInvalidArgument.h"
#include "GameSpecificPacketHandler.h"
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
	case NETWORKREQUESTTYPE_IDENTIFICATION: case NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE: case NETWORKREQUESTTYPE_SERVERSTATUS:
	case NETWORKREQUESTTYPE_GETALLSYNCVALUES:
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

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(JoinServerRequestData* newddata, int timeout /*= 1000*/, PACKET_TIMEOUT_STYLE style 
	/*= PACKAGE_TIMEOUT_STYLE_TIMEDMS*/) : ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_JOINSERVER), 
	TimeOutValue(timeout), TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(GetSingleSyncValueRequestData* newddata, int timeout /*= 1000*/, PACKET_TIMEOUT_STYLE style 
	/*= PACKAGE_TIMEOUT_STYLE_TIMEDMS*/) : ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_GETSINGLESYNCVALUE), 
	TimeOutValue(timeout), TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(CustomRequestData* newddata, int timeout /*= 1000*/, PACKET_TIMEOUT_STYLE style 
	/*= PACKAGE_TIMEOUT_STYLE_TIMEDMS*/) : ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_CUSTOM), 
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
	case NETWORKREQUESTTYPE_JOINSERVER:
		{
			RequestData = new JoinServerRequestData(frompacket);
		}
		break;
	case NETWORKREQUESTTYPE_GETSINGLESYNCVALUE:
		{
			RequestData = new GetSingleSyncValueRequestData(frompacket);
		}
		break;
	case NETWORKREQUESTTYPE_CUSTOM:
		{
			RequestData = new CustomRequestData(frompacket);
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
DLLEXPORT RemoteConsoleOpenRequestDataTo* Leviathan::NetworkRequest::GetRemoteConsoleOpenToData(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_OPENREMOTECONSOLETO)
		return static_cast<RemoteConsoleOpenRequestDataTo*>(RequestData);
	return NULL;
}

DLLEXPORT RemoteConsoleAccessRequestData* Leviathan::NetworkRequest::GetRemoteConsoleAccessRequestData(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE)
		return static_cast<RemoteConsoleAccessRequestData*>(RequestData);
	return NULL;
}

DLLEXPORT CustomRequestData* Leviathan::NetworkRequest::GetCustomRequestData(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_CUSTOM)
		return static_cast<CustomRequestData*>(RequestData);
	return NULL;
}
// ------------------ RemoteConsoleOpenRequestDataTo ------------------ //
DLLEXPORT Leviathan::RemoteConsoleOpenRequestDataTo::RemoteConsoleOpenRequestDataTo(int token) : SessionToken(token){

}

DLLEXPORT Leviathan::RemoteConsoleOpenRequestDataTo::RemoteConsoleOpenRequestDataTo(sf::Packet &frompacket){
	if(!(frompacket >> SessionToken)){
		throw ExceptionInvalidArgument(L"invalid packet", 0, __WFUNCTION__, L"frompacket", L"");
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
		throw ExceptionInvalidArgument(L"invalid packet", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::RemoteConsoleAccessRequestData::AddDataToPacket(sf::Packet &packet){
	packet << SessionToken;
}
// ------------------ JoinServerRequestData ------------------ //
DLLEXPORT Leviathan::JoinServerRequestData::JoinServerRequestData(int outmasterid /*= -1*/) : MasterServerID(outmasterid){

}

DLLEXPORT Leviathan::JoinServerRequestData::JoinServerRequestData(sf::Packet &frompacket){
	if(!(frompacket >> MasterServerID)){
		throw ExceptionInvalidArgument(L"invalid packet", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::JoinServerRequestData::AddDataToPacket(sf::Packet &packet){
	packet << MasterServerID;
}
// ------------------ JoinServerRequestData ------------------ //
DLLEXPORT Leviathan::GetSingleSyncValueRequestData::GetSingleSyncValueRequestData(const wstring &name) : NameOfValue(name){

}

DLLEXPORT Leviathan::GetSingleSyncValueRequestData::GetSingleSyncValueRequestData(sf::Packet &frompacket){
	if(!(frompacket >> NameOfValue)){
		throw ExceptionInvalidArgument(L"invalid packet", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::GetSingleSyncValueRequestData::AddDataToPacket(sf::Packet &packet){
	packet << NameOfValue;
}
// ------------------ CustomRequestData ------------------ //
DLLEXPORT Leviathan::CustomRequestData::CustomRequestData(GameSpecificPacketData* newddata) : ActualPacketData(newddata){

}

DLLEXPORT Leviathan::CustomRequestData::CustomRequestData(BaseGameSpecificRequestPacket* newddata) : 
	ActualPacketData(new GameSpecificPacketData(newddata))
{
	
}

DLLEXPORT Leviathan::CustomRequestData::CustomRequestData(sf::Packet &frompacket){
	ActualPacketData = GameSpecificPacketHandler::Get()->ReadGameSpecificPacketFromPacket(false, frompacket);
	if(!ActualPacketData){
		// Because the above loading function doesn't throw, we should throw here
		throw ExceptionInvalidArgument(L"invalid packet format for user defined request", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::CustomRequestData::AddDataToPacket(sf::Packet &packet){
	GameSpecificPacketHandler::Get()->PassGameSpecificDataToPacket(ActualPacketData.get(), packet);
}
