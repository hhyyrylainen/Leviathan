#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "NetworkResponse.h"
#include "NetworkRequest.h"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "../Common/ThreadSafe.h"
#include "NetworkHandler.h"
#include "../Common/BaseNotifier.h"
#include <future>
#include <map>
#include <list>
#include <vector>
#include <memory>
#include <deque>

namespace Leviathan{

#define DEFAULT_ACKCOUNT		32
#define KEEPALIVE_TIME			120000
#define KEEPALIVE_RESPOND		30000
#define ACKKEEPALIVE			200


//! \brief The amount of received packet ids to keep in memory, these ids are used to discard duplicate packets
#define KEEP_IDS_FOR_DISCARD	40

//! Makes the program spam a ton of debug info about packets //
//#define SPAM_ME_SOME_PACKETS	1

    //! \brief Fail reason for ConnectionInfo::CalculateNetworkPing
    enum CONNECTION_PING_FAIL_REASON {
        CONNECTION_PING_FAIL_REASON_LOSS_TOO_HIGH,
        CONNECTION_PING_FAIL_REASON_CONNECTION_CLOSED};

    
    
	//! \brief Allows restricting connections to allow only certain packets
	enum CONNECTION_RESTRICTION {
        CONNECTION_RESTRICTION_NONE,
        CONNECTION_RESTRICTION_RECEIVEREMOTECONSOLE
    };

    //! Represents a sent packet and holds all kinds of data for it
    //! \todo Make this properly thread safe
    //! \todo Make BaseSendable receive proper callbacks even after this has failed, so add
    //! a place where failed packets are stored for a while
	class SentNetworkThing{
    public:

		//! This is the signature for request packets
		DLLEXPORT SentNetworkThing(int packetid, int expectedresponseid,
            std::shared_ptr<NetworkRequest> request, int maxtries,
            PACKET_TIMEOUT_STYLE howtotimeout,
            int timeoutvalue, const sf::Packet &packetsdata, int attempnumber = 1);
        
		//! Empty destructor to link this in
		DLLEXPORT ~SentNetworkThing();
		// This is the signature for response packets //
		DLLEXPORT SentNetworkThing(int packetid, std::shared_ptr<NetworkResponse> response,
            int maxtries, PACKET_TIMEOUT_STYLE howtotimeout, int timeoutvalue,
            const sf::Packet &packetsdata, int attempnumber = 1);

        //! \brief Returns true once the packet has been received by the target or lost
        //! too many times
        DLLEXPORT inline bool IsFinalized(){

            return IsDone.load(std::memory_order_consume);
        }

        //! \brief Gets the status once IsFinalized returns true blocks otherwise
        //! \return True when the packet has been successfully received, false if lost
        DLLEXPORT bool GetStatus();
        
        //! \brief Sets the status of the wait object notifying all waiters that this has
        //! succeeded or failed
        //!
        //! Will also call the Callback if one is set
        //! \note May only be called once
        DLLEXPORT void SetWaitStatus(bool status);
        
        //! \brief Sets this packet as a timed packet
        //! \note A timed package will have the ConfirmReceiveTime set to the time a response
        //! (or receive notification) is received
        DLLEXPORT void SetAsTimed();

        //! \brief Binds a callback function that is called either when the packet is
        //! successfully sent or it times out
        DLLEXPORT void SetCallback(std::function<void(bool, SentNetworkThing&)> func);

		int PacketNumber;

		int MaxTries;
		int AttempNumber;

		PACKET_TIMEOUT_STYLE PacketTimeoutStyle;

        //! Callback function called when succeeded or failed
        std::function<void(bool, SentNetworkThing&)> Callback;
        
		int TimeOutMS;
		int64_t RequestStartTime;

        //! \brief The time when this packed got marked as received
        //!
        //! This will roughly be the time it took for the packet to reach the destination and return
        //! the round-trip time
        //! \note This will only be set if this value is set to 1 before the packet is sent
        //! \note This value is only valid if the packet wasn't lost (failed requests have this unset)
        std::atomic<int64_t> ConfirmReceiveTime;
		int ExpectedResponseID;


		//! Marks this as received by the other
        std::condition_variable Notifier;

        Mutex NotifyMutex;

        std::atomic_bool IsDone;
        bool Succeeded;
        
		// This is stored for resending the data //
        //! \todo Store as a pointer
		sf::Packet AlmostCompleteData;

		// If set the following variables will be used //
		bool IsArequest;
        std::shared_ptr<NetworkResponse> GotResponse;
        std::shared_ptr<NetworkRequest> OriginalRequest;
		// Else (if not a request) no response is expected (other than a receive confirmation) //
        std::shared_ptr<NetworkResponse> SentResponse;
	};

	static_assert(sizeof(char) == 1, "Char must be one byte in size");
	static_assert(sizeof(int) == 4, "Int must be four bytes in size");

	typedef std::map<int, bool> ReceivedPacketField;

	class NetworkAckField{
	public:

		DLLEXPORT NetworkAckField(){};
		DLLEXPORT NetworkAckField(sf::Int32 firstpacketid, char maxacks,
            ReceivedPacketField &copyfrom);

		DLLEXPORT inline bool IsAckSet(size_t ackindex){
			// We can use division to find out which vector element is wanted //
			size_t vecelement = ackindex/8;

			return (Acks[vecelement] & (1 << (ackindex-vecelement))) != 0;
		}

