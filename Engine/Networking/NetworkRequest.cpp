// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKREQUEST
#include "NetworkRequest.h"
#endif
#include "Exceptions.h"
#include "GameSpecificPacketHandler.h"
#include "NetworkedInput.h"
#include "../Handlers/IDFactory.h"
#include "../Utility/Convert.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(NETWORKREQUESTTYPE type, int timeout /*= 1000*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_TIMEDMS*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(type), TimeOutValue(timeout), TimeOutStyle(style), RequestData(NULL)
{
	// We need to make sure the type is correct for this kind of packet //
#ifdef _DEBUG
	switch(TypeOfRequest){
		// With these cases not having extra data is valid //
        case NETWORKREQUESTTYPE_IDENTIFICATION: case NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE:
        case NETWORKREQUESTTYPE_SERVERSTATUS: case NETWORKREQUESTTYPE_GETALLSYNCVALUES: case NETWORKREQUESTTYPE_ECHO:
		return;
	default:
		assert(0 && "trying to create a request which requires extra data without providing any extra data!");
	}

#endif // _DEBUG
}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(RemoteConsoleOpenRequestDataTo* newddata, int timeout /*= 1000*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_TIMEDMS*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_OPENREMOTECONSOLETO), TimeOutValue(timeout),
    TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(RemoteConsoleAccessRequestData* newddata, int timeout /*= 1000*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_TIMEDMS*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE), TimeOutValue(timeout),
    TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(JoinServerRequestData* newddata, int timeout /*= 1000*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_TIMEDMS*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_JOINSERVER), TimeOutValue(timeout),
    TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(GetSingleSyncValueRequestData* newddata, int timeout /*= 1000*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_TIMEDMS*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_GETSINGLESYNCVALUE), TimeOutValue(timeout),
    TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(CustomRequestData* newddata, int timeout /*= 1000*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_TIMEDMS*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_CUSTOM), TimeOutValue(timeout),
    TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(RequestCommandExecutionData* newddata, int timeout /*= 10*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_REQUESTEXECUTION), TimeOutValue(timeout),
    TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(RequestConnectInputData* newddata, int timeout /*= 1000*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_TIMEDMS*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_CONNECTINPUT), TimeOutValue(timeout),
    TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(RequestWorldClockSyncData* newddata, int timeout /*= 1000*/,
    PACKET_TIMEOUT_STYLE style /*= PACKET_TIMEOUT_STYLE_TIMEDMS*/) :
    ResponseID(IDFactory::GetID()), TypeOfRequest(NETWORKREQUESTTYPE_WORLD_CLOCK_SYNC), TimeOutValue(timeout),
    TimeOutStyle(style), RequestData(newddata)
{

}

DLLEXPORT Leviathan::NetworkRequest::NetworkRequest(sf::Packet &frompacket) : TimeOutValue(-1){
	// Get the heading data //
	frompacket >> ResponseID;

	int tmpval;
	frompacket >> tmpval;

    if(!(frompacket))
        throw InvalidArgument("packet has invalid format");
    
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
        case NETWORKREQUESTTYPE_REQUESTEXECUTION:
		{
			RequestData = new RequestCommandExecutionData(frompacket);
		}
		break;
        case NETWORKREQUESTTYPE_CONNECTINPUT:
		{
			RequestData = new RequestConnectInputData(frompacket);
		}
		break;
        case NETWORKREQUESTTYPE_WORLD_CLOCK_SYNC:
        {
            RequestData = new RequestWorldClockSyncData(frompacket);
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
	if(TypeOfRequest == NETWORKREQUESTTYPE_OPENREMOTECONSOLETO && RequestData)
		return static_cast<RemoteConsoleOpenRequestDataTo*>(RequestData);
	return NULL;
}

DLLEXPORT RemoteConsoleAccessRequestData* Leviathan::NetworkRequest::GetRemoteConsoleAccessRequestData(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE && RequestData)
		return static_cast<RemoteConsoleAccessRequestData*>(RequestData);
	return NULL;
}

DLLEXPORT CustomRequestData* Leviathan::NetworkRequest::GetCustomRequestData(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_CUSTOM && RequestData)
		return static_cast<CustomRequestData*>(RequestData);
	return NULL;
}

DLLEXPORT RequestCommandExecutionData* Leviathan::NetworkRequest::GetCommandExecutionRequestData(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_REQUESTEXECUTION && RequestData)
		return static_cast<RequestCommandExecutionData*>(RequestData);
	return NULL;
}

DLLEXPORT RequestConnectInputData* Leviathan::NetworkRequest::GetConnectInputRequestData(){
	if(TypeOfRequest == NETWORKREQUESTTYPE_CONNECTINPUT && RequestData)
		return static_cast<RequestConnectInputData*>(RequestData);
	return NULL;
}

DLLEXPORT RequestWorldClockSyncData* Leviathan::NetworkRequest::GetWorldClockSyncRequestData(){
    if(TypeOfRequest == NETWORKREQUESTTYPE_WORLD_CLOCK_SYNC && RequestData)
        return static_cast<RequestWorldClockSyncData*>(RequestData);
    return NULL;
}
// ------------------ RemoteConsoleOpenRequestDataTo ------------------ //
DLLEXPORT Leviathan::RemoteConsoleOpenRequestDataTo::RemoteConsoleOpenRequestDataTo(int token) : SessionToken(token){

}

