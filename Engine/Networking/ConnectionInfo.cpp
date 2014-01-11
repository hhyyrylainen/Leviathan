#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CONNECTIONINFO
#include "ConnectionInfo.h"
#endif
#include "Utility/Iterators/WstringIterator.h"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "NetworkHandler.h"
#include "Exceptions/ExceptionInvalidArgument.h"
#include "RemoteConsole.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ConnectionInfo::ConnectionInfo(shared_ptr<wstring> hostname) : HostName(hostname), AddressGot(false), LastUsedID(-1), 
	LastSentConfirmID(-1), MaxAckReduntancy(1), MyLastSentReceived(-1), LastReceivedPacketTime(0), RestrictType(CONNECTION_RESTRICTION_NONE)
{
	// We need to split the port number from the address //
	WstringIterator itr(hostname.get(), false);

	auto result = itr.GetUntilNextCharacterOrAll(L':');

	HostName = shared_ptr<wstring>(result.release());

	// We should be fine not skipping a letter //
	result = itr.GetNextNumber(DECIMALSEPARATORTYPE_NONE);

	TargetPortNumber = Convert::WstringToInt(*result.get());
}

DLLEXPORT Leviathan::ConnectionInfo::ConnectionInfo(const sf::IpAddress &targetaddress, USHORT port) : HostName(nullptr), AddressGot(true), 
	TargetPortNumber(port), TargetHost(targetaddress), LastUsedID(-1), LastSentConfirmID(-1), MaxAckReduntancy(1), MyLastSentReceived(-1),
	LastReceivedPacketTime(0), RestrictType(CONNECTION_RESTRICTION_NONE)
{

}

DLLEXPORT Leviathan::ConnectionInfo::~ConnectionInfo(){

}
// ------------------ Packet extensions ------------------ //
sf::Packet& operator <<(sf::Packet& packet, const NetworkAckField &data){
	// First set the trivial data //
	sf::Int8 tmpsize = data.Acks.size();

	packet << data.FirstPacketID << tmpsize;
	// Now to fill in the ack data //
	for(char i = 0; i < tmpsize; i++){
		packet << data.Acks[i];
	}

	return packet;
}

