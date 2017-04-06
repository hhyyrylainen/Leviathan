// ------------------------------------ //
#include "Connection.h"

#include "NetworkResponse.h"
#include "NetworkRequest.h"
#include "Iterators/StringIterator.h"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "NetworkHandler.h"
#include "Exceptions.h"
#include "RemoteConsole.h"
#include "Threading/ThreadingManager.h"
#include "TimeIncludes.h"
#include "Utility/Convert.h"
#include "Engine.h"
#include "Application/GameConfiguration.h"

#include "SentNetworkThing.h"

using namespace Leviathan;
// ------------------------------------ //

//! Makes the program spam a ton of debug info about packets //
#define SPAM_ME_SOME_PACKETS 1

//#define OUTPUT_PACKET_BITS 1

// ------------------------------------ //
DLLEXPORT Connection::Connection(const std::string &hostname) : 
    RawAddress(hostname), HostName(hostname)
{
    // We need to split the port number from the address //
    StringIterator itr(hostname);

    auto result = itr.GetUntilNextCharacterOrAll<std::string>(':');

    HostName = *result;

    // We should be fine not skipping a letter //
    result = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

    if(!result){
        
        GAMECONFIGURATION_GET_VARIABLEACCESS(vars);

        int tmpport = 0;

        if(!vars->GetValueAndConvertTo<int>("DefaultServerPort", tmpport)){

            tmpport = 80;
        }

        TargetPortNumber = (unsigned short)tmpport;
        
        LOG_WARNING("Connection: no port defined, using default, "
            "hostname: " + hostname + ":" + Convert::ToString(TargetPortNumber));
        return;
    }
    
    TargetPortNumber = Convert::StringTo<int>(*result.get());
}

DLLEXPORT Connection::Connection(const sf::IpAddress &targetaddress,
    unsigned short port) :
    TargetPortNumber(port), TargetHost(targetaddress), AddressGot(true)
{
    RawAddress = GenerateFormatedAddressString();
}

DLLEXPORT Connection::~Connection(){

}
// ------------------------------------ //
DLLEXPORT bool Connection::Init(NetworkHandler* owninghandler){
    
    Owner = owninghandler;

    {
        // TODO: make this asynchronous
        GUARD_LOCK();

        // This might do something //
        if(!AddressGot){
            TargetHost = sf::IpAddress(HostName);
        }
    }

    // We fail if we got an invalid address //
    if(TargetHost == sf::IpAddress::None){

        LOG_ERROR("Connection: Init: couldn't translate host name to a real address, "
            "host: " + HostName);
        return false;
    }

    Logger::Get()->Info("Connection: opening connection to " + 
        GenerateFormatedAddressString());

    // Reset various timers //
    LastSentPacketTime = LastReceivedPacketTime = Time::GetTimeMs64();

    // Send hello message //
    if(!SendPacketToConnection(std::make_shared<RequestConnect>(),
            RECEIVE_GUARANTEE::Critical))
    {
        LEVIATHAN_ASSERT(0, "Connection Init cannot send packet");
    }
    
    return true;
}

DLLEXPORT void Connection::Release(){

    GUARD_LOCK();

    Logger::Get()->Info("Connection: disconnecting from " + GenerateFormatedAddressString());

    // Send a close packet //
    // This will mark this as closed
    SendCloseConnectionPacket(guard);


    // Make sure that all our remaining packets fail //
    for(auto& packet : PendingRequests)
        packet->SetWaitStatus(false);

    PendingRequests.clear();

    for(auto& packet : ResponsesNeedingConfirmation)
        packet->SetWaitStatus(false);

    ResponsesNeedingConfirmation.clear();
    
    // All are now properly closed //

    // Destroy some of our stuff //
    TargetHost = sf::IpAddress::None;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<SentRequest> Connection::SendPacketToConnection(Lock &guard,
    const std::shared_ptr<NetworkRequest> &request, RECEIVE_GUARANTEE guarantee)
{
    if(!IsValidForSend() || !request)
        return nullptr;

    // Generate a packet from the request //
    sf::Packet actualpackettosend;

    std::array<uint32_t, 1> messages;
    // Requests don't use this marking mechanism
    messages[0] = ++LastUsedMessageNumber;
    
    // We need a complete header with acks and stuff //
    _PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, &messages[0], 1,
        actualpackettosend);

    // Request type //
    actualpackettosend << NORMAL_REQUEST_TYPE;
    
    // Message number //
    actualpackettosend << LastUsedMessageNumber;

    // Pack the message data in //
    request->AddDataToPacket(actualpackettosend);

    _SendPacketToSocket(actualpackettosend);

    // Add to the sent packets //
    auto sentthing = std::make_shared<SentRequest>(LastUsedLocalID, LastUsedMessageNumber,
        guarantee, request);

    PendingRequests.push_back(sentthing);

    return sentthing;
}

