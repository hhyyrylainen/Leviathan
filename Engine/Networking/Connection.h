// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../Common/SFMLPackets.h"
#include "../Common/ThreadSafe.h"
#include "CommonNetwork.h"

#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"

#include "boost/circular_buffer.hpp"

#include <future>
#include <map>
#include <vector>
#include <memory>

//#define PACKET_USE_LOCK_WITH_CALLBACK

namespace Leviathan{

constexpr auto DEFAULT_ACKCOUNT = 32;
constexpr auto KEEPALIVE_TIME = 120000;
constexpr auto KEEPALIVE_RESPOND = 30000;
constexpr auto ACKKEEPALIVE = 200;
//! \brief The amount of received packet ids to keep in memory,
//! these ids are used to discard duplicate packets
constexpr auto KEEP_IDS_FOR_DISCARD	= 40;

//! \brief Fail reason for ConnectionInfo::CalculateNetworkPing
enum class PING_FAIL_REASON {
    
    LossTooHigh,
    ConnectionClosed
};

//! \brief Allows restricting connections to allow only certain packets
enum class CONNECTION_RESTRICTION {
    None,
    ReceiveRemoteConsole
};

//! \brief Main state of a connection
//!
//! Controls what packets can be received and also sending wrong ones will cause warning
//! messages
enum class CONNECTION_STATE {
    //! This is active when just created the object.
    //! Moves to Punchthrough or Initial depending on whether a NAT is blocking the connection
    NothingReceived,
    
    //! Begins the handshake with version number check
    Initial,

    //! Moves to this state once handshake is complete
    Connected,

    //! After a secure connection is set up, or if server tells client it isn't needed
    //! and client doesn't force secured
    Secured,

    //! Once the user has logged in and their unique ID has been verified
    //! At this point Player object should be created for the player. So that they can
    //! join a GameWorld
    Authenticated,

    //! Cannot receive anything. Caused by not receiveing packets or an explicit close
    //! Will send a single close connection packet when moving to this state
    //! Invalidates Player object and forces a logout
    Closed,

    //! When doing a NAT punhcthrough, will move to Initial after this
    //! In this state 10 punch through packets will be sent separated by 100 ms
    Punchthrough
};

//! Represents a sent packet and holds all kinds of data for it
class SentNetworkThing{
public:

    using CallbackType = std::function<void(bool, SentNetworkThing&)>;

    //! This is the signature for request packets
    DLLEXPORT SentNetworkThing(uint32_t packetid, std::shared_ptr<NetworkRequest> request,
        RECEIVE_GUARANTEE guarantee, PACKET_TIMEOUT_STYLE timeout, sf::Packet&& packetsdata);
        
    //! Empty destructor to link this in
    DLLEXPORT ~SentNetworkThing();
    
    // This is the signature for response packets //
    DLLEXPORT SentNetworkThing(uint32_t packetid, std::shared_ptr<NetworkResponse> response,
        RECEIVE_GUARANTEE guarantee, PACKET_TIMEOUT_STYLE timeout, sf::Packet&& packetsdata);