sf::Packet& operator >>(sf::Packet& packet, NetworkAckField &data){
	// Get data //
	packet >> data.FirstPacketID;
	sf::Int8 tmpsize = 0;
	packet >> tmpsize;
	// Fill in the acks from the packet //
	data.Acks.resize(tmpsize);
	for(char i = 0; i < tmpsize; i++){
		packet >> data.Acks[i];
	}

	return packet;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConnectionInfo::Init(){
	ObjectLock guard(*this);
	// This might do something //
	if(!AddressGot){
		TargetHost = sf::IpAddress(Convert::WstringToString(*HostName.get()));
	}
	// Register us //
	NetworkHandler::Get()->_RegisterConnectionInfo(this);

	Logger::Get()->Info(L"ConnectionInfo: opening connection to host on "+Convert::StringToWstring(TargetHost.toString())+L" to port "
		+Convert::ToWstring(TargetPortNumber));

	return true;
}

DLLEXPORT void Leviathan::ConnectionInfo::Release(){
	ObjectLock guard(*this);
	// Remove us from the queue //
	NetworkHandler::Get()->_UnregisterConnectionInfo(this);

	Logger::Get()->Info(L"ConnectionInfo: disconnecting from "+Convert::StringToWstring(TargetHost.toString())+L" on port "
		+Convert::ToWstring(TargetPortNumber));

	// Send a close packet //
	SendCloseConnectionPacket(guard);
}
// ------------------------------------ //
DLLEXPORT shared_ptr<NetworkResponse> Leviathan::ConnectionInfo::SendRequestAndBlockUntilDone(shared_ptr<NetworkRequest> request, int maxtries /*= 2*/){
	// Send the request //
	shared_ptr<SentNetworkThing> sentrequest = SendPacketToConnection(request, maxtries);

	// Now we wait //
	sentrequest->WaitForMe->get_future().get();

	return sentrequest->GotResponse;
}

DLLEXPORT shared_ptr<SentNetworkThing> Leviathan::ConnectionInfo::SendPacketToConnection(shared_ptr<NetworkRequest> request, int maxretries){
	ObjectLock guard(*this);
	// Generate a packet from the request //
	sf::Packet actualpackettosend;
	// We need a complete header with acks and stuff //
	_PreparePacketHeaderForPacket(++LastUsedID, actualpackettosend, true);

	// Generate packet object for the request //
	sf::Packet requestsdata = request->GeneratePacketForRequest();

	// Add the data to the actual packet //
	actualpackettosend.append(requestsdata.getData(), requestsdata.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info(L"PacketSpam: Sending request packet ("+Convert::ToWstring(request->GetExpectedResponseID())+L", id"+
		Convert::ToWstring(LastUsedID)+L") to "
		+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
	}

	// Add to the sent packets //
	shared_ptr<SentNetworkThing> tmprequestinfo(new SentNetworkThing(LastUsedID, request->GetExpectedResponseID(), request, shared_ptr<boost::promise<bool>>(new 
		boost::promise<bool>()), maxretries, request->GetTimeOutType(), request->GetTimeOutValue(), requestsdata, 1));

	WaitingRequests.push_back(tmprequestinfo);

	return tmprequestinfo;
}


DLLEXPORT shared_ptr<SentNetworkThing> Leviathan::ConnectionInfo::SendPacketToConnection(shared_ptr<NetworkResponse> response, int maxtries){
	ObjectLock guard(*this);
	// Generate a packet from the request //
	sf::Packet actualpackettosend;
	// We need a complete header with acks and stuff //
	_PreparePacketHeaderForPacket(++LastUsedID, actualpackettosend, false);

	// Generate packet object for the request //
	sf::Packet requestsdata = response->GeneratePacketForResponse();

	// Add the data to the actual packet //
	actualpackettosend.append(requestsdata.getData(), requestsdata.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info(L"PacketSpam: Sending response packet ("+Convert::ToWstring(response->GetResponseID())+L", id"+
		Convert::ToWstring(LastUsedID)+L") to "
		+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
	}

	// Add to the sent packets //
	shared_ptr<SentNetworkThing> tmprequestinfo(new SentNetworkThing(LastUsedID, response, shared_ptr<boost::promise<bool>>(new 
		boost::promise<bool>()), maxtries, response->GetTimeOutType(), response->GetTimeOutValue(), requestsdata, 1));

	WaitingRequests.push_back(tmprequestinfo);

	return tmprequestinfo;
}

DLLEXPORT void Leviathan::ConnectionInfo::SendKeepAlivePacket(ObjectLock &guard){
	VerifyLock(guard);
	// Generate a packet from the request //
	sf::Packet actualpackettosend;
	// We need a complete header with acks and stuff //
	_PreparePacketHeaderForPacket(++LastUsedID, actualpackettosend, false);

	// Generate packet object for the request //
	shared_ptr<NetworkResponse> response(new NetworkResponse(-1, PACKAGE_TIMEOUT_STYLE_TIMEDMS, 100));
	response->GenerateKeepAliveResponse();
	sf::Packet requestsdata = response->GeneratePacketForResponse();

	// Add the data to the actual packet //
	actualpackettosend.append(requestsdata.getData(), requestsdata.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info(L"PacketSpam: Sending keepalive packet (id "+Convert::ToWstring(LastUsedID)+L") to "
		+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
	}

	// Add to the sent packets //
	shared_ptr<SentNetworkThing> tmprequestinfo(new SentNetworkThing(LastUsedID, response, shared_ptr<boost::promise<bool>>(new 
		boost::promise<bool>()), 1, response->GetTimeOutType(), response->GetTimeOutValue(), requestsdata, 1));

	WaitingRequests.push_back(tmprequestinfo);
}


DLLEXPORT void Leviathan::ConnectionInfo::SendCloseConnectionPacket(ObjectLock &guard){
	VerifyLock(guard);
	// Generate a packet from the request //
	sf::Packet actualpackettosend;
	// We need a complete header with acks and stuff //
	_PreparePacketHeaderForPacket(++LastUsedID, actualpackettosend, false);

	// Generate packet object for the request //
	shared_ptr<NetworkResponse> response(new NetworkResponse(-1, PACKAGE_TIMEOUT_STYLE_TIMEDMS, 100));
	response->GenerateCloseConnectionResponse();
	sf::Packet requestsdata = response->GeneratePacketForResponse();

	// Add the data to the actual packet //
	actualpackettosend.append(requestsdata.getData(), requestsdata.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info(L"PacketSpam: Sending close connection packet (id "+Convert::ToWstring(LastUsedID)+L") to "
		+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(actualpackettosend, TargetHost, TargetPortNumber);
	}
}
// ------------------------------------ //
void Leviathan::ConnectionInfo::_ResendRequest(shared_ptr<SentNetworkThing> toresend, ObjectLock &guard){
	VerifyLock(guard);
	// Generate a packet from the request //
	sf::Packet tosend;

	_PreparePacketHeaderForPacket(toresend->PacketNumber, tosend, toresend->IsArequest);

	// Add the packet data //
	tosend.append(toresend->AlmostCompleteData.getData(), toresend->AlmostCompleteData.getDataSize());

#ifdef SPAM_ME_SOME_PACKETS
	Logger::Get()->Info(L"PacketSpam: resending packet ("+Convert::ToWstring(toresend->PacketNumber)+L") to "
		+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

	// We need to use the handlers socket //
	{
		auto guard(NetworkHandler::Get()->LockSocketForUse());
		NetworkHandler::Get()->_Socket.send(tosend, TargetHost, TargetPortNumber);
	}

	// Reset the time //
	toresend->RequestStartTime = Misc::GetTimeMs64();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ConnectionInfo::UpdateListening(){

	// Timeout stuff (if possible) //
	__int64 timems = Misc::GetTimeMs64();

	ObjectLock guard(*this);

	for(auto iter = WaitingRequests.begin(); iter != WaitingRequests.end(); ){
		// Check is it already here //
		auto itergot = SentPacketsConfirmedAsReceived.find((*iter)->PacketNumber);


		if(itergot != SentPacketsConfirmedAsReceived.end() && itergot->second){
			// It is here! //
			if(!(*iter)->IsArequest){
				// It is now done! //
#ifdef SPAM_ME_SOME_PACKETS
				Logger::Get()->Info(L"PacketSpam: non-request packet successfully sent ("+Convert::ToWstring((*iter)->PacketNumber)+L") to "
					+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
				// We want to notify all waiters that it has been received //
				(*iter)->WaitForMe->set_value(true);
				iter = WaitingRequests.erase(iter);
				continue;
			}
			// Check is the response received //
			if((*iter)->GotResponse){
#ifdef SPAM_ME_SOME_PACKETS
				Logger::Get()->Info(L"PacketSpam: request packet successfully sent and got response ("+Convert::ToWstring((*iter)->PacketNumber)+L", "+
					Convert::ToWstring((*iter)->ExpectedResponseID)+L") from "+Convert::StringToWstring(TargetHost.toString())+L":"
					+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
				// It has a proper response //
				// We want to notify all waiters that it has been received //
				(*iter)->WaitForMe->set_value(true);
				iter = WaitingRequests.erase(iter);
				continue;
			}
		}


		// Check for resend //
		if((*iter)->TimeOutMS != -1){
			// Check based on different styles //
			switch((*iter)->PacketTimeoutStyle){
			case PACKAGE_TIMEOUT_STYLE_TIMEDMS:
				{
					if(timems-(*iter)->RequestStartTime > (*iter)->TimeOutMS){
movepacketsendattemptonexttry:

						// Move to next try //
						if(++(*iter)->AttempNumber > (*iter)->MaxTries && (*iter)->MaxTries > 0){

							Logger::Get()->Warning(L"ConnectionInfo: packet reached maximum tries");
							// We want to notify all waiters that it failed //
							(*iter)->WaitForMe->set_value(false);
							iter = WaitingRequests.erase(iter);
							continue;
						}
						// Resend request //
						_ResendRequest((*iter), guard);
					}
				}
				break;
			case PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED:
				{
					if((*iter)->PacketNumber+(*iter)->TimeOutMS < SentPacketsConfirmedAsReceived.rbegin()->first){

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

	// Check which ack packets have been received //
	// Mark acks as successfully sent //
	for(size_t i = 0; i < AcksNotConfirmedAsReceived.size(); i++){

		auto iter = SentPacketsConfirmedAsReceived.find(AcksNotConfirmedAsReceived[i]->InsidePacket);


		if(iter != SentPacketsConfirmedAsReceived.end() && iter->second){
#ifdef SPAM_ME_SOME_PACKETS
			Logger::Get()->Info(L"PacketSpam: acks successfully sent (first id "+Convert::ToWstring(AcksNotConfirmedAsReceived[i]->AcksInThePacket->FirstPacketID)+L") to "
				+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
			// Mark as properly sent //
			AcksNotConfirmedAsReceived[i]->Received = true;

			// Erase from the main ack send map //
			AcksNotConfirmedAsReceived[i]->AcksInThePacket->RemoveMatchingPacketIDsFromMap(ReceivedPacketsNotifiedAsReceivedByUs);
		}
	}


	// Verify current ack number //
	// We only want to go back if there are ack objects that haven't made it to the receiver //
	bool FoundSent = false;
	for(int i = AcksNotConfirmedAsReceived.size()-1; i > -1; i--){
		// If we have successfully sent earlier ack packets we can set send count to 0 to resend them //
		if(FoundSent){
			if(AcksNotConfirmedAsReceived[i]->Received){
				AcksNotConfirmedAsReceived.erase(AcksNotConfirmedAsReceived.begin()+i);
				continue;
			}
			// This needs resending //
			AcksNotConfirmedAsReceived[i]->SendCount = 0;
			AcksCouldBeSent = true;
#ifdef SPAM_ME_SOME_PACKETS
			Logger::Get()->Info(L"PacketSpam: acks missing (resending) (first id "+Convert::ToWstring(AcksNotConfirmedAsReceived[i]->AcksInThePacket->FirstPacketID)+L") to "
				+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

		} else {
			// Check if we have sent this new packet //
			if(AcksNotConfirmedAsReceived[i]->Received){
				FoundSent = true;
				AcksNotConfirmedAsReceived.erase(AcksNotConfirmedAsReceived.begin()+i);
			}
		}
	}

	// Check if we have unpacketed acks //
	for(auto iter = ReceivedPacketsNotifiedAsReceivedByUs.begin(); iter != ReceivedPacketsNotifiedAsReceivedByUs.end(); ++iter){
		if(!iter->second){
			// Ack is not in any acket field //
			AcksCouldBeSent = true;
			break;
		}

	}


	// TODO: send keep alive packet if it has been a while //
	if(timems > LastSentPacketTime+KEEPALIVE_TIME){
		// Send a keep alive packet //
		Logger::Get()->Info(L"ConnectionInfo: sending keepalive packet (because"+Convert::ToWstring(timems-LastSentPacketTime)+L" since last sent has elapsed) to "
			+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
		SendKeepAlivePacket();

	} else if(AcksCouldBeSent && timems > LastSentPacketTime+ACKKEEPALIVE){
		// Send some acks //
		Logger::Get()->Info(L"ConnectionInfo: sending keepalive packet (because"+Convert::ToWstring(timems-LastSentPacketTime)+L" passed and acks await) to "
			+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
		SendKeepAlivePacket();

	}

	// Check for connection close //
	if(timems > LastSentPacketTime+KEEPALIVE_TIME/4.f && timems > LastReceivedPacketTime+KEEPALIVE_TIME){
		// We could timeout the connection //
		Logger::Get()->Info(L"ConnectionInfo: could timeout connection to "+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));

		if(NetworkHandler::GetInterface()->CanConnectionTerminate(this)){
			// Mark us as closing //
			NetworkHandler::Get()->SafelyCloseConnectionTo(this);
		}

	}

}
// ------------------------------------ //
DLLEXPORT void Leviathan::ConnectionInfo::CheckKeepAliveSend(){
	ObjectLock guard(*this);
	// Check is a keepalive reasonable to send (don't want to end up spamming them between the instances) //
	auto timenow = Misc::GetTimeMs64();
	if(timenow > LastSentPacketTime+KEEPALIVE_RESPOND){
		// Respond to it //
		Logger::Get()->Info(L"ConnectionInfo: replying to a keepalive packet (because "+Convert::ToWstring(timenow-LastSentPacketTime)+L" since last sent has elapsed) to "
			+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
		SendKeepAlivePacket();
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ConnectionInfo::IsThisYours(sf::Packet &packet, sf::IpAddress &sender, USHORT &sentport){
	// Check for matching sender with our target //
	if(sentport != TargetPortNumber || sender != TargetHost){
		// Not mine //
		return false;
	}
	// It is mine //
	Logger::Get()->Info(L"It is mine, in fact");

	// Handle incoming packet //
	int packetnumber = 0;

	if(!(packet >> packetnumber)){

		Logger::Get()->Error(L"Received package has invalid format");
	}

	NetworkAckField otherreceivedpackages;

	if(!(packet >> otherreceivedpackages)){

		Logger::Get()->Error(L"Received package has invalid format");
	}

	bool isrequest = false;

	if(!(packet >> isrequest)){

		Logger::Get()->Error(L"Received package has invalid format");
	}
	// Rest of the data is now the actual thing //

	// Mark as received a packet //
	LastReceivedPacketTime = Misc::GetTimeMs64();

	bool ShouldNotBeMarkedAsReceived = false;

	// Handle the packet (hopefully the handling function queues it or something to not stall execution) //
	if(isrequest){
		// Generate a request object and make the interface handle it //
		shared_ptr<NetworkRequest> request(new NetworkRequest(packet));

		// Restrict mode checking //
		if(RestrictType != CONNECTION_RESTRICTION_NONE){
			// We can possibly drop the connection or perform other extra tasks //
			if(RestrictType == CONNECTION_RESTRICTION_RECEIVEREMOTECONSOLE){
				// Check type //
				if(RemoteConsole::Get()->CanOpenNewConnection(this, request)){
					// Successfully opened, connection should now be safe as a general purpose connection //
					RestrictType = CONNECTION_RESTRICTION_NONE;
				} else {
					// We want to close //
					Logger::Get()->Error(L"ConnectionInfo: received a non-valid packet to receive remote console connection socket, "
						+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
					NetworkHandler::Get()->SafelyCloseConnectionTo(this);
					return true;
				}
			}
		}

#ifdef SPAM_ME_SOME_PACKETS
		Logger::Get()->Info(L"PacketSpam: received request ("+Convert::ToWstring(packetnumber)+L", request"+
			Convert::ToWstring(request->GetExpectedResponseID())+L") from "+Convert::StringToWstring(TargetHost.toString())+L":"
			+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
		try{
			NetworkHandler::GetInterface()->HandleRequestPacket(request, this);
		} catch(const ExceptionInvalidArgument &e){
			// We couldn't handle this packet //
			Logger::Get()->Error(L"ConnectionInfo: couldn't handle request packet! :");
			e.PrintToLog();
		}

	} else {
		// Generate a response and pass to the interface //
		shared_ptr<NetworkResponse> response(new NetworkResponse(packet));

		// The response might have a corresponding request //
		ObjectLock guard(*this);
		shared_ptr<SentNetworkThing> possiblerequest = _GetPossibleRequestForResponse(response);

		// Restrict mode checking //
		if(RestrictType != CONNECTION_RESTRICTION_NONE){
			// We can possibly drop the connection or perform other extra tasks //
			if(RestrictType == CONNECTION_RESTRICTION_RECEIVEREMOTECONSOLE){
				// We want to close //
				Logger::Get()->Error(L"ConnectionInfo: received a response packet to receive remote console connection socket, "
					+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
				NetworkHandler::Get()->SafelyCloseConnectionTo(this);
				return true;
			}
		}

		// See if interface wants to drop it //
		if(!NetworkHandler::GetInterface()->PreHandleResponse(response, possiblerequest ? possiblerequest->OriginalRequest: NULL, this)){

			Logger::Get()->Warning(L"ConnectionInfo: dropping packet due to interface not accepting response");
			return true;
		}

#ifdef SPAM_ME_SOME_PACKETS
		Logger::Get()->Info(L"PacketSpam: received response ("+Convert::ToWstring(packetnumber)+L", response"+
			Convert::ToWstring(response->GetResponseID())+L") to "+Convert::StringToWstring(TargetHost.toString())+L":"
			+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

		if(possiblerequest){
			// Add to the request //
			possiblerequest->GotResponse = response;

			// Notify all that it succeeded will be done by the update loop //


		} else {
			// Handle the response only packet //
			NetworkHandler::GetInterface()->HandleResponseOnlyPacket(response, this, ShouldNotBeMarkedAsReceived);
		}
	}

	{
		ObjectLock guard(*this);

		// Handle resends based on ack field //
		otherreceivedpackages.SetPacketsReceivedIfNotSet(SentPacketsConfirmedAsReceived);

		// We possibly do not want to create an ack for this packet //

		if(!ShouldNotBeMarkedAsReceived){
			// Report the packet as received //
			_VerifyAckPacketsAsSuccesfullyReceivedFromHost(packetnumber);
		}
	}


	return true;
}
// ------------------------------------ //
void Leviathan::ConnectionInfo::_VerifyAckPacketsAsSuccesfullyReceivedFromHost(int packetreceived){
	// Mark it as received //
	ObjectLock guard(*this);

	// Only set if we haven't already set it //
	auto iter = ReceivedPacketsNotifiedAsReceivedByUs.find(packetreceived);

	if(iter == ReceivedPacketsNotifiedAsReceivedByUs.end()){
#ifdef SPAM_ME_SOME_PACKETS
		Logger::Get()->Info(L"PacketSpam: packet is now marked as received ("+Convert::ToWstring(packetreceived)+L") from "
			+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
		// We set this to false to keep track if we have told the sender that we got this packet (then it is true and can be removed from the map) //
		ReceivedPacketsNotifiedAsReceivedByUs[packetreceived] = false;
	}
}

void Leviathan::ConnectionInfo::_PreparePacketHeaderForPacket(int packetid, sf::Packet &tofill, bool isrequest){
	// First thing is the packet number //
	tofill << packetid;

	// Now the hard part, creating the ack table //

	// Actually this can be skipped if we can send a ack packet again //
	ObjectLock guard(*this);

	// We have now made a new packet //
	LastSentPacketTime = Misc::GetTimeMs64();

	bool newrequired = true;

	for(size_t i = 0; i < AcksNotConfirmedAsReceived.size(); i++){
		if(AcksNotConfirmedAsReceived[i]->SendCount < MaxAckReduntancy){

			AcksNotConfirmedAsReceived[i]->SendCount++;

			tofill << *AcksNotConfirmedAsReceived[i]->AcksInThePacket;

			newrequired = false;
			break;
		}

	}

	if(newrequired){
		// First we need to determine which received packet to use as first value //

		if(ReceivedPacketsNotifiedAsReceivedByUs.size() > 0){

			bool foundstartval = ReceivedPacketsNotifiedAsReceivedByUs.find(LastSentConfirmID) != ReceivedPacketsNotifiedAsReceivedByUs.end();
			int lastval = ReceivedPacketsNotifiedAsReceivedByUs.rbegin()->first;

			if(LastSentConfirmID != -1 && foundstartval && (LastSentConfirmID+DEFAULT_ACKCOUNT <= lastval || 
				ReceivedPacketsNotifiedAsReceivedByUs.size() < (size_t)(1.5f*DEFAULT_ACKCOUNT)) && (LastSentConfirmID+4 <= lastval || 
				ReceivedPacketsNotifiedAsReceivedByUs.size() < (size_t)(6)))
			{
				// Current last sent is ok //

			} else {
				// Go back to beginning //
				LastSentConfirmID = ReceivedPacketsNotifiedAsReceivedByUs.begin()->first;
			}
		}

		// Create the ack field //
		shared_ptr<SentAcks> tmpacks(new SentAcks(packetid, new NetworkAckField(LastSentConfirmID, DEFAULT_ACKCOUNT, 
			ReceivedPacketsNotifiedAsReceivedByUs)));

		// Add to acks that actually matter if it has anything //
		if(tmpacks->AcksInThePacket->Acks.size() > 0)
			AcksNotConfirmedAsReceived.push_back(tmpacks);

#ifdef SPAM_ME_SOME_PACKETS
		Logger::Get()->Info(L"PacketSpam: sending new acks (first id "+Convert::ToWstring(
			tmpacks->AcksInThePacket->FirstPacketID)+L", count "+
			Convert::ToWstring(tmpacks->AcksInThePacket->Acks.size())+L") to "
			+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS

		// Put into the packet //
		tofill << *tmpacks->AcksInThePacket;

		// We increment the last send, just for fun and to see what happens //
		if(LastSentConfirmID != -1)
			LastSentConfirmID++;
	}

	// Mark as request, if it is one //
	tofill << isrequest;
}

shared_ptr<SentNetworkThing> Leviathan::ConnectionInfo::_GetPossibleRequestForResponse(shared_ptr<NetworkResponse> response){
	// Return if it doesn't have a proper matching expected response id //
	int lookingforid = response->GetResponseID();

	if(lookingforid == -1)
		return NULL;

	ObjectLock guard(*this);

	for(auto iter = WaitingRequests.begin(); iter != WaitingRequests.end(); ++iter){

		if((*iter)->ExpectedResponseID == lookingforid){
#ifdef SPAM_ME_SOME_PACKETS
			Logger::Get()->Info(L"PacketSpam: matching response to request ("+Convert::ToWstring(response->GetResponseID())+L") from "
				+Convert::StringToWstring(TargetHost.toString())+L":"+Convert::ToWstring(TargetPortNumber));
#endif // SPAM_ME_SOME_PACKETS
			// Found matching object //
			return *iter;
		}
	}

	// Nothing found //
	return NULL;
}

DLLEXPORT bool Leviathan::ConnectionInfo::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	throw std::exception();
}

DLLEXPORT void Leviathan::ConnectionInfo::SetRestrictionMode(CONNECTION_RESTRICTION type){
	RestrictType = type;
}

DLLEXPORT bool Leviathan::ConnectionInfo::IsTargetHostLocalhost(){
	// Check does the address match localhost //
	return TargetHost == sf::IpAddress::LocalHost;
}
// ------------------ SentNetworkThing ------------------ //
Leviathan::SentNetworkThing::SentNetworkThing(int packetid, int expectedresponseid, shared_ptr<NetworkRequest> request, shared_ptr<boost::promise<bool>> waitobject, 
	int maxtries, PACKET_TIMEOUT_STYLE howtotimeout, int timeoutvalue, const sf::Packet &packetsdata, int attempnumber /*= 1*/) : PacketNumber(packetid),
	ExpectedResponseID(expectedresponseid), OriginalRequest(request), WaitForMe(waitobject), MaxTries(maxtries), PacketTimeoutStyle(howtotimeout),
	TimeOutMS(timeoutvalue), AlmostCompleteData(packetsdata), AttempNumber(attempnumber), RequestStartTime(Misc::GetTimeMs64()), ConfirmReceiveTime(0),
	IsArequest(true)
{

}

Leviathan::SentNetworkThing::SentNetworkThing(int packetid, shared_ptr<NetworkResponse> response, shared_ptr<boost::promise<bool>> waitobject, 
	int maxtries, PACKET_TIMEOUT_STYLE howtotimeout, int timeoutvalue, const sf::Packet &packetsdata, int attempnumber /*= 1*/) : PacketNumber(packetid),
	ExpectedResponseID(-1), SentResponse(response), WaitForMe(waitobject), MaxTries(maxtries), PacketTimeoutStyle(howtotimeout),
	TimeOutMS(timeoutvalue), AlmostCompleteData(packetsdata), AttempNumber(attempnumber), RequestStartTime(Misc::GetTimeMs64()), ConfirmReceiveTime(0),
	IsArequest(false)
{

}
// ------------------ NetworkAckField ------------------ //
Leviathan::NetworkAckField::NetworkAckField(sf::Int32 firstpacketid, char maxacks, ReceivedPacketField &copyfrom) : FirstPacketID(firstpacketid){

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
			// Mark it as copied to a ack field //
			iter->second = true;
			// Set it //
setcurrentnumbertoackfield:

			Acks[vecelement] |= (1 << (i-vecelement));
			++iter;

		} else if(iter->first > curcheckacknumber){
			// We are missing these packets so let them be zeroed out //

		} else {
			// Advance iterator to reach this //
			for(; iter != copyfrom.end(); ++iter){
				if(iter->first >= curcheckacknumber){
					goto setcurrentnumbertoackfield;
				}
			}
		}
	}

	// We need to reset FirstPacketID if no acks set //
	if(Acks.size() == 0)
		FirstPacketID = -1;
}

DLLEXPORT void Leviathan::NetworkAckField::SetPacketsReceivedIfNotSet(ReceivedPacketField &copydatato){
	// We need to loop through all our acks and set them in the map //
	for(size_t i = 0; i < Acks.size(); i++){
		// Loop the individual bits //
		for(int bit = 0; bit < 8; bit++){

			// Check is it set //
			if(Acks[i] & (1 << bit)){
				// Set as received //
				copydatato[FirstPacketID+i*8+bit] = true;
			}
		}
	}
}

DLLEXPORT void Leviathan::NetworkAckField::RemoveMatchingPacketIDsFromMap(ReceivedPacketField &removefrom){
	// We need to loop through all our acks and erase them from the map (if set) //
	for(size_t i = 0; i < Acks.size(); i++){
		// Loop the individual bits //
		for(int bit = 0; bit < 8; bit++){

			// Check is it set //
			if(Acks[i] & (1 << bit)){
				// Remove it //
				removefrom.erase(FirstPacketID+i*8+bit);
			}
		}
	}
}
// ------------------ SentAcks ------------------ //
Leviathan::SentAcks::SentAcks(int packet, NetworkAckField* newddata) : InsidePacket(packet), AcksInThePacket(newddata), SendCount(1), Received(false){

}

Leviathan::SentAcks::~SentAcks(){
	SAFE_DELETE(AcksInThePacket);
}