DLLEXPORT bool Connection::SendPacketToConnection(Lock &guard,
    const NetworkResponse &response)
{
    if(!IsValidForSend())
        return false;

    // Generate a packet from the request //
    sf::Packet actualpackettosend;

    std::array<uint32_t, 1> messages;
    messages[0] = ++LastUsedMessageNumber;
    
    // We need a complete header with acks and stuff //
    _PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, &messages[0], 1,
        actualpackettosend);

    // Request type //
    actualpackettosend << NORMAL_RESPONSE_TYPE;
    
    // Message number //
    actualpackettosend << LastUsedMessageNumber;

    // Pack the message data in //
    response.AddDataToPacket(actualpackettosend);
    
    _SendPacketToSocket(actualpackettosend);

    return true;
}

DLLEXPORT std::shared_ptr<SentResponse> Connection::SendPacketToConnection(Lock &guard,
    const std::shared_ptr<NetworkResponse> &response, RECEIVE_GUARANTEE guarantee)
{
    if(!IsValidForSend() || !response)
        return nullptr;

    // Generate a packet from the request //
    sf::Packet actualpackettosend;

    std::array<uint32_t, 1> messages;
    messages[0] = ++LastUsedMessageNumber;
    
    // We need a complete header with acks and stuff //
    _PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, &messages[0], 1,
        actualpackettosend);

    // Request type //
    actualpackettosend << NORMAL_RESPONSE_TYPE;
    
    // Message number //
    actualpackettosend << LastUsedMessageNumber;

    // Pack the message data in //
    response->AddDataToPacket(actualpackettosend);
    
    _SendPacketToSocket(actualpackettosend);

    // Add to the sent packets //
    if(guarantee != RECEIVE_GUARANTEE::None){
        
        auto sentthing = std::make_shared<SentResponse>(LastUsedLocalID, LastUsedMessageNumber,
            guarantee, response);

        ResponsesNeedingConfirmation.push_back(sentthing);

        return sentthing;
    }

    return nullptr;
}
// ------------------------------------ //
DLLEXPORT void Connection::SendKeepAlivePacket(Lock &guard){
    
    SendPacketToConnection(guard, ResponseNone(NETWORK_RESPONSE_TYPE::Keepalive));
}

DLLEXPORT void Connection::SendCloseConnectionPacket(Lock &guard){

    State = CONNECTION_STATE::Closed;

    SendPacketToConnection(guard, ResponseNone(NETWORK_RESPONSE_TYPE::CloseConnection));
}
// ------------------------------------ //
void Connection::_Resend(Lock &guard, SentRequest &toresend){

    // Generate a packet from the request //
    sf::Packet actualpackettosend;

    std::array<uint32_t, 1> messages;
    // Requests don't use this marking mechanism
    messages[0] = toresend.MessageNumber;
    
    // We need a complete header with acks and stuff //
    _PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, &messages[0], 1,
        actualpackettosend);

    // Request type //
    actualpackettosend << NORMAL_REQUEST_TYPE;
    
    // Message number //
    actualpackettosend << LastUsedMessageNumber;

    // Pack the message data in //
    toresend.SentRequestData->AddDataToPacket(actualpackettosend);

    _SendPacketToSocket(actualpackettosend);

    toresend.ResetStartTime();

    // Increase attempt number
    ++toresend.AttemptNumber;
}

