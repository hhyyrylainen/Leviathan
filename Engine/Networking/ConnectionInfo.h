#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "../Common/ThreadSafe.h"
#include "NetworkHandler.h"
#include "../Common/BaseNotifier.h"
#include <future>
#include <map>
#include <vector>
#include <memory>
#include "boost/circular_buffer.hpp"

namespace Leviathan{

#define DEFAULT_ACKCOUNT		32
#define KEEPALIVE_TIME			120000
#define KEEPALIVE_RESPOND		30000
#define ACKKEEPALIVE			200


//! \brief The amount of received packet ids to keep in memory, these ids are used to discard duplicate packets
#define KEEP_IDS_FOR_DISCARD	40

    //! \brief Fail reason for ConnectionInfo::CalculateNetworkPing
    enum CONNECTION_PING_FAIL_REASON {
        CONNECTION_PING_FAIL_REASON_LOSS_TOO_HIGH,
        CONNECTION_PING_FAIL_REASON_CONNECTION_CLOSED
    };

    
    
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

        using CallbackType = std::function<void(bool, SentNetworkThing&)>;

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
        //! \bug This can corrupt the arguments passed to this function, not recommended for use
        DLLEXPORT void SetCallback(std::shared_ptr<CallbackType> func = nullptr);

		int PacketNumber;

		int MaxTries;
		int AttempNumber;

		PACKET_TIMEOUT_STYLE PacketTimeoutStyle;

        //! Callback function called when succeeded or failed
        std::shared_ptr<std::function<void(bool, SentNetworkThing&)>> Callback;
        
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

        //! Locked when Callback is being changed or while it is executing
        Mutex CallbackMutex;

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

    enum RECEIVED_STATE{

        //! Packet hasn't been received
        RECEIVED_STATE_NOT_RECEIVED = 0,

        //! Packet is received but no acks have been sent
        RECEIVED_STATE_RECEIVED,

        //! Packet is received and an ack has been sent
        RECEIVED_STATE_ACKS_SENT,

        //! Packet is received and the ack is also received
        RECEIVED_STATE_RECEIVED_ACK_SUCCEEDED
    };

	using ReceivedPacketField = std::map<int, RECEIVED_STATE>;

	class NetworkAckField{
	public:

		DLLEXPORT NetworkAckField(int32_t firstpacketid, char maxacks,
            ReceivedPacketField &copyfrom);

        DLLEXPORT void AddDataToPacket(sf::Packet &packet);

        DLLEXPORT NetworkAckField(sf::Packet &packet);


		DLLEXPORT inline bool IsAckSet(size_t ackindex){
			// We can use division to find out which vector element is wanted //
			size_t vecelement = ackindex/8;

			return (Acks[vecelement] & (1 << (ackindex-vecelement))) != 0;
		}

        //! \brief Sets acks in this packet as properly sent in receiver
        //!
        //! Acks that were false in this packet are untouched
		DLLEXPORT void SetPacketsReceivedIfNotSet(ConnectionInfo &receiver);

        //! \brief Removes acks that were successful in this packet from target
        //! \note Should be called after the packet containing these acks is marked as
        //! successfull
        DLLEXPORT void RemoveSucceededAcks(ConnectionInfo &target);


		// Data //
		int32_t FirstPacketID;
        std::vector<int8_t> Acks;
	};

	struct SentAcks{

        //! \param insidepacket The packet in which the data was sent in
		SentAcks(int insidepacket, std::shared_ptr<NetworkAckField> acks);

		//! The packet (SentNetworkThing) in which these acks were sent //
		int InsidePacket;

        std::shared_ptr<NetworkAckField> AcksInThePacket;
        
		//! Used to control how many times to send each packet of acks
		//! If package loss is high this will be increased to make sure acks are received
		int SendCount = 1;

        //! Marks when the remote host tells us that any packet in which bunch is is received
		bool Received = false;
	};

	//! \brief Class that handles a single connection to another instance
	//!
	//! \note this class does not use reference counting so it it safe to use std::shared_ptr with this class
	//! \todo Internal security tokens to all packets
	//! \todo Remove sent ack groups after they have "probably failed"
	class ConnectionInfo : public BaseNotifierAll{
        friend NetworkAckField;
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

        //! \brief Ticks this connection, times out sent packets
		DLLEXPORT void UpdateListening(Lock &guard);
        

		DLLEXPORT std::shared_ptr<SentNetworkThing> SendPacketToConnection(Lock &guard, 
            std::shared_ptr<NetworkRequest> request, int maxretries);