		// If the ack in this field is set then it is set in the argument map, but if ack is not set in this field
        //! it isn't reseted in the argument map //
		DLLEXPORT void SetPacketsReceivedIfNotSet(ReceivedPacketField &copydatato);

		DLLEXPORT void RemoveMatchingPacketIDsFromMap(ReceivedPacketField &removefrom);

		// Data //
		sf::Int32 FirstPacketID;
        std::vector<sf::Int8> Acks;
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
	//! \note this class does not use reference counting so it it safe to use std::shared_ptr with this class
	//! \todo Internal security tokens to all packets
	//! \todo Remove sent ack groups after they have "probably failed"
	class ConnectionInfo : public BaseNotifierAll{
	public:
		//! \brief Creates a new connection to hostname
		//! \todo Add a option to game configuration for default port
		DLLEXPORT ConnectionInfo(const std::string &hostname);
		DLLEXPORT ConnectionInfo(const sf::IpAddress &targetaddress, unsigned short port);
		DLLEXPORT ~ConnectionInfo();

		//! Creates the address object
        //! \warning This function will lock the NetworkHandler object and thus it needs to be still valid
		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		//! \brief Adds special restriction on the connection
		DLLEXPORT void SetRestrictionMode(CONNECTION_RESTRICTION type);

        //! \brief Checks does the sender and port match our corresponding values
		DLLEXPORT bool IsThisYours(sf::IpAddress &sender, unsigned short &sentport);

        //! \brief Handles a packet
        //! \note No other locks should be held while calling this
        DLLEXPORT void HandlePacket(sf::Packet &packet, sf::IpAddress &sender,
            unsigned short &sentport);
        
		DLLEXPORT bool IsTargetHostLocalhost();


		DLLEXPORT void UpdateListening();

		DLLEXPORT std::shared_ptr<SentNetworkThing> SendPacketToConnection(
            std::shared_ptr<NetworkRequest> request, int maxretries);
        
		DLLEXPORT std::shared_ptr<SentNetworkThing> SendPacketToConnection(
            std::shared_ptr<NetworkResponse> response, int maxtries);

		// Data exchange functions //
		DLLEXPORT std::shared_ptr<NetworkResponse> SendRequestAndBlockUntilDone(
            std::shared_ptr<NetworkRequest> request, int maxtries = 2);

		DLLEXPORT void CheckKeepAliveSend();
		DLLEXPORT void SendKeepAlivePacket(Lock &guard);
		DLLEXPORT FORCE_INLINE void SendKeepAlivePacket(){
			GUARD_LOCK();
			SendKeepAlivePacket(guard);
		}

		//! \brief Sends a packet that tells the other side to disconnect
		//! \todo Add a message parameter for the reason
		DLLEXPORT void SendCloseConnectionPacket(Lock &guard);
		DLLEXPORT FORCE_INLINE void SendCloseConnectionPacket(){
			GUARD_LOCK();
			SendCloseConnectionPacket(guard);
		}

		//! \brief Returns a nicely formated address string for this connection
		//!
		//! \return For example something like "0.0.0.127:2565"
		//! \todo this could be cached
		DLLEXPORT std::string GenerateFormatedAddressString() const;

        //! \brief Calculates the ping (round-trip time) on this connection
        //! \note This will send packets asynchronously to the connection and can take up to an second to
        //! call the callbacks
        //! \param packets The amount of timing packets to send
        //! \param allowedfails Is the amount of missed packets allowed before failing
        //! \param onsucceeded Called after successfully completed. First value is the ping in milliseconds and the
        //! second is failed packets
        //! \param onfailed Is called if the function fails. First value will be the reason and
        //! second the failed packet count
        //! \todo Check whether the packets should be send in a cluster or not (as they are currently sent in one go)
        DLLEXPORT void CalculateNetworkPing(int packets, int allowedfails,
            std::function<void(int, int)> onsucceeded,
            std::function<void(CONNECTION_PING_FAIL_REASON, int)> onfailed);

		//! Don't call this
		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

	private:

		//void _PopMadeRequest(shared_ptr<SentNetworkThing> objectptr, Lock &guard);
		void _ResendRequest(std::shared_ptr<SentNetworkThing> toresend, Lock &guard);

		// Marks the acks in packet received as successfully sent and erases them //
		void _VerifyAckPacketsAsSuccesfullyReceivedFromHost(int packetreceived);

		void _PreparePacketHeaderForPacket(int packetid, sf::Packet &tofill, bool isrequest,
            bool dontsendacks = false);

        std::shared_ptr<SentNetworkThing> _GetPossibleRequestForResponse(
            std::shared_ptr<NetworkResponse> response);

		//! \brief Checks whether a packet with the number is received
		//!
		//! This function will also store the packetid for later checks
		bool _IsAlreadyReceived(int packetid);

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

		int64_t LastSentPacketTime;
		int64_t LastReceivedPacketTime;

		//! With this we can close connections that have never received anything //
		bool HasReceived;

		// Sent packets that haven't been confirmed as arrived //
		std::list<std::shared_ptr<SentNetworkThing>> WaitingRequests;

		std::vector<std::shared_ptr<SentAcks>> AcksNotConfirmedAsReceived;

		//! IDs of packets used to drop same packets
		std::deque<int> LastReceivedPacketIDs;

		unsigned short TargetPortNumber;
		std::string HostName;
		sf::IpAddress TargetHost;
		bool AddressGot;
	};

}