void Connection::_Resend(Lock &guard, SentResponse &toresend){

    // Generate a packet from the request //
    sf::Packet actualpackettosend;

    std::array<uint32_t, 1> messages;
    // Requests don't use this marking mechanism
    messages[0] = toresend.MessageNumber;
    
    // We need a complete header with acks and stuff //
    _PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, &messages[0], 1,
        actualpackettosend);

    // Request type //
    actualpackettosend << NORMAL_RESPONSE_TYPE;
    
    // Message number //
    actualpackettosend << LastUsedMessageNumber;

    // Pack the message data in //
    toresend.SentResponseData->AddDataToPacket(actualpackettosend);

    _SendPacketToSocket(actualpackettosend);

    toresend.ResetStartTime();

    // Increase attempt number
    ++toresend.AttemptNumber;
}
// ------------------------------------ //
DLLEXPORT inline void Connection::HandleRemoteAck(Lock &guard, 
    uint32_t localidconfirmedassent)
{
    if(localidconfirmedassent > LastConfirmedSent)
        LastConfirmedSent = localidconfirmedassent;

    for(auto iter = ResponsesNeedingConfirmation.begin();
        iter != ResponsesNeedingConfirmation.end(); ++iter)
    {
        if(localidconfirmedassent == (*iter)->PacketNumber){

            (*iter)->OnFinalized(true);

            ResponsesNeedingConfirmation.erase(iter);
            break;
        }
    }

    // Check which ack packets have been received //
    for (auto iter = SentAckPackets.begin(); iter != SentAckPackets.end(); ++iter){

        if(localidconfirmedassent == (*iter)->InsidePacket){

            // Mark as properly sent //
            (*iter)->Received = true;

            //! Mark acks as received
            RemoveSucceededAcks(guard, *(*iter)->AcksInThePacket);

            SentAckPackets.erase(iter);
            break;
        }
    }
}
// ------------------------------------ //
template<class TSentType>
    void Leviathan::Connection::_HandleTimeouts(Lock &guard, int64_t timems,
        std::vector<std::shared_ptr<TSentType>> sentthing)
{
    for(auto iter = sentthing.begin(); iter != sentthing.end(); ){

        // Second timeout //
        if((timems - (*iter)->RequestStartTime > PACKET_LOST_AFTER_MILLISECONDS) ||
            (LastConfirmedSent > (*iter)->PacketNumber + PACKET_LOST_AFTER_RECEIVED_NEWER))
        {
            // The current attempt is lost //
            if((*iter)->Resend == RECEIVE_GUARANTEE::ResendOnce){

                if(++(*iter)->AttemptNumber <= 2){

                    // Resend //
                    _Resend(guard, **iter);
                    ++iter;
                    continue;
                }

            } else if((*iter)->Resend == RECEIVE_GUARANTEE::Critical){

                // If a critical one fails the connection must be closed //
                if(++(*iter)->AttemptNumber > CRITICAL_PACKET_MAX_TRIES){

                    LOG_INFO("Connection: Lost critical packet too many times, "
                        "closing connection to: " + GenerateFormatedAddressString());

                    (*iter)->OnFinalized(false);
                    Owner->CloseConnection(*this);
                    return;
                }

                // Resend //
                _Resend(guard, **iter);
                ++iter;
                continue;
            }

            // Shouldn't be sent anymore //

            // Failed //
            (*iter)->OnFinalized(false);
            _FailPacketAcks((*iter)->PacketNumber);
            iter = sentthing.erase(iter);

        } else {

            ++iter;
        }
    }
}

DLLEXPORT void Connection::UpdateListening(){

    // Timeout stuff (if possible) //
    int64_t timems = Time::GetTimeMs64();

    // Check for connection close //
    if(timems > LastReceivedPacketTime + KEEPALIVE_TIME*1.5f){
        // We could timeout the connection //
        Logger::Get()->Info("Connection: timing out connection to " +
            GenerateFormatedAddressString());

        // Mark us as closing //
        Owner->CloseConnection(*this);
        return;
    }

    GUARD_LOCK();

    _HandleTimeouts(guard, timems, PendingRequests);

    _HandleTimeouts(guard, timems, ResponsesNeedingConfirmation);


    bool AcksCouldBeSent = false;

    // Check if we have acks that haven't been sent //
    for(auto iter = ReceivedRemotePackets.begin(); iter != ReceivedRemotePackets.end(); 
        ++iter)
    {
        if(iter->second == RECEIVED_STATE::StateReceived){

            // Waiting to be sent //
            AcksCouldBeSent = true;
            break;
        }
    }

    // Send keep alive packet if it has been a while //
    if(timems > LastSentPacketTime+KEEPALIVE_TIME){
        
        // Send a keep alive packet //
        SendKeepAlivePacket(guard);

    } else if(AcksCouldBeSent && timems > LastSentPacketTime+ACKKEEPALIVE){

        // Send some acks //
        SendKeepAlivePacket(guard);
    }

}
// ------------------------------------ //
DLLEXPORT void Connection::HandlePacket(sf::Packet &packet){
    
    // Handle incoming packet //

    // Header //
    uint16_t leviathanMagic = 0;
    packet >> leviathanMagic;
    
    uint32_t packetnumber = 0;
    packet >> packetnumber;

    if(!packet){

        Logger::Get()->Error("Received packet has invalid (header) format");
    }

    GUARD_LOCK();

    NetworkAckField otherreceivedpackages(packet);

    if(!packet){

        Logger::Get()->Error("Received packet has invalid format");
    }

#ifdef OUTPUT_PACKET_BITS

    LOG_WRITE("Received bits: \n" +
        Convert::HexDump(reinterpret_cast<const uint8_t*>(packet.getData()),
            packet.getDataSize()));

#endif // OUTPUT_PACKET_BITS

    // Messages //
    uint8_t messageCount = 0;
    
    if(!(packet >> messageCount)){

        Logger::Get()->Error("Received packet has invalid format, missing Message Count");
    }

    // Marks things as successfully sent //
    SetPacketsReceivedIfNotSet(guard, otherreceivedpackages);

    // Report the packet as received //
    ReceivedRemotePackets[packetnumber] = RECEIVED_STATE::StateReceived;

    for(int i = 0; i < messageCount; ++i){

        uint8_t messageType = 0;
        packet >> messageType;

        uint32_t messageNumber = 0;
        packet >> messageNumber;
        
        if(!packet){

            LOG_ERROR("Connection: received packet has an invalid message "
                "(some may have been processed already)");
            return;
        }

        // We can discard this here if this is message is already received //
        bool alreadyReceived = false;
        
        if(_IsAlreadyReceived(messageNumber)){

            alreadyReceived = true;
        }

        switch(messageType){
        case NORMAL_RESPONSE_TYPE:
        {
            _HandleResponsePacket(guard, packet, 
                alreadyReceived);

            break;
        }
        case NORMAL_REQUEST_TYPE:
        {
            _HandleRequestPacket(guard, packet, messageNumber, alreadyReceived);
            break;
        }
        default:
        {
            LOG_ERROR("Connection: received packet has unknown message type (" +
                Convert::ToString(messageType) + "(some may have been processed already)");
            return;
        }
            
        }
    }

    LastReceivedPacketTime = Time::GetTimeMs64();

    if(State == CONNECTION_STATE::NothingReceived){
    
        State = CONNECTION_STATE::Initial;
    }
}