		DLLEXPORT inline std::shared_ptr<SentNetworkThing> SendPacketToConnection(
            std::shared_ptr<NetworkRequest> request, int maxretries)
        {
            GUARD_LOCK();
            return SendPacketToConnection(guard, request, maxretries);
        }
        
        
		DLLEXPORT std::shared_ptr<SentNetworkThing> SendPacketToConnection(Lock &guard, 
            std::shared_ptr<NetworkResponse> response, int maxtries);

		DLLEXPORT inline std::shared_ptr<SentNetworkThing> SendPacketToConnection(
            std::shared_ptr<NetworkResponse> response, int maxtries)
        {
            GUARD_LOCK();
            return SendPacketToConnection(guard, response, maxtries);
        }
        

		// Data exchange functions //
        //! \deprecated Should loop and wait on SentNetworkThing
		DLLEXPORT std::shared_ptr<NetworkResponse> SendRequestAndBlockUntilDone(
            std::shared_ptr<NetworkRequest> request, int maxtries = 2);
        
        //! \brief Sends a keepalive packet if enough time has passed
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

        //! \brief Called when the other side sends us an ack
        //!
        //! Used to mark our packets as sent
        DLLEXPORT void HandleRemoteAck(Lock &guard, int oursentconfirmed);

        DLLEXPORT inline void HandleRemoteAck(int oursentconfirmed){

            GUARD_LOCK();
            HandleRemoteAck(guard, oursentconfirmed);
        }

	private:

        //! \brief Prepares a new header for the thing but keeps the original local id and
        //! sends the packet again
		void _ResendRequest(std::shared_ptr<SentNetworkThing> toresend, Lock &guard);

        //! \brief Creates a standard header and ack field for outgoing packet
        //! \param tofill An empty packet where the packet header can be added
        //! \param dontsendacks If true first ack will be set to -1 and count to 0
		void _PreparePacketHeaderForPacket(Lock &guard, int localpacketid, sf::Packet &tofill,
            bool isrequest, bool dontsendacks = false);

        //! \brief Returns a request matching the response's reference ID or NULL
        std::shared_ptr<SentNetworkThing> _GetPossibleRequestForResponse(Lock &guard,
            std::shared_ptr<NetworkResponse> response);

		//! \brief Checks whether a packet with the number is received from remote
		//!
		//! This function will also store the packetid for later checks
		bool _IsAlreadyReceived(int remotepacketid);

		// ------------------------------------ //

        //! Used to send acks for received remote packets
		ReceivedPacketField ReceivedRemotePackets;

		//! Holds the ID of the last sent packet
        //! Incremented everytime a packet is sent to keep local
        //! packet ids different
        //! \note The world will break once this wraps around and reaches -1
		int LastUsedLocalID = -1;

        //! Holds the id of last local sent packet that we have received an ack for
        int LastConfirmedSent = -1;
        

		//! Connections might have special restrictions on them (mainly the accept only remote console feature)
		CONNECTION_RESTRICTION RestrictType;

        //! Flipped everytime a packet is sent to toggle sending acks from the front or the back
        //! In normal operation doesn't matter but in exceptional circumstances allows more acks to be sent
        //! by sending 2 group of acks for each round while waiting for confirmation of ack receive
        bool FrontAcks = true;

        //! When acks pile up increase this value to send more acks
        //! \todo Add a proper method that tracks unsent acks with time and modifies this
        int ExtraAckCount = 0;
        
		int64_t LastSentPacketTime;
		int64_t LastReceivedPacketTime;

		//! With this we can close connections that have never received anything //
		bool HasReceived = false;

		//! Sent packets that haven't been confirmed as arrived
        //! \todo Split this into two, requests and responses
		std::vector<std::shared_ptr<SentNetworkThing>> WaitingRequests;

        //! Holds sent ack groups until they are considered lost or received and
        //! then is used to mark the received packets as successfully sent
		std::vector<std::shared_ptr<SentAcks>> SentAckPackets;

		//! IDs of packets used to drop same packets
        boost::circular_buffer<int> LastReceivedPacketIDs;

		unsigned short TargetPortNumber;

        //! If the target remote was specified with an url this stores it for
        //! later reconstruction
		std::string HostName;
		sf::IpAddress TargetHost;

        //! True when TargetHost has been retrieved from HostName or TargetHost is
        //! made valid some other way
		bool AddressGot;
	};

}

