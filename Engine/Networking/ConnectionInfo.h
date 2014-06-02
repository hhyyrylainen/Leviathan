#ifndef LEVIATHAN_CONNECTIONINFO
#define LEVIATHAN_CONNECTIONINFO
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NetworkResponse.h"
#include "NetworkRequest.h"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "Common/ThreadSafe.h"
#include <boost/thread/future.hpp>
#include "NetworkHandler.h"
#include "Common/BaseNotifier.h"

namespace Leviathan{

#define DEFAULT_ACKCOUNT		32
#define KEEPALIVE_TIME			120000
#define KEEPALIVE_RESPOND		30000
#define ACKKEEPALIVE			50


//! Makes the program spam a ton of debug info about packets //
//#define SPAM_ME_SOME_PACKETS	1

	//! \brief Allows restricting connections to allow only certain packets
	enum CONNECTION_RESTRICTION {CONNECTION_RESTRICTION_NONE, CONNECTION_RESTRICTION_RECEIVEREMOTECONSOLE};

	struct SentNetworkThing{

		//! This is the signature for request packets
		DLLEXPORT SentNetworkThing(int packetid, int expectedresponseid, shared_ptr<NetworkRequest> request, shared_ptr<boost::promise<bool>> waitobject, 
			int maxtries, PACKET_TIMEOUT_STYLE howtotimeout, int timeoutvalue, const sf::Packet &packetsdata, int attempnumber = 1);
		//! Empty destructor to link this in
		DLLEXPORT ~SentNetworkThing();
		// This is the signature for response packets //
		DLLEXPORT SentNetworkThing(int packetid, shared_ptr<NetworkResponse> response, shared_ptr<boost::promise<bool>> waitobject, int maxtries, 
			PACKET_TIMEOUT_STYLE howtotimeout, int timeoutvalue, const sf::Packet &packetsdata, int attempnumber = 1);

		DLLEXPORT boost::unique_future<bool>& GetFutureForThis();

		int PacketNumber;

		int MaxTries;
		int AttempNumber;

		PACKET_TIMEOUT_STYLE PacketTimeoutStyle;

		int TimeOutMS;
		__int64 RequestStartTime;
		__int64 ConfirmReceiveTime;
		int ExpectedResponseID;


		//! Marks this as received by the other
		shared_ptr<boost::promise<bool>> WaitForMe;
		//! The stored future that is returned when requested
		boost::unique_future<bool> FutureValue;
		//! Controls when the future will be fetched, it is safe to retrieve only once
		bool FutureFetched;


		// This is stored for resending the data //
		sf::Packet AlmostCompleteData;

		// If set the following variables will be used //
		bool IsArequest;
		shared_ptr<NetworkResponse> GotResponse;
		shared_ptr<NetworkRequest> OriginalRequest;
		// Else (if not a request) no response is expected (other than a receive confirmation) //
		shared_ptr<NetworkResponse> SentResponse;
	};

	static_assert(sizeof(char) == 1, "Char must be one byte in size");
	static_assert(sizeof(int) == 4, "Int must be four bytes in size");

	typedef std::map<int, bool> ReceivedPacketField;

	class NetworkAckField{
	public:

		DLLEXPORT NetworkAckField(){};
		DLLEXPORT NetworkAckField(sf::Int32 firstpacketid, char maxacks, ReceivedPacketField &copyfrom);

		DLLEXPORT inline bool IsAckSet(size_t ackindex){
			// We can use division to find out which vector element is wanted //
			size_t vecelement = ackindex/8;

			return (Acks[vecelement] & (1 << (ackindex-vecelement))) != 0;
		}

		// If the ack in this field is set then it is set in the argument map, but if ack is not set in this field it isn't reseted in the argument map //
		DLLEXPORT void SetPacketsReceivedIfNotSet(ReceivedPacketField &copydatato);

		DLLEXPORT void RemoveMatchingPacketIDsFromMap(ReceivedPacketField &removefrom);

		// Data //
		sf::Int32 FirstPacketID;
		vector<sf::Int8> Acks;
	};

	struct SentAcks{

		SentAcks(int packet, NetworkAckField* newddata);
		~SentAcks();

		// The packet (SentNetworkThing) in which these acks were sent //
		int InsidePacket;
		//! Used to control how many times to send each ackbunch //
		//! If package loss is high this will be increased to make sure acks are received //
		int SendCount;