DLLEXPORT void Leviathan::Connection::_HandleResponsePacket(Lock &guard, sf::Packet &packet, 
    bool alreadyreceived) 
{
    // Generate a response and pass to the interface //
    std::shared_ptr<NetworkResponse> response;
    try {

        response = NetworkResponse::LoadFromPacket(packet);

        if(!response)
            throw InvalidArgument("response is null");

    }
    catch (const InvalidArgument& e){

        LOG_ERROR("Connection: received an invalid response packet, exception: ");
        e.PrintToLog();
        return;
    }

    if(alreadyreceived){

        return;
    }

    // The response might have a corresponding request //
    auto possiblerequest = _GetPossibleRequestForResponse(guard, response);

    // Add the response to the request //
    if(possiblerequest)
        possiblerequest->GotResponse = response;

    if(_HandleInternalResponse(guard, response))
        return;

    if(RestrictType != CONNECTION_RESTRICTION::None){

        // Restrict mode checking //

        // Not allowed //
        _OnRestrictFail(static_cast<uint16_t>(response->GetType()));
        return;
    }

    // See if interface wants to drop it //
    if(!Owner->GetInterface()->PreHandleResponse(response, possiblerequest.get(), *this))
    {

        LOG_WARNING("Connection: dropping packet due to interface not accepting response");
        return;
    }

    if(!possiblerequest){

        // Just discard if it is an empty response //
        if(response->GetType() == NETWORK_RESPONSE_TYPE::None)
            return;

        // Handle the response only packet //
        Owner->GetInterface()->HandleResponseOnlyPacket(response, *this);
        
    } else {

        // Remove the request //
        for(auto iter = PendingRequests.begin(); iter != PendingRequests.end(); ++iter){

            if((*iter).get() == possiblerequest.get()){

                // Notify that the request is done /
                possiblerequest->OnFinalized(true);
                PendingRequests.erase(iter);
                break;
            }
        }
    }
}

DLLEXPORT void Leviathan::Connection::_HandleRequestPacket(Lock &guard, sf::Packet &packet,
    uint32_t packetnumber, bool alreadyreceived) 
{
    // Generate a request object and make the interface handle it //
    std::shared_ptr<NetworkRequest> request;
    try {

        request = NetworkRequest::LoadFromPacket(packet, packetnumber);

        if(!request)
            throw InvalidArgument("request is null");

    }
    catch (const InvalidArgument& e){

        LOG_ERROR("Connection: received an invalid request packet, exception: ");
        e.PrintToLog();
        return;
    }

    if(alreadyreceived){

        return;
    }

    if(_HandleInternalRequest(guard, request))
        return;

    if(RestrictType != CONNECTION_RESTRICTION::None){

        // Restrict mode checking //

        // We can possibly drop the connection or perform other extra tasks //
        if(RestrictType == CONNECTION_RESTRICTION::ReceiveRemoteConsole){
            // Check type //
            if(!Engine::Get()->GetRemoteConsole()->CanOpenNewConnection(
                Owner->GetConnection(this), request))
            {
                _OnRestrictFail(static_cast<uint16_t>(request->GetType()));
                return;
            }

            // Successfully opened, connection should now be safe as a
            // general purpose connection
            RestrictType = CONNECTION_RESTRICTION::None;
            return;
        }
    }

    try{
        
        Owner->GetInterface()->HandleRequestPacket(request, *this);
        
    } catch(const InvalidArgument &e){
        // We couldn't handle this packet //
        Logger::Get()->Error("Connection: couldn't handle request packet! :");
        e.PrintToLog();
    }
}


