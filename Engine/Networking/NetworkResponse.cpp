#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKRESPONSE
#include "NetworkResponse.h"
#endif
#include "Exceptions/ExceptionInvalidArgument.h"
#include "Common/DataStoring/NamedVars.h"
#include "GameSpecificPacketHandler.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkResponse::NetworkResponse(int inresponseto, PACKET_TIMEOUT_STYLE timeout, int timeoutvalue) : 
	ResponseType(NETWORKRESPONSETYPE_NONE), ResponseID(inresponseto), TimeOutStyle(timeout), TimeOutValue(timeoutvalue), ResponseData(NULL)
{

}

DLLEXPORT Leviathan::NetworkResponse::NetworkResponse(sf::Packet &receivedresponse) : TimeOutValue(-1){
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
	case NETWORKRESPONSETYPE_REMOTECONSOLECLOSED:
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
	case NETWORKRESPONSETYPE_INVALIDREQUEST:
		{
			ResponseData = new NetworkResponseDataForInvalidRequest(receivedresponse);
		}
		break;
	case NETWORKRESPONSETYPE_SERVERSTATUS:
		{
			ResponseData = new NetworkResponseDataForServerStatus(receivedresponse);
		}
		break;
	case NETWORKRESPONSETYPE_SERVERALLOW:
		{
			ResponseData = new NetworkResponseDataForServerAllow(receivedresponse);
		}
		break;
	case NETWORKRESPONSETYPE_SERVERDISALLOW:
		{
			ResponseData = new NetworkResponseDataForServerDisallow(receivedresponse);
		}
		break;
	case NETWORKRESPONSETYPE_SYNCVALDATA:
		{
			ResponseData = new NetworkResponseDataForSyncValData(receivedresponse);
		}
		break;
	case NETWORKRESPONSETYPE_SYNCDATAEND:
		{
			ResponseData = new NetworkResponseDataForSyncDataEnd(receivedresponse);
		}
		break;
	case NETWORKRESPONSETYPE_CUSTOM:
		{
			ResponseData = new NetworkResponseDataForCustom(receivedresponse);
		}
		break;
	case NETWORKRESPONSETYPE_SYNCRESOURCEDATA:
		{
			ResponseData = new NetworkResponseDataForSyncResourceData(receivedresponse);
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

DLLEXPORT void Leviathan::NetworkResponse::GenerateInvalidRequestResponse(NetworkResponseDataForInvalidRequest* newddata){
	ResponseType = NETWORKRESPONSETYPE_INVALIDREQUEST;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateServerStatusResponse(NetworkResponseDataForServerStatus* newddata){
	ResponseType = NETWORKRESPONSETYPE_SERVERSTATUS;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateServerDisallowResponse(NetworkResponseDataForServerDisallow* newddata){
	ResponseType = NETWORKRESPONSETYPE_SERVERDISALLOW;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateServerAllowResponse(NetworkResponseDataForServerAllow* newddata){
	ResponseType = NETWORKRESPONSETYPE_SERVERALLOW;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateValueSyncResponse(NetworkResponseDataForSyncValData* newddata){
	ResponseType = NETWORKRESPONSETYPE_SYNCVALDATA;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateValueSyncEndResponse(NetworkResponseDataForSyncDataEnd* newddata){
	ResponseType = NETWORKRESPONSETYPE_SYNCDATAEND;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = newddata;
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateResourceSyncResponse(const char* dataptr, size_t datasize){
	ResponseType = NETWORKRESPONSETYPE_SYNCRESOURCEDATA;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = new NetworkResponseDataForSyncResourceData(string(dataptr, datasize));
}
// ------------------------------------ //
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

DLLEXPORT void Leviathan::NetworkResponse::GenerateRemoteConsoleClosedResponse(){
	ResponseType = NETWORKRESPONSETYPE_REMOTECONSOLECLOSED;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateEmptyResponse(){
	ResponseType = NETWORKRESPONSETYPE_NONE;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkResponse::GenerateCustomResponse(GameSpecificPacketData* newdpacketdata){
	ResponseType = NETWORKRESPONSETYPE_CUSTOM;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = new NetworkResponseDataForCustom(newdpacketdata);
}

DLLEXPORT void Leviathan::NetworkResponse::GenerateCustomResponse(BaseGameSpecificResponsePacket* newdpacketdata){
	ResponseType = NETWORKRESPONSETYPE_CUSTOM;
	// Destroy old data if any //
	SAFE_DELETE(ResponseData);

	ResponseData = new NetworkResponseDataForCustom(new GameSpecificPacketData(newdpacketdata));
}
// ------------------------------------ //
DLLEXPORT sf::Packet Leviathan::NetworkResponse::GeneratePacketForResponse() const{

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
DLLEXPORT int Leviathan::NetworkResponse::GetTimeOutValue() const{
	return TimeOutValue;
}

DLLEXPORT PACKET_TIMEOUT_STYLE Leviathan::NetworkResponse::GetTimeOutType() const{
	return TimeOutStyle;
}

DLLEXPORT int Leviathan::NetworkResponse::GetResponseID() const{
	return ResponseID;
}

DLLEXPORT NETWORKRESPONSETYPE Leviathan::NetworkResponse::GetTypeOfResponse() const{
	return ResponseType;
}

DLLEXPORT NETWORKRESPONSETYPE Leviathan::NetworkResponse::GetType() const{
	return ResponseType;
}
// ------------------------------------ //
DLLEXPORT NetworkResponseDataForIdentificationString* Leviathan::NetworkResponse::GetResponseDataForIdentificationString() const{
	if(ResponseType == NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS && ResponseData)
		return static_cast<NetworkResponseDataForIdentificationString*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForServerStatus* Leviathan::NetworkResponse::GetResponseDataForServerStatus() const{
	if(ResponseType == NETWORKRESPONSETYPE_SERVERSTATUS && ResponseData)
		return static_cast<NetworkResponseDataForServerStatus*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForSyncValData* Leviathan::NetworkResponse::GetResponseDataForValueSyncResponse() const{
	if(ResponseType == NETWORKRESPONSETYPE_SYNCVALDATA && ResponseData)
		return static_cast<NetworkResponseDataForSyncValData*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForSyncDataEnd* Leviathan::NetworkResponse::GetResponseDataForValueSyncEndResponse() const{
	if(ResponseType == NETWORKRESPONSETYPE_SYNCDATAEND && ResponseData)
		return static_cast<NetworkResponseDataForSyncDataEnd*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForCustom* Leviathan::NetworkResponse::GetResponseDataForGameSpecific() const{
	if(ResponseType == NETWORKRESPONSETYPE_CUSTOM && ResponseData)
		return static_cast<NetworkResponseDataForCustom*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForSyncResourceData* Leviathan::NetworkResponse::GetResponseDataForSyncResourceResponse() const{
	if(ResponseType == NETWORKRESPONSETYPE_SYNCRESOURCEDATA && ResponseData)
		return static_cast<NetworkResponseDataForSyncResourceData*>(ResponseData);
	return NULL;
}

DLLEXPORT NetworkResponseDataForServerAllow* Leviathan::NetworkResponse::GetResponseDataForServerAllowResponse() const{
	if(ResponseType == NETWORKRESPONSETYPE_SERVERALLOW && ResponseData)
		return static_cast<NetworkResponseDataForServerAllow*>(ResponseData);
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
// ------------------ NetworkResponseDataForInvalidRequest ------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForInvalidRequest::NetworkResponseDataForInvalidRequest(NETWORKRESPONSE_INVALIDREASON reason, 
	const wstring &additional /*= wstring()*/) : Invalidness(reason), AdditionalInfo(additional)
{

}

DLLEXPORT Leviathan::NetworkResponseDataForInvalidRequest::NetworkResponseDataForInvalidRequest(sf::Packet &frompacket){
	// Extract the data from the packet //
	int tmpextract;

	if(!(frompacket >> tmpextract)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	Invalidness = static_cast<NETWORKRESPONSE_INVALIDREASON>(tmpextract);

	if(!(frompacket >> AdditionalInfo)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::NetworkResponseDataForInvalidRequest::AddDataToPacket(sf::Packet &packet){
	packet << static_cast<int>(Invalidness) << AdditionalInfo;
}
// ------------------ NetworkResponseDataForServerStatus ------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForServerStatus::NetworkResponseDataForServerStatus(const wstring &servername, bool isjoinable, 
	NETWORKRESPONSE_SERVERJOINRESTRICT whocanjoin, int players, int maxplayers, int bots, NETWORKRESPONSE_SERVERSTATUS currentstatus, 
	int serverflags) : ServerNameString(servername), Joinable(isjoinable), JoinRestriction(whocanjoin), Players(players), MaxPlayers(maxplayers),
		Bots(bots), ServerStatus(currentstatus), AdditionalFlags(serverflags)
{
	// Check the length //
	if(ServerNameString.length() > 100){

		Logger::Get()->Warning(L"NetworkResponse: NetworkResponseDataForServerStatus: server name is too long, max 100 characters (is "+
			Convert::ToWstring(ServerNameString.length())+L") : "+ServerNameString+L" will be truncated:");
		ServerNameString.resize(100);
		Logger::Get()->Write(L"\t> "+ServerNameString+L"\n");
	}
}

DLLEXPORT Leviathan::NetworkResponseDataForServerStatus::NetworkResponseDataForServerStatus(sf::Packet &frompacket){
	// Try to reverse the process //
	int tmpextract;

	if(!(frompacket >> ServerNameString)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	// Just in case we got an invalid packet check this //
	if(ServerNameString.size() > 100){

		ServerNameString.resize(100);
		Logger::Get()->Warning(L"NetworkResponseDataForServerStatus: packet had too long server name string");
	}

	if(!(frompacket >> Joinable)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	if(!(frompacket >> tmpextract)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	JoinRestriction = static_cast<NETWORKRESPONSE_SERVERJOINRESTRICT>(tmpextract);

	if(!(frompacket >> Players)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	if(!(frompacket >> MaxPlayers)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	if(!(frompacket >> Bots)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	if(!(frompacket >> tmpextract)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	ServerStatus = static_cast<NETWORKRESPONSE_SERVERSTATUS>(tmpextract);

	if(!(frompacket >> AdditionalFlags)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::NetworkResponseDataForServerStatus::AddDataToPacket(sf::Packet &packet){
	packet << ServerNameString << Joinable << static_cast<int>(JoinRestriction) << Players << MaxPlayers << Bots << static_cast<int>(ServerStatus) << AdditionalFlags;
}
// ------------------ NetworkResponseDataForServerDisallow ------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForServerDisallow::NetworkResponseDataForServerDisallow(sf::Packet &frompacket){
	int tmpextract;

	if(!(frompacket >> tmpextract)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	Reason = static_cast<NETWORKRESPONSE_INVALIDREASON>(tmpextract);

	if(!(frompacket >> Message)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	// Just in case we got an invalid packet check this //
	if(Message.size() > 100){

		Message.resize(100);
		Logger::Get()->Warning(L"NetworkResponseDataForServerDisallow: packet had too long message string");
	}
}

DLLEXPORT Leviathan::NetworkResponseDataForServerDisallow::NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON reason, 
	const wstring &message /*= L"Default disallow"*/) : Reason(reason), Message(message)
{
	// Check the length //
	if(Message.length() > 100){

		Logger::Get()->Warning(L"NetworkResponse: NetworkResponseDataForServerDisallow: message is too long (is "+Convert::ToWstring(Message.length())
			+L") : "+Message+L" will be truncated:");
		Message.resize(100);
		Logger::Get()->Write(L"\t> "+Message+L"\n");
	}
}

DLLEXPORT void Leviathan::NetworkResponseDataForServerDisallow::AddDataToPacket(sf::Packet &packet){
	packet << Reason << Message;
}
// ------------------ NetworkResponseDataForServerAllow ------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForServerAllow::NetworkResponseDataForServerAllow(sf::Packet &frompacket){
	int tmpextract;

	if(!(frompacket >> tmpextract)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	ServerAcceptedWhat = static_cast<NETWORKRESPONSE_SERVERACCEPTED_TYPE>(tmpextract);

	if(!(frompacket >> Message)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}

	// Just in case we got an invalid packet check this //
	if(Message.size() > 100){

		Message.resize(100);
		Logger::Get()->Warning(L"NetworkResponseDataForServerAllow: packet had too long message string");
	}
}

DLLEXPORT Leviathan::NetworkResponseDataForServerAllow::NetworkResponseDataForServerAllow(NETWORKRESPONSE_SERVERACCEPTED_TYPE whataccepted, 
	const wstring &message /*= L""*/) : ServerAcceptedWhat(whataccepted), Message(message)
{
	// Check the length //
	if(Message.length() > 100){

		Logger::Get()->Warning(L"NetworkResponse: NetworkResponseDataForServerAllow: message is too long (is "+Convert::ToWstring(Message.length())
			+L") : "+Message+L" will be truncated:");
		Message.resize(100);
		Logger::Get()->Write(L"\t> "+Message+L"\n");
	}
}

DLLEXPORT void Leviathan::NetworkResponseDataForServerAllow::AddDataToPacket(sf::Packet &packet){
	packet << ServerAcceptedWhat << Message;
}
// ------------------ NetworkResponseDataForSyncValData ------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForSyncValData::NetworkResponseDataForSyncValData(NamedVariableList* newddata) : SyncValueData(newddata){

}

DLLEXPORT Leviathan::NetworkResponseDataForSyncValData::NetworkResponseDataForSyncValData(sf::Packet &frompacket){
	// Create new value from the packet //
	SyncValueData = shared_ptr<NamedVariableList>(new NamedVariableList(frompacket));
}

DLLEXPORT void Leviathan::NetworkResponseDataForSyncValData::AddDataToPacket(sf::Packet &packet){
	// Pass the data to the packet //
	SyncValueData->AddToPacket(packet);
}
// ------------------ NetworkResponseDataForSyncDataEnd ------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForSyncDataEnd::NetworkResponseDataForSyncDataEnd(sf::Packet &frompacket){
	if(!(frompacket >> Succeeded)){

		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT Leviathan::NetworkResponseDataForSyncDataEnd::NetworkResponseDataForSyncDataEnd(bool succeeded) : Succeeded(succeeded){

}

DLLEXPORT void Leviathan::NetworkResponseDataForSyncDataEnd::AddDataToPacket(sf::Packet &packet){
	packet << Succeeded;
}
// ------------------ NetworkResponseDataForGameSpecific ------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForCustom::NetworkResponseDataForCustom(GameSpecificPacketData* newdpacketdata) : 
	ActualPacketData(newdpacketdata)
{

}

DLLEXPORT Leviathan::NetworkResponseDataForCustom::NetworkResponseDataForCustom(BaseGameSpecificResponsePacket* newddata) : 
	ActualPacketData(new GameSpecificPacketData(newddata))
{

}

DLLEXPORT Leviathan::NetworkResponseDataForCustom::NetworkResponseDataForCustom(sf::Packet &frompacket){
	ActualPacketData = GameSpecificPacketHandler::Get()->ReadGameSpecificPacketFromPacket(true, frompacket);
	if(!ActualPacketData){
		// Because the above loading function doesn't throw, we should throw here
		throw ExceptionInvalidArgument(L"invalid packet format for user defined response", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT void Leviathan::NetworkResponseDataForCustom::AddDataToPacket(sf::Packet &packet){
	GameSpecificPacketHandler::Get()->PassGameSpecificDataToPacket(ActualPacketData.get(), packet);
}
// ------------------ NetworkResponseDataForCustom ------------------ //
DLLEXPORT Leviathan::NetworkResponseDataForSyncResourceData::NetworkResponseDataForSyncResourceData(sf::Packet &frompacket){
	
	if(!(frompacket >> OurCustomData)){
		throw ExceptionInvalidArgument(L"invalid packet format", 0, __WFUNCTION__, L"frompacket", L"");
	}
}

DLLEXPORT Leviathan::NetworkResponseDataForSyncResourceData::NetworkResponseDataForSyncResourceData(const string &containeddata) : 
	OurCustomData(containeddata)
{

}

DLLEXPORT void Leviathan::NetworkResponseDataForSyncResourceData::AddDataToPacket(sf::Packet &packet){
	packet << OurCustomData;
}