DLLEXPORT Leviathan::RemoteConsoleOpenRequestDataTo::RemoteConsoleOpenRequestDataTo(sf::Packet &frompacket){
	if(!(frompacket >> SessionToken)){
		throw InvalidArgument("invalid packet");
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
		throw InvalidArgument("invalid packet");
	}
}

DLLEXPORT void Leviathan::RemoteConsoleAccessRequestData::AddDataToPacket(sf::Packet &packet){
	packet << SessionToken;
}
// ------------------ JoinServerRequestData ------------------ //
DLLEXPORT Leviathan::JoinServerRequestData::JoinServerRequestData(int outmasterid /*= -1*/) :
    MasterServerID(outmasterid)
{

}

DLLEXPORT Leviathan::JoinServerRequestData::JoinServerRequestData(sf::Packet &frompacket){
	if(!(frompacket >> MasterServerID)){
		throw InvalidArgument("invalid packet");
	}
}

DLLEXPORT void Leviathan::JoinServerRequestData::AddDataToPacket(sf::Packet &packet){
	packet << MasterServerID;
}
// ------------------ JoinServerRequestData ------------------ //
DLLEXPORT Leviathan::GetSingleSyncValueRequestData::GetSingleSyncValueRequestData(const std::string &name) :
    NameOfValue(name)
{

}

DLLEXPORT Leviathan::GetSingleSyncValueRequestData::GetSingleSyncValueRequestData(sf::Packet &frompacket){
	if(!(frompacket >> NameOfValue)){
		throw InvalidArgument("invalid packet");
	}
}

DLLEXPORT void Leviathan::GetSingleSyncValueRequestData::AddDataToPacket(sf::Packet &packet){
	packet << NameOfValue;
}
// ------------------ CustomRequestData ------------------ //
DLLEXPORT Leviathan::CustomRequestData::CustomRequestData(GameSpecificPacketData* newddata) :
    ActualPacketData(newddata)
{

}

DLLEXPORT Leviathan::CustomRequestData::CustomRequestData(BaseGameSpecificRequestPacket* newddata) : 
	ActualPacketData(new GameSpecificPacketData(newddata))
{
	
}

DLLEXPORT Leviathan::CustomRequestData::CustomRequestData(sf::Packet &frompacket){
	ActualPacketData = GameSpecificPacketHandler::Get()->ReadGameSpecificPacketFromPacket(false, frompacket);
	if(!ActualPacketData){
		// Because the above loading function doesn't throw, we should throw here
		throw InvalidArgument("invalid packet format for user defined request");
	}
}

DLLEXPORT void Leviathan::CustomRequestData::AddDataToPacket(sf::Packet &packet){
	GameSpecificPacketHandler::Get()->PassGameSpecificDataToPacket(ActualPacketData.get(), packet);
}
// ------------------ RequestCommandExecution ------------------ //
DLLEXPORT Leviathan::RequestCommandExecutionData::RequestCommandExecutionData(const string &commandstr) :
    Command(commandstr)
{

	if(Command.length() > MAX_SERVERCOMMAND_LENGTH){

		Logger::Get()->Warning("NetworkRequest: RequestCommandExecution: command is too long (is "+
            Convert::ToString(Command.length())
			+") : "+Command+" will be truncated:");

		// Cut it to fit //
		Command.resize(MAX_SERVERCOMMAND_LENGTH);

		Logger::Get()->Write("\t> "+Command+"\n");
	}
}

DLLEXPORT Leviathan::RequestCommandExecutionData::RequestCommandExecutionData(sf::Packet &frompacket){
	if(!(frompacket >> Command)){
		throw InvalidArgument("invalid packet");
	}
}

DLLEXPORT void Leviathan::RequestCommandExecutionData::AddDataToPacket(sf::Packet &packet){
	packet << Command;
}
// ------------------ RequestConnectInputData ------------------ //
DLLEXPORT Leviathan::RequestConnectInputData::RequestConnectInputData(NetworkedInput &tosend){
	// Copy the data to our packet //

	tosend.AddFullDataToPacket(DataForObject);
}

DLLEXPORT Leviathan::RequestConnectInputData::RequestConnectInputData(sf::Packet &frompacket){
	// Load the packet from the packet, packetception? //
	string tmpstr;
	frompacket >> tmpstr;

	// Fill the actual packet //
	DataForObject.append(tmpstr.c_str(), tmpstr.size());
}

DLLEXPORT void Leviathan::RequestConnectInputData::AddDataToPacket(sf::Packet &packet){


	packet << string(reinterpret_cast<const char*>(DataForObject.getData()), DataForObject.getDataSize());
}
// ------------------ RequestWorldClockSyncData ------------------ //
DLLEXPORT RequestWorldClockSyncData::RequestWorldClockSyncData(sf::Packet &frompacket){

    frompacket >> WorldID >> Ticks >> EngineMSTweak >> Absolute;

    if(!frompacket)
        throw InvalidArgument("invalid packet");
}

DLLEXPORT RequestWorldClockSyncData::RequestWorldClockSyncData(int worldid, int ticks, int enginetick,
    bool absolute /*= true*/) :
    WorldID(worldid), Ticks(ticks), Absolute(absolute), EngineMSTweak(enginetick)
{

}

DLLEXPORT void Leviathan::RequestWorldClockSyncData::AddDataToPacket(sf::Packet &packet){
    packet << WorldID << Ticks << EngineMSTweak << Absolute;
}


