DLLEXPORT bool Leviathan::Connection::_HandleInternalRequest(Lock &guard, 
    std::shared_ptr<NetworkRequest> request) 
{
    switch (request->GetType()){
    case NETWORK_REQUEST_TYPE::Connect:
    {
        SendPacketToConnection(guard, std::make_shared<ResponseConnect>(
                request->GetIDForResponse()), 
            RECEIVE_GUARANTEE::Critical);

        return true;
    }
    case NETWORK_REQUEST_TYPE::Security:
    {
        // CONNECTION_STATE::Initial is allowed here because the client might send a security
        // request before sending us a response to our connect request
        if(State != CONNECTION_STATE::Connected && State != CONNECTION_STATE::Initial){

            return true;
        }

        // Security has been set up for this connection //
        SendPacketToConnection(guard, std::make_shared<ResponseSecurity>(
                request->GetIDForResponse(), CONNECTION_ENCRYPTION::None),
            RECEIVE_GUARANTEE::Critical);
        
        return true;
    }
    case NETWORK_REQUEST_TYPE::Authenticate:
    {
        // Client is not allowed to respond to this //
        if(Owner->GetNetworkType() == NETWORKED_TYPE::Client){

            LOG_ERROR("Connection: Client received NETWORK_REQUEST_TYPE::Authenticate");
            return true;
        }

        // Connection is now authenticated //
        State = CONNECTION_STATE::Authenticated;

        LOG_INFO("Connection: Authenticate request accepted from: " +
            GenerateFormatedAddressString());

        int32_t ConnectedPlayerID = 0;
        uint64_t token = 0;

        SendPacketToConnection(guard, std::make_shared<ResponseAuthenticate>(
                request->GetIDForResponse(), ConnectedPlayerID, token),
            RECEIVE_GUARANTEE::Critical);

        return true;
    }
    default:
        break;
    }

    // Eat up all the packets if not properly opened yet
    if(State != CONNECTION_STATE::Authenticated){

        LOG_WARNING("Connection: not yet properly open, ignoring packet from: " +
            GenerateFormatedAddressString());
        return true;
    }

    // Did nothing, normal handling continues //
    return false;
}

DLLEXPORT bool Leviathan::Connection::_HandleInternalResponse(Lock &guard, 
    std::shared_ptr<NetworkResponse> response) 
{
    switch (response->GetType()){
    case NETWORK_RESPONSE_TYPE::CloseConnection:
    {
        // Forced disconnect //
        State = CONNECTION_STATE::Closed;

        // Release will be called by NetworkHandler which sends a close connection packet
        return true;
    }
    case NETWORK_RESPONSE_TYPE::Connect:
    {
        // The first packet of this type that is in response to our initial request
        // moves this connection to Connected on our side
        if(State == CONNECTION_STATE::NothingReceived || State == CONNECTION_STATE::Initial){

            State = CONNECTION_STATE::Connected;

            // Client will do security setup here //
            // TODO: figure out how master server connections should work
            if(Owner->GetNetworkType() == NETWORKED_TYPE::Client){

                SendPacketToConnection(guard, std::make_shared<RequestSecurity>(
                        CONNECTION_ENCRYPTION::None),
                    RECEIVE_GUARANTEE::Critical);
            }
        }
        return true;
    }
    case NETWORK_RESPONSE_TYPE::Security:
    {
        if(State != CONNECTION_STATE::Connected){

            return true;
        }

        // Verify security type is what we wanted //
        auto* securityresponse = static_cast<ResponseSecurity*>(response.get());

        if(securityresponse->SecureType != CONNECTION_ENCRYPTION::None){

            LOG_ERROR("Connection: mismatch security, disconnecting");
            SendCloseConnectionPacket(guard);
            return true;
        }

        State = CONNECTION_STATE::Secured;

        // TODO: send an empty authentication request if this is a master server connection
        if(Owner->GetNetworkType() == NETWORKED_TYPE::Client){

            SendPacketToConnection(guard, std::make_shared<RequestAuthenticate>("player"),
                RECEIVE_GUARANTEE::Critical);
        }

        return true;
    }
    case NETWORK_RESPONSE_TYPE::Authenticate:
    {
        // Connection is now good to go //

        auto* authresponse = static_cast<ResponseAuthenticate*>(response.get());

        //authresponse->UserID;

        State = CONNECTION_STATE::Authenticated;

        LOG_INFO("Connection: Authenticate succeeded from: " +
            GenerateFormatedAddressString());

        return true;
    }
    default:
        break;
    }

    // Eat up all the packets if not properly opened yet
    if(State != CONNECTION_STATE::Authenticated){

        LOG_WARNING("Connection: not yet properly open, ignoring packet from: " + 
            GenerateFormatedAddressString());
        return true;
    }

    // Did nothing, normal handling continues //
    return false;
}
// ------------------------------------ //
void Leviathan::Connection::_PreparePacketHeaderForPacket(Lock &guard, uint32_t localpacketid,
    uint32_t* firstmessagenumber, size_t messagenumbercount,
    sf::Packet &tofill)
{
    LEVIATHAN_ASSERT(localpacketid > 0, "Trying to fill packet with packetid == 0");
    
    // See the doxygen page networkformat for the header format //
    
    // Type //
    tofill << LEVIATHAN_NORMAL_PACKET;
    
    // PKT ID //
    tofill << localpacketid;

    // Acks
    _FillHeaderAcks(guard, localpacketid, tofill);
        
    // Message count
    LEVIATHAN_ASSERT(messagenumbercount <= std::numeric_limits<uint8_t>::max(),
        "too many messages in _PreparePacketHeader");

    tofill << static_cast<uint8_t>(messagenumbercount);
}

