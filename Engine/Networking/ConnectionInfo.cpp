// ------------------------------------ //
#include "ConnectionInfo.h"

#include "NetworkResponse.h"
#include "NetworkRequest.h"
#include "Iterators/StringIterator.h"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "NetworkHandler.h"
#include "Exceptions.h"
#include "RemoteConsole.h"
#include "Threading/ThreadingManager.h"
#include "../TimeIncludes.h"
#include "../Utility/Convert.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

//! Makes the program spam a ton of debug info about packets //
#define SPAM_ME_SOME_PACKETS 1

// ------------------------------------ //
DLLEXPORT Leviathan::ConnectionInfo::ConnectionInfo(const string &hostname) : 
    RestrictType(CONNECTION_RESTRICTION_NONE),
    LastSentPacketTime(0), LastReceivedPacketTime(0),
    LastReceivedPacketIDs(KEEP_IDS_FOR_DISCARD),
    HostName(hostname), AddressGot(false)
{
	// We need to split the port number from the address //
	StringIterator itr(hostname);

	auto result = itr.GetUntilNextCharacterOrAll<string>(':');

	HostName = *result;

	// We should be fine not skipping a letter //
	result = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_NONE);

	if(!result){
		// Probably should get the default port number //
		Logger::Get()->Warning("ConnectionInfo: no port defined, using default, "
            "hostname: "+hostname);
		TargetPortNumber = 80;
		return;
	}
    
	TargetPortNumber = Convert::StringTo<int>(*result.get());
}

DLLEXPORT Leviathan::ConnectionInfo::ConnectionInfo(const sf::IpAddress &targetaddress,
    unsigned short port) :
    RestrictType(CONNECTION_RESTRICTION_NONE),
    LastSentPacketTime(0), LastReceivedPacketTime(0),
    LastReceivedPacketIDs(KEEP_IDS_FOR_DISCARD),
    TargetPortNumber(port), TargetHost(targetaddress), AddressGot(true)
{

}