    //! \brief Returns true once the packet has been received by the target or lost
    //! too many times
    inline bool IsFinalized(){

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

    //! Local packet id
    uint32_t PacketNumber;

    //! If not RECEIVE_GUARANTEE::None this packet will be resent if considered lost
    RECEIVE_GUARANTEE Resend = RECEIVE_GUARANTEE::None;

    //! Used to detect when a critical packet is lost or if this packet has a specific
    //! number of resends
    uint8_t AttempNumber = 0;

    //! Controls how this packet is detected as lost
    //! For gameplay packets this should be PACKET_TIMEOUT_STYLE::PacketsAfterReceived
    //! and for requests and thing like chat PACKET_TIMEOUT_STYLE::Timed
    PACKET_TIMEOUT_STYLE PacketTimeoutStyle = PACKET_TIMEOUT_STYLE::PacketsAfterReceived;

    //! Callback function called when succeeded or failed
    //! May only be called by the receiving thread when removing this
    //! from the queue. May not be changed after settings to make sure
    //! that no race conditions exist
    std::shared_ptr<std::function<void(bool, SentNetworkThing&)>> Callback;

    //! The time this request was started
    int64_t RequestStartTime;

    //! \brief The time when this packed got marked as received
    //!
    //! This will roughly be the time it took for the packet to reach the destination and
    //! return the round-trip time
    //! \note This will only be set if this value is set to 1 before the packet is sent
    //! \note This value is only valid if the packet wasn't lost
    //! (failed requests have this unset)
    std::atomic<int64_t> ConfirmReceiveTime { 0 };

    //! Set to true once this object is no longer used
    std::atomic<bool> IsDone { false };
    bool Succeeded = false;
        
    // This is stored for resending the data //
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

//! \brief For keeping track of received remote packets.
//!
//! These are used to populate ack fields for outgoing packets
enum class RECEIVED_STATE{

    //! Packet hasn't been received
    NotReceived = 0,

    //! Packet is received but no acks have been sent
    StateReceived,

    //! Packet is received and an ack has been sent
    AcksSent,

    //! Packet is received and the ack is also received
    //! At this point we no longer need to think about this packet
    ReceivedAckSucceeded
};

using ReceivedPacketField = std::map<uint32_t, RECEIVED_STATE>;

class NetworkAckField{
public:

    DLLEXPORT NetworkAckField(uint32_t firstpacketid, uint8_t maxacks,
        ReceivedPacketField &copyfrom);

    DLLEXPORT void AddDataToPacket(sf::Packet &packet);

    DLLEXPORT NetworkAckField(sf::Packet &packet);

    //! \returns True if an ack number FirstPacketID + ackindex is set
    inline bool IsAckSet(uint8_t ackindex){
        
        // We can use division to find out which vector element is wanted //
        size_t vecelement = ackindex/8;

        return (Acks[vecelement] & (1 << (ackindex-vecelement))) != 0;
    }

    // Data //
    uint32_t FirstPacketID;
    std::vector<uint8_t> Acks;
};

//! \brief Holds sent ack packets in order to mark the acks as properly sent
struct SentAcks{

    //! \param insidepacket The packet in which the data was sent in
    SentAcks(uint32_t localpacketid, std::shared_ptr<NetworkAckField> acks) :
        InsidePacket(localpacketid), AcksInThePacket(acks)
    {}

    //! The packet (SentNetworkThing) in which these acks were sent //
    uint32_t InsidePacket;

    std::shared_ptr<NetworkAckField> AcksInThePacket;
        
    //! Marks when the remote host tells us that any packet in which bunch is in is received
    bool Received = false;
};

//! \brief Class that handles a single connection to another instance
//!
//! \note this class does not use reference counting 
class Connection : public ThreadSafe{
public:
    //! \brief Creates a new connection to hostname
    //! \todo Add a option to game configuration for default port
    DLLEXPORT Connection(const std::string &hostname);
    DLLEXPORT Connection(const sf::IpAddress &targetaddress, unsigned short port);
    DLLEXPORT ~Connection();

    //! Creates the address object
    //! \warning This function will lock the NetworkHandler object
    //! and thus it needs to be still valid
    DLLEXPORT bool Init(NetworkHandler* owninghandler);
    DLLEXPORT void Release();

    //! \brief Adds special restriction on the connection
    DLLEXPORT void SetRestrictionMode(CONNECTION_RESTRICTION type);

    //! \brief Checks does the sender and port match our corresponding values
    DLLEXPORT bool IsThisYours(sf::IpAddress &sender, unsigned short &sentport);

    //! \brief Handles a packet
    //! \note Care needs to be taken to avoid deadlocking while handling packet contents
    DLLEXPORT void HandlePacket(sf::Packet &packet);

    //! \returns True if this is a connection to a port on localhost
    DLLEXPORT bool IsTargetHostLocalhost();

    //! \brief Ticks this connection, times out sent packets
    DLLEXPORT void UpdateListening(Lock &guard);
        
    //! Send a request packet to this connection
    //! \returns nullptr If this connection is closed
    DLLEXPORT std::shared_ptr<SentNetworkThing> SendPacketToConnection(Lock &guard, 
        std::shared_ptr<NetworkRequest> request, RECEIVE_GUARANTEE guarantee,
        PACKET_TIMEOUT_STYLE timeout = PACKET_TIMEOUT_STYLE::PacketsAfterReceived);

    inline std::shared_ptr<SentNetworkThing> SendPacketToConnection(
        std::shared_ptr<NetworkRequest> request, RECEIVE_GUARANTEE guarantee,
        PACKET_TIMEOUT_STYLE timeout = PACKET_TIMEOUT_STYLE::PacketsAfterReceived)
    {
        GUARD_LOCK();
        return SendPacketToConnection(guard, request, guarantee, timeout);
    }
    
    //! Sends a reponse packet to this connection
    //! \returns nullptr If this connection is closed
    DLLEXPORT std::shared_ptr<SentNetworkThing> SendPacketToConnection(Lock &guard, 
        std::shared_ptr<NetworkResponse> response, RECEIVE_GUARANTEE guarantee,
        PACKET_TIMEOUT_STYLE timeout = PACKET_TIMEOUT_STYLE::PacketsAfterReceived);

    inline std::shared_ptr<SentNetworkThing> SendPacketToConnection(
        std::shared_ptr<NetworkResponse> response, RECEIVE_GUARANTEE guarantee,
        PACKET_TIMEOUT_STYLE timeout = PACKET_TIMEOUT_STYLE::PacketsAfterReceived)
    {
        GUARD_LOCK();
        return SendPacketToConnection(guard, response, guarantee, timeout);
    }
 

    //! \brief Blocks current thread until packet is successfully sent or lost
    //! \returns True if it was successfully sent, false if lost
    DLLEXPORT bool BlockUntilFinished(std::shared_ptr<NetworkResponse> packet);
        
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
    //! \note This will send packets asynchronously to the connection and
    //! can take up to an second to  call the callbacks
    //! \param packets The amount of timing packets to send
    //! \param allowedfails Is the amount of missed packets allowed before failing
    //! \param onsucceeded Called after successfully completed.
    //! First value is the ping in milliseconds and the second is failed packets
    //! \param onfailed Is called if the function fails. First value will be the reason and
    //! second the failed packet count
    //! \todo Check whether the packets should be send in a cluster or not
    //! (as they are currently sent in one go)
    DLLEXPORT void CalculateNetworkPing(int packets, int allowedfails,
        std::function<void(int, int)> onsucceeded,
        std::function<void(PING_FAIL_REASON, int)> onfailed);

    //! \brief Called when the other side sends us an ack
    //!
    //! Used to mark our packets as sent
    DLLEXPORT void HandleRemoteAck(Lock &guard, uint32_t localidconfirmedassent);

    //! \brief Fills a packet with all the data that it would get filled with in SendPacket
    //!
    //! Used mainly for local testing purposes where it's difficult to use sockets
    DLLEXPORT std::shared_ptr<SentNetworkThing> CreateFullSendablePacket(
        std::shared_ptr<NetworkResponse> data, sf::Packet &packettofill,
        bool skipwaitingrequests = false);

protected:

    
    //! \brief Sets acks in a packet as properly sent in this
    //!
    //! Acks that were false in the packet are untouched
    DLLEXPORT void SetPacketsReceivedIfNotSet(NetworkAckField &acks);

    //! \brief Removes acks that were successful in the packet from target
    //!
    //! Will also remove NetworkAckFields from the vector holding waiting ack groups
    //! \note Should be called after the packet containing these acks is marked as
    //! successfull
    DLLEXPORT void RemoveSucceededAcks(NetworkAckField &acks);
    
private:

    //! \brief Prepares a new header for the thing but keeps the original local id and
    //! sends the packet again
    void _ResendRequest(Lock &guard, std::shared_ptr<SentNetworkThing> toresend);

    //! \brief Creates a standard header and ack field for outgoing packet
    //! \param tofill An empty packet where the packet header can be added
    //! \param dontsendacks If true first ack will be set to -1 and count to 0
    void _PreparePacketHeaderForPacket(Lock &guard, uint32_t localpacketid,
        sf::Packet &tofill, bool isrequest, bool dontsendacks = false);

    //! \brief Returns a request matching the response's reference ID or NULL
    std::shared_ptr<SentNetworkThing> _GetPossibleRequestForResponse(Lock &guard,
        std::shared_ptr<NetworkResponse> response);

    //! \brief Marks a remote id as received
    //!
    //! This function will also store the packetid for later checks
    //! \returns True If already received a packet with the id
    bool _MarkNewAsReceived(uint32_t remotepacketid);

    // ------------------------------------ //

    //! The main state of connection
    CONNECTION_STATE State = CONNECTION_STATE::NothingReceived;
    
    //! Used to send acks for received remote packets
    ReceivedPacketField ReceivedRemotePackets;

    //! Holds the ID of the last sent packet
    //! Incremented everytime a packet is sent to keep local
    //! packet ids different
    //! \note The world will break once this wraps around and reaches 0
    //! \todo Fix that
    uint32_t LastUsedLocalID = 0;

    //! Holds the id of last local sent packet that we have received an ack for
    uint32_t LastConfirmedSent = 0;
        

    //! Connections might have special restrictions on them
    //! This is mainly used to accept only remote console feature on clients
    CONNECTION_RESTRICTION RestrictType = CONNECTION_RESTRICTION::None;

    //! Flipped everytime a packet is sent to toggle sending acks from the front or the back
    //! In normal operation doesn't matter but in exceptional circumstances
    //! allows more acks to be sent by sending 2 group of acks for each round while
    //! waiting for confirmation of ack receive
    bool FrontAcks = true;

    //! When acks pile up increase this value to send more acks
    //! \todo Implement this
    uint8_t ExtraAckCount = 0;
    
    int64_t LastSentPacketTime;
    int64_t LastReceivedPacketTime;

    //! Sent packets that haven't been confirmed as arrived
    //! \todo Split this into two, requests and responses
    std::vector<std::shared_ptr<SentNetworkThing>> WaitingRequests;

    //! Holds sent ack groups until they are considered lost or received and
    //! then is used to mark the received packets as successfully sent
    std::vector<std::shared_ptr<SentAcks>> SentAckPackets;

    //! IDs of packets used to drop same packets
    boost::circular_buffer<uint32_t> LastReceivedPacketIDs;

    //! The remote port
    unsigned short TargetPortNumber;

    //! If the target remote was specified with an url this stores it for
    //! later reconstruction
    std::string HostName;

    //! Resolved address of the remote
    sf::IpAddress TargetHost;

    //! True when TargetHost has been retrieved from HostName or TargetHost is
    //! made valid some other way
    bool AddressGot;
};

}