void Connection::_FillHeaderAcks(Lock &guard, uint32_t localpacketid, sf::Packet &tofill){

    if(ReceivedRemotePackets.empty()){

        // Set first to be zero which assumes that no data follows
        tofill << uint32_t(0);

    } else {

        // First we need to determine which received packet to use as first value //
        FrontAcks = !FrontAcks;

        uint32_t firstselected = 0;
        uint32_t last = 0;
        uint8_t count = DEFAULT_ACKCOUNT;

        if(FrontAcks){

            for(auto iter = ReceivedRemotePackets.begin(); iter != ReceivedRemotePackets.end();
                ++iter)
            {
                if(iter->second == RECEIVED_STATE::StateReceived){

                    firstselected = iter->first;
                    break;
                }
            }
            
        } else {

            int backoffset = 0;
            
            for(auto iter = ReceivedRemotePackets.rbegin();
                iter != ReceivedRemotePackets.rend(); ++iter)
            {
                if(iter->second == RECEIVED_STATE::StateReceived){

                    if(backoffset >= DEFAULT_ACKCOUNT){

                        break;
                    }

                    ++backoffset;
                    firstselected = iter->first;
                }

                ++backoffset;
            }
        }

        if(firstselected != 0 && count != 0){

            // Create the ack field //
            auto tmpacks = std::make_shared<SentAcks>(localpacketid,
                std::make_shared<NetworkAckField>(firstselected, count, 
                    ReceivedRemotePackets));

            // Add to acks that actually matter if it has anything //
            if(tmpacks->AcksInThePacket->Acks.size() > 0)
                SentAckPackets.push_back(tmpacks);

            // Put into the packet //
            tmpacks->AcksInThePacket->AddDataToPacket(tofill);
            
        } else {

            // None were found //
            tofill << uint32_t(0);
        }
    }
}

// ------------------------------------ //
std::shared_ptr<SentRequest> Connection::_GetPossibleRequestForResponse(
    Lock &guard, const std::shared_ptr<NetworkResponse> &response)
{
    // Return if it doesn't have a proper matching expected response id //
    const auto lookingforid = response->GetResponseID();

    if(lookingforid == 0)
        return nullptr;

    for(auto& packet : PendingRequests){

        if(packet->MessageNumber == lookingforid){
            
            // Found matching object //
            return packet;
        }
    }

    // Nothing found //
    return nullptr;
}

DLLEXPORT void Connection::SetRestrictionMode(CONNECTION_RESTRICTION type){
    RestrictType = type;
}

DLLEXPORT bool Connection::IsTargetHostLocalhost(){
    // Check does the address match localhost //
    return TargetHost == sf::IpAddress::LocalHost;
}