		//! Marks if this can be deleted (after using for resends, of course) //
		bool Received;

		NetworkAckField* AcksInThePacket;
	};

	//! \brief Class that handles a single connection to another instance
	//!
	//! \note this class does not use reference counting so it it safe to use shared_ptr with this class
	//! \todo Internal security tokens to all packets
	class ConnectionInfo : public BaseNotifierAll{
	public:
		//! \brief Creates a new connection to hostname
		//! \todo Add a option to game configuration for default port
		DLLEXPORT ConnectionInfo(const wstring &hostname);
		DLLEXPORT ConnectionInfo(const sf::IpAddress &targetaddress, USHORT port);
		DLLEXPORT ~ConnectionInfo();

		//! Creates the address object
		//! \todo Make calls to this threaded to avoid stalls
		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		//! \brief Adds special restriction on the connection
		DLLEXPORT void SetRestrictionMode(CONNECTION_RESTRICTION type);

		DLLEXPORT bool IsThisYours(sf::Packet &packet, sf::IpAddress &sender, USHORT &sentport);
		DLLEXPORT bool IsTargetHostLocalhost();


		DLLEXPORT void UpdateListening();

		DLLEXPORT shared_ptr<SentNetworkThing> SendPacketToConnection(shared_ptr<NetworkRequest> request, int maxretries);
		DLLEXPORT shared_ptr<SentNetworkThing> SendPacketToConnection(shared_ptr<NetworkResponse> response, int maxtries);

		// Data exchange functions //
		DLLEXPORT shared_ptr<NetworkResponse> SendRequestAndBlockUntilDone(shared_ptr<NetworkRequest> request, int maxtries = 2);

		DLLEXPORT void CheckKeepAliveSend();
		DLLEXPORT void SendKeepAlivePacket(ObjectLock &guard);
		DLLEXPORT FORCE_INLINE void SendKeepAlivePacket(){
			GUARD_LOCK_THIS_OBJECT();
			SendKeepAlivePacket(guard);
		}

		//! \brief Sends a packet that tells the other side to disconnect
		//! \todo Add a message parameter for the reason
		DLLEXPORT void SendCloseConnectionPacket(ObjectLock &guard);
		DLLEXPORT FORCE_INLINE void SendCloseConnectionPacket(){
			GUARD_LOCK_THIS_OBJECT();
			SendCloseConnectionPacket(guard);
		}

		//! \brief Returns a nicely formated address string for this connection
		//!
		//! \return For example something like "0.0.0.127:2565"
		//! \todo this could be cached
		DLLEXPORT wstring GenerateFormatedAddressString() const;

		//! Don't call this
		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

	private:

		//void _PopMadeRequest(shared_ptr<SentNetworkThing> objectptr, ObjectLock &guard);
		void _ResendRequest(shared_ptr<SentNetworkThing> toresend, ObjectLock &guard);

		// Marks the acks in packet received as successfully sent and erases them //
		void _VerifyAckPacketsAsSuccesfullyReceivedFromHost(int packetreceived);

		void _PreparePacketHeaderForPacket(int packetid, sf::Packet &tofill, bool isrequest);

		shared_ptr<SentNetworkThing> _GetPossibleRequestForResponse(shared_ptr<NetworkResponse> response);
		// ------------------------------------ //

		// Packet sent and received data //
		std::map<int, bool> SentPacketsConfirmedAsReceived;
		std::map<int, bool> ReceivedPacketsNotifiedAsReceivedByUs;
		int LastSentConfirmID;

		int MyLastSentReceived;

		// Holds the ID of the last sent packet //
		int LastUsedID;

		//! Connections might have special restrictions on them (mainly the accept only remote console feature) //
		CONNECTION_RESTRICTION RestrictType;


		// How many times the same ack table is sent before new one is generated (usually 1 with good connections) //
		int MaxAckReduntancy;

		__int64 LastSentPacketTime;
		__int64 LastReceivedPacketTime;

		//! With this we can close connections that have never received anything //
		bool HasReceived;

		// Sent packets that haven't been confirmed as arrived //
		std::list<shared_ptr<SentNetworkThing>> WaitingRequests;

		std::vector<shared_ptr<SentAcks>> AcksNotConfirmedAsReceived;

		USHORT TargetPortNumber;
		wstring HostName;
		sf::IpAddress TargetHost;
		bool AddressGot;
	};

}
#endif