DLLEXPORT Leviathan::ConnectionInfo::~ConnectionInfo(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConnectionInfo::Init(){
    // Lock the Network handler //
    GUARD_LOCK_OTHER_NAME(NetworkHandler::Get(), guard2);
    
	GUARD_LOCK();
    
	// This might do something //
	if(!AddressGot){
		TargetHost = sf::IpAddress(HostName);
	}

	// We fail if we got an invalid address //
	if(TargetHost == sf::IpAddress::None){

		Logger::Get()->Error("ConnectionInfo: Init: couldn't translate host name to a real address, host: "+HostName);
		return false;
	}

	// Register us //
	NetworkHandler::Get()->_RegisterConnectionInfo(this);

	Logger::Get()->Info("ConnectionInfo: opening connection to host on "+
        TargetHost.toString()+":"+Convert::ToString(TargetPortNumber));

    // Reset various timers //
    LastSentPacketTime = LastReceivedPacketTime = Time::GetTimeMs64();
    
	return true;
}

DLLEXPORT void Leviathan::ConnectionInfo::Release(){

	// Remove us from the queue //
	NetworkHandler::Get()->_UnregisterConnectionInfo(this);

    GUARD_LOCK();

    Logger::Get()->Info("ConnectionInfo: disconnecting from "+TargetHost.toString()+":"
        +Convert::ToString(TargetPortNumber));

    // Send a close packet //
    SendCloseConnectionPacket(guard);

    // Destroy some of our stuff //
    TargetHost == sf::IpAddress::None;
        
    // Make sure that all our remaining packets fail //
    auto end = WaitingRequests.end();
    for(auto iter = WaitingRequests.begin(); iter != end; ++iter){

        // Mark as failed //
        (*iter)->SetWaitStatus(false);
    }

    // All are now properly closed //
    WaitingRequests.clear();

    // Release all the listeners //
    ReleaseChildHooks(guard);
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<NetworkResponse> Leviathan::ConnectionInfo::SendRequestAndBlockUntilDone(
    std::shared_ptr<NetworkRequest> request, int maxtries /*= 2*/)
{
	// Send the request //
	shared_ptr<SentNetworkThing> sentrequest = SendPacketToConnection(request, maxtries);

	// Now we wait //
    sentrequest->GetStatus();

	return sentrequest->GotResponse;
}

DLLEXPORT std::shared_ptr<SentNetworkThing> Leviathan::ConnectionInfo::SendPacketToConnection(
    Lock &guard, std::shared_ptr<NetworkRequest> request, int maxretries)
{
	// Generate a packet from the request //
	sf::Packet actualpackettosend;
    
	// We need a complete header with acks and stuff //
	_PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, actualpackettosend, true);

	// Generate packet object for the request //
	sf::Packet requestsdata = request->GeneratePacketForRequest();

	// Add the data to the actual packet //
	actualpackettosend.append(requestsdata.getData(), requestsdata.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info("PacketSpam: Sending request packet ("+Convert::ToString(request->GetExpectedResponseID())+
        ", id "+Convert::ToString(LastUsedLocalID)+") to "+TargetHost.toString()+
        ":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
	}

	// Add to the sent packets //
	shared_ptr<SentNetworkThing> tmprequestinfo(
        new SentNetworkThing(LastUsedLocalID, request->GetExpectedResponseID(), request, 
            maxretries, request->GetTimeOutType(),
            request->GetTimeOutValue(), requestsdata, 1));

	WaitingRequests.push_back(tmprequestinfo);

	return tmprequestinfo;
}


DLLEXPORT std::shared_ptr<SentNetworkThing> Leviathan::ConnectionInfo::SendPacketToConnection(
    Lock &guard, shared_ptr<NetworkResponse> response, int maxtries)
{
	// Generate a packet from the request //
	sf::Packet actualpackettosend;
    
	// We need a complete header with acks and stuff //
	_PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, actualpackettosend, false);

	// Generate packet object for the request //
	sf::Packet requestsdata = response->GeneratePacketForResponse();

	// Add the data to the actual packet //
	actualpackettosend.append(requestsdata.getData(), requestsdata.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info("PacketSpam: Sending response packet ("+Convert::ToString(response->GetResponseID())+", id "+
		Convert::ToString(LastUsedLocalID)+") to "
		+(TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
	}

	// Add to the sent packets //
	shared_ptr<SentNetworkThing> tmprequestinfo(
        new SentNetworkThing(LastUsedLocalID, response, 
            maxtries, response->GetTimeOutType(), response->GetTimeOutValue(), requestsdata, 1));

	WaitingRequests.push_back(tmprequestinfo);

	return tmprequestinfo;
}
// ------------------------------------ //
std::shared_ptr<SentNetworkThing> ConnectionInfo::CreateFullSendablePacket(
    std::shared_ptr<NetworkResponse> data, sf::Packet &packettofill, bool skipwaitingrequests)
{
    GUARD_LOCK();
    
    // We need a complete header with acks and stuff //
    _PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, packettofill, false);

    // Generate packet object for the request //
    sf::Packet responsedata = data->GeneratePacketForResponse();

    // Add the data to the actual packet //
    packettofill.append(responsedata.getData(), responsedata.getDataSize());

    // Add to the sent packets //
    shared_ptr<SentNetworkThing> tmprequestinfo(
        new SentNetworkThing(LastUsedLocalID, data, 
            1, data->GetTimeOutType(), data->GetTimeOutValue(), responsedata, 0));

    if(!skipwaitingrequests)
        WaitingRequests.push_back(tmprequestinfo);

    return tmprequestinfo;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ConnectionInfo::SendKeepAlivePacket(Lock &guard){
    
	// Generate a packet from the request //
	sf::Packet actualpackettosend;
    
	// We need a complete header with acks and stuff //
	_PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, actualpackettosend, false, true);

	// Generate packet object for the request //
	auto response = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 100);
	response->GenerateKeepAliveResponse();
	sf::Packet requestsdata = response->GeneratePacketForResponse();

	// Add the data to the actual packet //
	actualpackettosend.append(requestsdata.getData(), requestsdata.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info("PacketSpam: Sending keepalive packet (id "+Convert::ToString(LastUsedLocalID)+") to "
		+(TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
	}

	// Add to the sent packets //
	shared_ptr<SentNetworkThing> tmprequestinfo(
        new SentNetworkThing(LastUsedLocalID, response, 1, response->GetTimeOutType(),
            response->GetTimeOutValue(), requestsdata, 1));

	WaitingRequests.push_back(tmprequestinfo);
}

DLLEXPORT void Leviathan::ConnectionInfo::SendCloseConnectionPacket(Lock &guard){

	// Generate a packet from the request //
	sf::Packet actualpackettosend;
    
	// We need a complete header with acks and stuff //
	_PreparePacketHeaderForPacket(guard, ++LastUsedLocalID, actualpackettosend, false);

	// Generate packet object for the request //
	auto response = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 100);
	response->GenerateCloseConnectionResponse();
	sf::Packet requestsdata = response->GeneratePacketForResponse();

	// Add the data to the actual packet //
	actualpackettosend.append(requestsdata.getData(), requestsdata.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info("PacketSpam: Sending close connection packet (id "+Convert::ToString(LastUsedLocalID)+") to "
		+(TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
	}
}
// ------------------------------------ //
void Leviathan::ConnectionInfo::_ResendRequest(shared_ptr<SentNetworkThing> toresend, Lock &guard){

	// Generate a packet from the request //
	sf::Packet tosend;

	_PreparePacketHeaderForPacket(guard, toresend->PacketNumber, tosend, toresend->IsArequest);

	// Add the packet data //
	tosend.append(toresend->AlmostCompleteData.getData(), toresend->AlmostCompleteData.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info("PacketSpam: resending packet ("+Convert::ToString(toresend->PacketNumber)+") to "
		+(TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(tosend, TargetHost, TargetPortNumber);
	}

	// Reset the time //
	toresend->RequestStartTime = Time::GetTimeMs64();
}
// ------------------------------------ //
DLLEXPORT inline void ConnectionInfo::HandleRemoteAck(Lock &guard, int oursentconfirmed){

    if(oursentconfirmed > LastConfirmedSent)
        LastConfirmedSent = oursentconfirmed;

    for(auto iter = WaitingRequests.begin(); iter != WaitingRequests.end(); ++iter){

        if((*iter)->IsArequest)
            continue;

        if(oursentconfirmed == (*iter)->PacketNumber){

#ifdef SPAM_ME_SOME_PACKETS
            Logger::Get()->Info("PacketSpam: non-request packet successfully sent ("+
                Convert::ToString((*iter)->PacketNumber)+") to " +
                (TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

            if((*iter)->ConfirmReceiveTime.load(memory_order_consume) == 1){

                (*iter)->ConfirmReceiveTime.store(Time::GetTimeMs64(),
                    memory_order_release);
            }

            // We want to notify all waiters that it has been received //
            (*iter)->SetWaitStatus(true);

            WaitingRequests.erase(iter);
            break;
        }
    }

    // Check which ack packets have been received //
	for(size_t i = 0; i < SentAckPackets.size(); i++){

        if(oursentconfirmed == SentAckPackets[i]->InsidePacket){
                
#ifdef SPAM_ME_SOME_PACKETS
            Logger::Get()->Info("PacketSpam: acks successfully sent (first id "+
                Convert::ToString(SentAckPackets[i]->AcksInThePacket->FirstPacketID)+") to "
                +(TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
                
            // Mark as properly sent //
            SentAckPackets[i]->Received = true;

            // Erase from the main ack send map //
            SentAckPackets[i]->AcksInThePacket->RemoveSucceededAcks(*this);

            SentAckPackets.erase(SentAckPackets.begin()+i);
            break;
        }
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ConnectionInfo::UpdateListening(Lock &guard){

	// Timeout stuff (if possible) //
	int64_t timems = Time::GetTimeMs64();

	for(auto iter = WaitingRequests.begin(); iter != WaitingRequests.end(); ){
        
		// Check for resend //
		if((*iter)->TimeOutMS != -1){
			// Check based on different styles //
			switch((*iter)->PacketTimeoutStyle){
			case PACKET_TIMEOUT_STYLE_TIMEDMS:
				{
					if(timems - (*iter)->RequestStartTime > (*iter)->TimeOutMS){

            movepacketsendattemptonexttry:

						// Move to next try //
						if(++(*iter)->AttempNumber > (*iter)->MaxTries && (*iter)->MaxTries > 0){

#ifdef SPAM_ME_SOME_PACKETS
                            Logger::Get()->Info("PacketSpam: packet reached maximum tries ("+
                                Convert::ToString((*iter)->PacketNumber)+") to " +
                                (TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
                            
							// We want to notify all waiters that it failed //
                            (*iter)->SetWaitStatus(true);
							iter = WaitingRequests.erase(iter);
							continue;
						}
                        
						// Resend request //
						_ResendRequest((*iter), guard);
					}
				}
				break;
			case PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED:
				{
					if(LastConfirmedSent > (*iter)->PacketNumber + (*iter)->TimeOutMS){

						goto movepacketsendattemptonexttry;
					}
				}
				break;
			}
		}

		// Move to next //
		++iter;
	}

	bool AcksCouldBeSent = false;

	// Check if we have unpacketed acks //
    for(auto iter = ReceivedRemotePackets.begin(); iter != ReceivedRemotePackets.end(); ++iter){

        if(iter->second == RECEIVED_STATE_RECEIVED){

            // Waiting to be sent //
            AcksCouldBeSent = true;
            break;
        }
    }

    // TODO: timeout ack packets
    // set AcksCouldBeSent if acks fail
    // but only if the same acks aren't sent in another packet
    
    
	// Send keep alive packet if it has been a while //
	if(timems > LastSentPacketTime+KEEPALIVE_TIME){
        
		// Send a keep alive packet //
		Logger::Get()->Info("ConnectionInfo: sending keepalive packet (because"+
            Convert::ToString(timems-LastSentPacketTime)+" since last sent has elapsed) to "
			+TargetHost.toString()+":"+Convert::ToString(TargetPortNumber));
        
		SendKeepAlivePacket(guard);

	} else if(AcksCouldBeSent && timems > LastSentPacketTime+ACKKEEPALIVE){
		// Send some acks //

#ifdef SPAM_ME_SOME_PACKETS
        Logger::Get()->Info("PacketSpam: acks can be sent and some time has elapsed, to " +
            (TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

		shared_ptr<NetworkResponse> emptyresponse(new NetworkResponse(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000));
		emptyresponse->GenerateEmptyResponse();

		SendPacketToConnection(guard, emptyresponse, 1);
	}

	// Check for connection close //
	if(timems > LastSentPacketTime+KEEPALIVE_TIME/4.f && timems > LastReceivedPacketTime+KEEPALIVE_TIME*1.5f){
		// We could timeout the connection //
		Logger::Get()->Info("ConnectionInfo: could timeout connection to "+
            TargetHost.toString()+":"+Convert::ToString(TargetPortNumber));

		if(NetworkHandler::GetInterface()->CanConnectionTerminate(this)){
			// Mark us as closing //
			NetworkHandler::Get()->SafelyCloseConnectionTo(this);
		}

	} else if(timems > LastSentPacketTime+KEEPALIVE_TIME/1.1f && !HasReceived){


		Logger::Get()->Info("ConnectionInfo: timing out connection (nothing received) "+
            TargetHost.toString()+":"+Convert::ToString(TargetPortNumber));
		// Mark us as closing //
		NetworkHandler::Get()->SafelyCloseConnectionTo(this);
	}

}
// ------------------------------------ //
DLLEXPORT void Leviathan::ConnectionInfo::CheckKeepAliveSend(){
	GUARD_LOCK();
    
	// Check is a keepalive reasonable to send (don't want to end up spamming them between the instances) //
	auto timenow = Time::GetTimeMs64();
	if(timenow > LastSentPacketTime+KEEPALIVE_RESPOND){
		// Respond to it //
		Logger::Get()->Info("ConnectionInfo: replying to a keepalive packet (because "+
            Convert::ToString(timenow-LastSentPacketTime)+" since last sent has elapsed) to "
			+TargetHost.toString()+":"+Convert::ToString(TargetPortNumber));
		SendKeepAlivePacket(guard);
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConnectionInfo::IsThisYours(sf::IpAddress &sender,
    unsigned short &sentport)
{
	// Check for matching sender with our target //
	if(sentport != TargetPortNumber || sender != TargetHost){
        
		// Not mine //
		return false;
	}
    
	// It is mine //
    return true;
}

DLLEXPORT void Leviathan::ConnectionInfo::HandlePacket(sf::Packet &packet){
    
    // Handle incoming packet //
	int packetnumber = 0;

	if(!(packet >> packetnumber)){

		Logger::Get()->Error("Received package has invalid format");
	}

	// We can discard this here if this is already received //
	if(_IsAlreadyReceived(packetnumber)){

		// Ignore repeat packet //
		return;
	}

	NetworkAckField otherreceivedpackages(packet);

	if(!packet){

		Logger::Get()->Error("Received package has invalid format");
	}

	bool isrequest = false;

	if(!(packet >> isrequest)){

		Logger::Get()->Error("Received package has invalid format");
	}
	// Rest of the data is now the actual thing //

	// Mark as received a packet //
    {
        GUARD_LOCK();
        
        LastReceivedPacketTime = Time::GetTimeMs64();
        HasReceived = true;

    }

	bool ShouldNotBeMarkedAsReceived = false;

	// Handle the packet (hopefully the handling function queues it or something to not stall execution) //
	if(isrequest){
		// Generate a request object and make the interface handle it //
		shared_ptr<NetworkRequest> request(new NetworkRequest(packet));

        {
            
            GUARD_LOCK();
        
            // Restrict mode checking //
            if(RestrictType != CONNECTION_RESTRICTION_NONE){
                // We can possibly drop the connection or perform other extra tasks //
                if(RestrictType == CONNECTION_RESTRICTION_RECEIVEREMOTECONSOLE){
                    // Check type //
                    if(RemoteConsole::Get()->CanOpenNewConnection(this, request)){
                        // Successfully opened, connection should now be safe as a
                        // general purpose connection
                        RestrictType = CONNECTION_RESTRICTION_NONE;
                        goto connectioninfoafterprocesslabel;

                    } else {
                        // We want to close //
                        Logger::Get()->Error("ConnectionInfo: received a non-valid packet "
                            "to receive remote console connection socket, "+
                            TargetHost.toString()+":"+
                            Convert::ToString(TargetPortNumber));
                    
                        NetworkHandler::Get()->SafelyCloseConnectionTo(this);
                        return;
                    }
                }
            }
        }

#ifdef SPAM_ME_SOME_PACKETS
		Logger::Get()->Info("PacketSpam: received request ("+Convert::ToString(packetnumber)+", request"+
			Convert::ToString(request->GetExpectedResponseID())+") from "+
            TargetHost.toString()+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
		try{
			NetworkHandler::GetInterface()->HandleRequestPacket(request, this);
		} catch(const InvalidArgument &e){
			// We couldn't handle this packet //
			Logger::Get()->Error("ConnectionInfo: couldn't handle request packet! :");
			e.PrintToLog();
		}

	} else {
		// Generate a response and pass to the interface //
		shared_ptr<NetworkResponse> response(new NetworkResponse(packet));

		// The response might have a corresponding request //
        GUARD_LOCK_NAME(lockit);
        
		shared_ptr<SentNetworkThing> possiblerequest = _GetPossibleRequestForResponse(lockit,
            response);

		// Restrict mode checking //
		if(RestrictType != CONNECTION_RESTRICTION_NONE){
			// We can possibly drop the connection or perform other extra tasks //
			if(RestrictType == CONNECTION_RESTRICTION_RECEIVEREMOTECONSOLE){
				// We want to close //
				Logger::Get()->Error("ConnectionInfo: received a response packet to receive remote console "
                    "connection socket, "+TargetHost.toString()+":"+Convert::ToString(
                        TargetPortNumber));
                lockit.unlock();
				NetworkHandler::Get()->SafelyCloseConnectionTo(this);
				return;
			}
		}

		// See if interface wants to drop it //
        lockit.unlock();
		if(!NetworkHandler::GetInterface()->PreHandleResponse(response, possiblerequest ?
                possiblerequest->OriginalRequest: NULL, this))
        {

			Logger::Get()->Warning("ConnectionInfo: dropping packet due to interface not accepting response");
			return;
		}

		if(possiblerequest){
			// Add to the request //
			possiblerequest->GotResponse = response;

            lockit.lock();

            // Remove the request //
            
            auto end = WaitingRequests.end();
            for(auto iter = WaitingRequests.begin(); iter != end; ++iter){

                if((*iter).get() == possiblerequest.get()){

                    WaitingRequests.erase(iter);
                    break;
                }
            }

            lockit.unlock();

#ifdef SPAM_ME_SOME_PACKETS
            Logger::Get()->Info("PacketSpam: received response to request ("+
                Convert::ToString(possiblerequest->PacketNumber)+", response "+
                Convert::ToString(packetnumber)+") from "+
                TargetHost.toString()+":"+Convert::ToString(TargetPortNumber));
#endif 

            if(possiblerequest->ConfirmReceiveTime.load(memory_order_consume) == 1){

                possiblerequest->ConfirmReceiveTime.store(Time::GetTimeMs64(),
                    memory_order_release);
            }
            
            // Notify that the request is done /
            possiblerequest->SetWaitStatus(true);

		} else {

			// Just discard if it is an empty response //
			if(response->GetType() != NETWORKRESPONSETYPE_NONE){

				// Handle the response only packet //
				NetworkHandler::GetInterface()->HandleResponseOnlyPacket(response, this, ShouldNotBeMarkedAsReceived);
			}
		}
	}

connectioninfoafterprocesslabel:
    
    // Handle resends based on ack field //
    otherreceivedpackages.SetPacketsReceivedIfNotSet(*this);
        
    // We possibly do not want to create an ack for this packet //

    if(!ShouldNotBeMarkedAsReceived){

        GUARD_LOCK();

        // Report the packet as received //
        ReceivedRemotePackets[packetnumber] = RECEIVED_STATE_RECEIVED;
    }

	return;
}
// ------------------------------------ //
void Leviathan::ConnectionInfo::_PreparePacketHeaderForPacket(Lock &guard,
    int packetid, sf::Packet &tofill, bool isrequest, bool dontsendacks /*= false*/)
{
	// First thing is the packet number //
	tofill << packetid;

	// Now the hard part, creating the ack table //

	// Actually this can be skipped if we can send a ack packet again //

	// We have now made a new packet //
	LastSentPacketTime = Time::GetTimeMs64();


	if(dontsendacks || ReceivedRemotePackets.empty()){

		tofill << sf::Int32(-1) << sf::Int8(0);

	} else {

        // First we need to determine which received packet to use as first value //
        FrontAcks = !FrontAcks;

        int firstselected = -1;
        int last = -1;
        int count = 0;

        // Number of acks that could be sent
        int couldbesent = 0;
        
        if(FrontAcks){

            for(auto iter = ReceivedRemotePackets.begin(); iter != ReceivedRemotePackets.end();
                ++iter)
            {

                if(iter->second == RECEIVED_STATE_RECEIVED){

                    firstselected = iter->first;
                    
                    ++couldbesent;
                }
            }

            if(firstselected == -1){

                goto copyacksfromtheendlabel;
            }

            const int maxcount = ReceivedRemotePackets.size() - firstselected;

            if(maxcount < DEFAULT_ACKCOUNT){

                count = maxcount;
                
            } else {

                
            }
            
            
            
            
        } else {

            for(auto iter = ReceivedRemotePackets.begin(); iter != ReceivedRemotePackets.end();
                ++iter)
            {

                if(iter->second == RECEIVED_STATE_RECEIVED){

                    ++couldbesent;
                }
            }

copyacksfromtheendlabel:

            firstselected = ReceivedRemotePackets.rbegin()->first;

            count = DEFAULT_ACKCOUNT;

            if(couldbesent >= DEFAULT_ACKCOUNT){

                count *= 2;
            }

            last = firstselected - count >= 0 ? firstselected - count: 0;
            
            std::swap(firstselected, last);
        }

        // Create the ack field //
        shared_ptr<SentAcks> tmpacks(new SentAcks(packetid,
                make_shared<NetworkAckField>(firstselected, count, 
                    ReceivedRemotePackets)));

        // Add to acks that actually matter if it has anything //
        if(tmpacks->AcksInThePacket->Acks.size() > 0)
            SentAckPackets.push_back(tmpacks);

#ifdef SPAM_ME_SOME_PACKETS
        Logger::Get()->Info("PacketSpam: sending new acks (first id "+Convert::ToString(
				tmpacks->AcksInThePacket->FirstPacketID)+", count "+
            Convert::ToString(count)+") to "
            +(TargetHost.toString())+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

        // Put into the packet //
        tmpacks->AcksInThePacket->AddDataToPacket(tofill);
	}

	// Mark as request, if it is one //
	tofill << isrequest;
}

shared_ptr<SentNetworkThing> Leviathan::ConnectionInfo::_GetPossibleRequestForResponse(
    Lock &guard, shared_ptr<NetworkResponse> response)
{
	// Return if it doesn't have a proper matching expected response id //
	const int lookingforid = response->GetResponseID();

	if(lookingforid == -1)
		return NULL;

	for(auto iter = WaitingRequests.begin(); iter != WaitingRequests.end(); ++iter){

		if((*iter)->ExpectedResponseID == lookingforid){
#ifdef SPAM_ME_SOME_PACKETS
			Logger::Get()->Info("PacketSpam: matching response to request ("+
                Convert::ToString(response->GetResponseID())+") from "
				+TargetHost.toString()+":"+Convert::ToString(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
            
			// Found matching object //
			return *iter;
		}
	}

	// Nothing found //
	return NULL;
}

DLLEXPORT void Leviathan::ConnectionInfo::SetRestrictionMode(CONNECTION_RESTRICTION type){
	RestrictType = type;
}

DLLEXPORT bool Leviathan::ConnectionInfo::IsTargetHostLocalhost(){
	// Check does the address match localhost //
	return TargetHost == sf::IpAddress::LocalHost;
}

DLLEXPORT string Leviathan::ConnectionInfo::GenerateFormatedAddressString() const{
	return TargetHost.toString()+":"+Convert::ToString(TargetPortNumber);
}
// ------------------------------------ //
DLLEXPORT void ConnectionInfo::CalculateNetworkPing(int packets, int allowedfails,
    std::function<void(int, int)> onsucceeded, std::function<void(CONNECTION_PING_FAIL_REASON,
        int)> onfailed)
{
    // Avoid dividing by zero here //
    if(packets == 0){

        Logger::Get()->Error("ConnectionInfo: avoided dividing by zero by dropping a "
            "ping request");
        return;
    }
    
    // The finishing check task needs to store this. Using a smart pointer avoids copying this around
    std::shared_ptr<std::vector<shared_ptr<SentNetworkThing>>> sentechos = make_shared<
        std::vector<shared_ptr<SentNetworkThing>>>();
    
    sentechos->reserve(packets);

    if(packets >= 100){

        Logger::Get()->Warning("ConnectionInfo: trying to send loads of ping packets, sending "+
            Convert::ToString(packets)+" packets");
    }

    // Send the packet count of echo requests //
    for(int i = 0; i < packets; i++){
        
        // Create a suitable echo request //
        // This needs to be regenerated for each loop as each need to have unique id for responses
        // to be registered properly
        std::shared_ptr<NetworkRequest> echorequest =
            make_shared<NetworkRequest>(NETWORKREQUESTTYPE_ECHO, 1000,
            PACKET_TIMEOUT_STYLE_TIMEDMS);

        
        // Locked in here to allow the connection do stuff in between //
        GUARD_LOCK();
    
        // TODO: check is the connection still open
        
        auto cursent = SendPacketToConnection(guard, echorequest, 1);
        cursent->SetAsTimed();

        sentechos->push_back(cursent);
    }



    ThreadingManager::Get()->QueueTask(new ConditionalTask(std::bind<void>([](
                    std::shared_ptr<std::vector<shared_ptr<SentNetworkThing>>> requests,
                    std::function<void(int, int)> onsucceeded,
                    std::function<void(CONNECTION_PING_FAIL_REASON, int)> onfailed,
                    int allowedfails) -> void
        {
            int fails = 0;

            std::vector<int> packagetimes;
            packagetimes.reserve(requests->size());
            
            // Collect the times //
            auto end = requests->end();
            for(auto iter = requests->begin(); iter != end; ++iter){

                if(!(*iter)->Succeeded ||
                    (*iter)->ConfirmReceiveTime.load(memory_order_acquire) < 2)
                {
                    // This one has failed //
                    
                    fails++;
                    continue;
                }

                // Store the time //
                packagetimes.push_back((*iter)->ConfirmReceiveTime.load(memory_order_acquire) -
                    (*iter)->RequestStartTime);
            }
            
            
            // Check has too many failed //
            if(fails > allowedfails){

                Logger::Get()->Warning("ConnectionInfo: pinging failed due to too many lost "
                    "packets, lost: "+Convert::ToString(fails));
                
                onfailed(CONNECTION_PING_FAIL_REASON_LOSS_TOO_HIGH, fails);
                return;
            }

            // Use some nice distribution math to get the ping //
            int finalping = 0;

            // The values shouldn't be able to be more than 1000 each so ints will
            // be able to hold all the values
            int sum = 0;
            float averagesquared = 0.f;
            
            auto end2 = packagetimes.end();
            for(auto iter = packagetimes.begin(); iter != end2; ++iter){

                sum += *iter;
                averagesquared += powf(*iter, 2);
            }
            
            float average = sum/static_cast<float>(packagetimes.size());

            averagesquared /= static_cast<float>(packagetimes.size());

            float standarddeviation = sqrtf(averagesquared-powf(average, 2));

            // End mathematics

            // Just one more calculation to get ping that represents average bad case in some way,
            // might require tweaking...
            
            finalping = static_cast<int>(average+(standarddeviation*0.7f));
            
            onsucceeded(finalping, fails);
            
            Logger::Get()->Info("ConnectionInfo: pinging completed, ping: "+
                Convert::ToString(finalping));
            
        }, sentechos, onsucceeded, onfailed, allowedfails), std::bind<bool>([](
                std::shared_ptr<std::vector<shared_ptr<SentNetworkThing>>> requests) -> bool
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
bool Leviathan::ConnectionInfo::_IsAlreadyReceived(int packetid){
    GUARD_LOCK();

	// It is moved through in reverse to quickly return matches,
    // but receiving the same packet twice isn't that common
    for(auto id : LastReceivedPacketIDs){

        if(id == packetid){

            // Found a match, this is an already received packet //
			return true;
        }
    }
    
	// Not found, add for future searches //
	LastReceivedPacketIDs.push_back(packetid);

	// It wasn't there //
	return false;
}
// ------------------ SentNetworkThing ------------------ //
Leviathan::SentNetworkThing::SentNetworkThing(int packetid, int expectedresponseid,
    std::shared_ptr<NetworkRequest> request, int maxtries, PACKET_TIMEOUT_STYLE howtotimeout,
    int timeoutvalue, const sf::Packet &packetsdata, int attempnumber /*= 1*/) :
    PacketNumber(packetid), 
    MaxTries(maxtries), 
    AttempNumber(attempnumber),
    PacketTimeoutStyle(howtotimeout), TimeOutMS(timeoutvalue),
    RequestStartTime(Time::GetTimeMs64()), ExpectedResponseID(expectedresponseid),
    AlmostCompleteData(packetsdata), IsArequest(true),  OriginalRequest(request)
{

}

Leviathan::SentNetworkThing::SentNetworkThing(int packetid,
    std::shared_ptr<NetworkResponse> response, int maxtries, PACKET_TIMEOUT_STYLE howtotimeout,
    int timeoutvalue, const sf::Packet &packetsdata, int attempnumber /*= 1*/) :
    PacketNumber(packetid), 
    MaxTries(maxtries), 
    AttempNumber(attempnumber),
    PacketTimeoutStyle(howtotimeout), TimeOutMS(timeoutvalue), 
    RequestStartTime(Time::GetTimeMs64()), ExpectedResponseID(-1), 
	AlmostCompleteData(packetsdata), IsArequest(false), SentResponse(response)
{

}

DLLEXPORT Leviathan::SentNetworkThing::~SentNetworkThing(){

}

DLLEXPORT void Leviathan::SentNetworkThing::SetWaitStatus(bool status){
    
    Succeeded = status;
    IsDone.store(true, memory_order_release);

    {
        Lock lock(CallbackMutex);
        
        if(Callback)
            (*Callback)(status, *this);
    }
    
    {
        Lock lock(NotifyMutex);
        Notifier.notify_all();
    }
}

DLLEXPORT bool SentNetworkThing::GetStatus(){

    while(!IsDone.load(memory_order_acquire)){

        Lock lock(NotifyMutex);
        // TODO: make sure that this cannot deadlock //
        Notifier.wait(lock);
    }

    return Succeeded;
}

DLLEXPORT void Leviathan::SentNetworkThing::SetAsTimed(){
    
    ConfirmReceiveTime.store(1, memory_order_release);
}

DLLEXPORT void Leviathan::SentNetworkThing::SetCallback(std::shared_ptr<CallbackType> func)
{
    Lock lock(CallbackMutex);
    Callback = func;
}
// ------------------ NetworkAckField ------------------ //
DLLEXPORT NetworkAckField::NetworkAckField(sf::Packet &packet){

    // Get data //
	packet >> FirstPacketID;
    
	sf::Int8 tmpsize = 0;
    
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

	// First set the trivial data //
	sf::Int8 tmpsize = Acks.size();

	packet << FirstPacketID << tmpsize;
    
	// Now to fill in the ack data //
	for(char i = 0; i < tmpsize; i++){
        
		packet << Acks[i];
	}
}
// ------------------------------------ //
Leviathan::NetworkAckField::NetworkAckField(sf::Int32 firstpacketid, char maxacks,
    ReceivedPacketField &copyfrom) :
    FirstPacketID(firstpacketid)
{

	// Id is -1 nothing should be copied //
	if(FirstPacketID == -1)
		return;

	auto iter = copyfrom.begin();

	for(; iter != copyfrom.end(); ++iter){
		if(iter->first >= firstpacketid){
			break;
		}
	}

	if(iter == copyfrom.end()){
		// No start ack found //
		FirstPacketID = -1;
		return;
	}

	// Reserve space for data //
	Acks.reserve(maxacks/8);

	// Start filling it //
	for(char i = 0; i < maxacks; i++){

		int curcheckacknumber = firstpacketid+i;

		size_t vecelement = i/8;

		while(Acks.size() <= vecelement){

			Acks.push_back(0);
		}

		// If reached the end just leave the zeroes //
		if(iter == copyfrom.end()){
			break;
		}

		if(iter->first == curcheckacknumber){

setcurrentnumbertoackfield:
            
			// Mark it as copied to a ack field //
			iter->second = RECEIVED_STATE_ACKS_SENT;
            
			// Set it //
			Acks[vecelement] |= (1 << (i-vecelement));
            
			++iter;

		} else if(iter->first > curcheckacknumber){
			// We are missing these packets so let them be zeroed out //

		} else {
			// Advance iterator to reach this //
			for(; iter != copyfrom.end(); ++iter){
                
				if(iter->first == curcheckacknumber){
                    
					goto setcurrentnumbertoackfield;
                    
				} else if(iter->first > curcheckacknumber){

                    break;
                }
			}
		}
	}

	// We need to reset FirstPacketID if no acks set //
	if(Acks.size() == 0)
		FirstPacketID = -1;
}

DLLEXPORT void Leviathan::NetworkAckField::SetPacketsReceivedIfNotSet(ConnectionInfo &receiver){

    GUARD_LOCK_OTHER(receiver);
    
	// We need to loop through all our acks and set them in the map //
	for(size_t i = 0; i < Acks.size(); i++){
        
		// Loop the individual bits //
		for(int bit = 0; bit < 8; bit++){

			// Check is it set //
			if(Acks[i] & (1 << bit)){
                
				// Set as received //
                receiver.HandleRemoteAck(guard, FirstPacketID + i*8 + bit);
			}
		}
	}
}

DLLEXPORT void NetworkAckField::RemoveSucceededAcks(ConnectionInfo &target){

    GUARD_LOCK_OTHER(target);
    
	// We need to loop through all our acks and erase them from the map (if set) //
	for(size_t i = 0; i < Acks.size(); i++){
		// Loop the individual bits //
		for(int bit = 0; bit < 8; bit++){

			// Check is it set //
			if(Acks[i] & (1 << bit)){

                // Remove it //
                target.ReceivedRemotePackets.erase(FirstPacketID + i*8 + bit);
			}
		}
	}
}
// ------------------ SentAcks ------------------ //
Leviathan::SentAcks::SentAcks(int packet, shared_ptr<NetworkAckField> data) :
    InsidePacket(packet), AcksInThePacket(data)
{

}