DLLEXPORT std::string Connection::GenerateFormatedAddressString() const{
    return TargetHost.toString() + ":" + Convert::ToString(TargetPortNumber);
}
// ------------------------------------ //
DLLEXPORT void Connection::CalculateNetworkPing(int packets, int allowedfails,
    std::function<void(int, int)> onsucceeded, std::function<void(PING_FAIL_REASON,
        int)> onfailed)
{
    // Avoid dividing by zero here //
    if(packets == 0){

        ++packets;
        LOG_WARNING("Connection: avoided dividing by zero by increasing ping packet count by "
            "one");
    }
    
    // The finishing check task needs to store this. Using a smart
    // pointer avoids copying this around
    std::shared_ptr<std::vector<std::shared_ptr<SentNetworkThing>>> sentechos =
        std::make_shared<std::vector<std::shared_ptr<SentNetworkThing>>>();
    
    sentechos->reserve(packets);

    if(packets >= 100){

        Logger::Get()->Warning("Connection: trying to send loads of ping packets, sending "+
            Convert::ToString(packets)+" packets");
    }

    // Send the packet count of echo requests //
    for(int i = 0; i < packets; i++){
        
        // Create a suitable echo request This needs to be
        // regenerated for each loop as each need to have unique id
        // for responses to be registered properly
        auto cursent = SendPacketToConnection(std::make_shared<RequestNone>(
                NETWORK_REQUEST_TYPE::Echo), RECEIVE_GUARANTEE::Critical);
        cursent->SetAsTimed();

        sentechos->push_back(cursent);
    }

    ThreadingManager::Get()->QueueTask(new ConditionalTask(std::bind<void>([](
                    std::shared_ptr<std::vector<std::shared_ptr<SentNetworkThing>>> requests,
                    std::function<void(int, int)> onsucceeded,
                    std::function<void(PING_FAIL_REASON, int)> onfailed,
                    int allowedfails) -> void
        {
            int fails = 0;

            std::vector<int64_t> packagetimes;
            packagetimes.reserve(requests->size());
            
            // Collect the times //
            auto end = requests->end();
            for(auto iter = requests->begin(); iter != end; ++iter){

                if((*iter)->IsDone != SentNetworkThing::DONE_STATUS::DONE ||
                    (*iter)->ConfirmReceiveTime.load(std::memory_order_acquire) < 2)
                {
                    // This one has failed //
                    
                    fails++;
                    continue;
                }

                // Store the time //
                packagetimes.push_back((*iter)->ConfirmReceiveTime.load(
                    std::memory_order_acquire));
            }
            
            
            // Check has too many failed //
            if(fails > allowedfails){

                Logger::Get()->Warning("Connection: pinging failed due to too many lost "
                    "packets, lost: "+Convert::ToString(fails));
                
                onfailed(PING_FAIL_REASON::LossTooHigh, fails);
                return;
            }

            // Use some nice distribution math to get the ping //
            int finalping = 0;

            // The values shouldn't be able to be more than 1000 each so ints will
            // be able to hold all the values
            int64_t sum = 0;
            float averagesquared = 0.f;
            
            auto end2 = packagetimes.end();
            for(auto iter = packagetimes.begin(); iter != end2; ++iter){

                sum += *iter;
                averagesquared += (powf(static_cast<float>(*iter), 2));
            }
            
            float average = sum/static_cast<float>(packagetimes.size());

            averagesquared /= static_cast<float>(packagetimes.size());

            float standarddeviation = sqrtf(averagesquared-powf(average, 2));

            // End mathematics

            // Just one more calculation to get ping that represents average bad case in some way,
            // might require tweaking...
            
            finalping = static_cast<int>(average+(standarddeviation*0.7f));
            
            onsucceeded(finalping, fails);
            
            Logger::Get()->Info("Connection: pinging completed, ping: "+
                Convert::ToString(finalping));
            
        }, sentechos, onsucceeded, onfailed, allowedfails), std::bind<bool>([](
                std::shared_ptr<std::vector<std::shared_ptr<SentNetworkThing>>> requests) 
            -> bool
            {
                // Check if even one is still waiting //
                auto end = requests->end();
                for(auto iter = requests->begin(); iter != end; ++iter){
                    if(!(*iter)->IsFinalized())
                        return false;
                }

                // None are still waiting, good to go //
                return true;

            }, sentechos)));
    
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Connection::AddDataForResponseWithoutData(sf::Packet &packet,
    NETWORK_RESPONSE_TYPE type) 
{
    packet << false << static_cast<uint16_t>(type) << static_cast<uint32_t>(0);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Connection::SetPacketsReceivedIfNotSet(Lock &guard, 
    NetworkAckField &acks) 
{
    // We need to loop through all our acks and set them in the map //
    for (uint32_t i = 0; i < static_cast<uint32_t>(acks.Acks.size()); i++){

        for(uint8_t bit = 0; bit < 8; ++bit){
        
            const auto id = (i * 8) + bit + acks.FirstPacketID;

            if(acks.Acks[i] & (1 << bit)){

                HandleRemoteAck(guard, id);
            }
        }
    }
}


DLLEXPORT void Leviathan::Connection::RemoveSucceededAcks(Lock &guard, NetworkAckField &acks){

    // We need to loop through all our acks and erase them from the map (if set) //
    for (uint32_t i = 0; i < static_cast<uint32_t>(acks.Acks.size()); i++){

        for(uint8_t bit = 0; bit < 8; ++bit){
        
            const auto id = (i * 8) + bit + acks.FirstPacketID;

            if(acks.Acks[i] & (1 << bit)){

                ReceivedRemotePackets.erase(id);
            }
        }
    }
}

DLLEXPORT std::vector<uint32_t> Connection::GetCurrentlySentAcks(){

    std::vector<uint32_t> ids;

    for(const auto& acks : SentAckPackets){

        acks->AcksInThePacket->InvokeForEachAck([&](uint32_t id){

                ids.push_back(id);
            });
    }

    return ids;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::Connection::_SendPacketToSocket(
    sf::Packet &actualpackettosend)
{
    LEVIATHAN_ASSERT(Owner, "Connection no owner");

    // We have now sent a packet //
    LastSentPacketTime = Time::GetTimeMs64();

#ifdef OUTPUT_PACKET_BITS

    LOG_WRITE("Packet bits: \n" + 
        Convert::HexDump(reinterpret_cast<const uint8_t*>(actualpackettosend.getData()), 
            actualpackettosend.getDataSize()));

#endif // OUTPUT_PACKET_BITS

    auto guard(Owner->LockSocketForUse());
    Owner->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
}
// ------------------------------------ //
bool Connection::_IsAlreadyReceived(uint32_t packetid){

    // It is moved through in reverse to quickly return matches,
    // but receiving the same packet twice isn't that common
    for(auto id : LastReceivedMessageNumbers){

        if(id == packetid){

            // Found a match, this is an already received packet //
            return true;
        }
    }
    
    // Not found, add for future searches //
    LastReceivedMessageNumbers.push_back(packetid);

    // It wasn't there //
    return false;
}



DLLEXPORT void Leviathan::Connection::_FailPacketAcks(uint32_t packetid){

    for (auto iter = SentAckPackets.begin(); iter != SentAckPackets.end(); ++iter){

        if((*iter)->InsidePacket == packetid){

            SentAckPackets.erase(iter);
            return;
        }
    }
}

DLLEXPORT void Leviathan::Connection::_OnRestrictFail(uint16_t type){

    LOG_ERROR("Connection: received a non-valid packet "
        "in restrict mode(" + Convert::ToString(static_cast<int>(RestrictType)) + ") type: " +
        Convert::ToString(static_cast<int>(type)) + " from: " + 
        GenerateFormatedAddressString());

    Owner->CloseConnection(*this);
}

// ------------------ NetworkAckField ------------------ //
DLLEXPORT Leviathan::NetworkAckField::NetworkAckField(uint32_t firstpacketid, 
    uint8_t maxacks, PacketReceiveStatus &copyfrom) :
    FirstPacketID(firstpacketid)
{

    // Id is 0 nothing should be copied //
    if(FirstPacketID == 0)
        return;

    // Before we reach the first packet id we will want to skip stuff //
    bool foundstart = false;

    for (auto iter = copyfrom.begin(); iter != copyfrom.end(); ++iter){

        // Skip current if not received //
        if(iter->second == RECEIVED_STATE::NotReceived)
            continue;

        if(!foundstart){

            if(iter->first >= FirstPacketID){


                // Adjust the starting index, if it is an exact match for firstpacketid
                // this should be 0
                auto startindex = iter->first - FirstPacketID;

                // Check that the index is within the size of the field, otherwise fail
                if(startindex >= maxacks)
                    break;

                foundstart = true;

            } else {

                continue;
            }

        }

        size_t currentindex = iter->first - FirstPacketID;

        if(currentindex >= maxacks)
            break;

        // Copy current ack //
        const auto vecelement = currentindex / 8;

        while (Acks.size() <= vecelement){

            Acks.push_back(0);
        }

        Acks[vecelement] |= (1 << (currentindex % 8));

        // Stop copying once enough acks have been set. The loop can end before this, but
        // that just leaves the rest of the acks as 0
        if(currentindex + 1 >= maxacks)
            break;
    }

    if(!foundstart){

        // No acks to send //
        FirstPacketID = 0;
        return;
    }
}

DLLEXPORT NetworkAckField::NetworkAckField(sf::Packet &packet){

    // Get data //
    packet >> FirstPacketID;

    // Empty ack fields //
    if(FirstPacketID == 0)
        return;
    
    uint8_t tmpsize = 0;
    
    packet >> tmpsize;

    if(!packet)
        return;
    
    // Fill in the acks from the packet //
    Acks.resize(tmpsize);
    
    for(char i = 0; i < tmpsize; i++){
        packet >> Acks[i];
    }
}

DLLEXPORT void NetworkAckField::AddDataToPacket(sf::Packet &packet){

    packet << FirstPacketID;
    
    if(FirstPacketID == 0)
        return;

    uint8_t tmpsize = static_cast<uint8_t>(Acks.size());
    packet << tmpsize;
    
    // fill in the ack data //
    for(uint8_t i = 0; i < tmpsize; i++){
        
        packet << Acks[i];
    }
}

DLLEXPORT void NetworkAckField::InvokeForEachAck(std::function<void (uint32_t)> func){

    for(size_t i = 0; i < Acks.size(); ++i){

        for(uint8_t bit = 0; bit < 8; ++bit){
        
            const auto id = (i * 8) + bit + FirstPacketID;

            if(Acks[i] & (1 << bit))
                func(id);
        }
    }
}

