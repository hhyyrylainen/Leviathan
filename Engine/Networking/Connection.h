// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "CommonNetwork.h"

#include "NetworkAckField.h"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"

#include "boost/circular_buffer.hpp"

#include <map>
#include <memory>
#include <vector>

namespace sf {
class Packet;
}

namespace Leviathan {

class SentRequest;
class SentResponse;

enum class NETWORK_RESPONSE_TYPE : uint16_t;

constexpr auto DEFAULT_ACKCOUNT = 32;
constexpr auto KEEPALIVE_TIME = 120000;
constexpr auto ACKKEEPALIVE = 200;

constexpr auto ACK_ONLY_DEFAULT_MAX = 4;

//! If true 2 ack only packets are created each time one is sent
constexpr auto DOUBLE_SEND_FOR_ACK_ONLY = true;

//! Number of bytes that should be in each packet for it to be considered full
//! Could probably be set to 1452 and still be fine on most connections
//! But ipv4 promises that at least 512 byte payload should work
constexpr auto DEFAULT_PACKET_FILL_AMOUNT = 512;

//! \brief The amount of received message numbers to keep in memory,
//! these numbers are used to discard duplicates
//!
//! This is required for ack only packets to work correctly (as they
//! rely on getting a resend if the acks are lost)
constexpr auto KEEP_IDS_FOR_DISCARD = 50;

//! This is for debugging purposes to make sure that packet numbers and message numbers aren't
//! used interchangeably anywhere
constexpr auto PACKET_NUMBERING_OFFSET = 1000;

//! \brief Fail reason for ConnectionInfo::CalculateNetworkPing
enum class PING_FAIL_REASON {

    LossTooHigh,
    ConnectionClosed
};

//! \brief Allows restricting connections to allow only certain packets
enum class CONNECTION_RESTRICTION { None, ReceiveRemoteConsole };

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

    //! Cannot receive anything. Caused by not receiving packets or an explicit close
    //! Will send a single close connection packet when moving to this state
    //! Invalidates Player object and forces a logout
    Closed,

    //! When doing a NAT punch through, will move to Initial after this
    //! In this state 10 punch through packets will be sent separated by 100 ms
    Punchthrough
};


//! \brief Class that handles a single connection to another instance
//!
//! This is not thread safe so only one thread at a time may handle this connection.
//! this means that all worker threads must use Engine::Invoke to send packets
//! \note this class does not use reference counting
class Connection {
public:
    //! \brief Creates a new connection to hostname
    //! \todo Add a option to game configuration for default port
    DLLEXPORT Connection(const std::string& hostname);
    DLLEXPORT Connection(const sf::IpAddress& targetaddress, unsigned short port);
    DLLEXPORT ~Connection();

    //! Creates the address object
    //! \warning This function will lock the NetworkHandler object
    //! and thus it needs to be still valid
    //! \todo Make this async resolve the address
    DLLEXPORT bool Init(NetworkHandler* owninghandler);
    DLLEXPORT void Release();

    //! \brief Returns true if this socket is valid for sending
    inline bool IsValidForSend() const
    {
        return State != CONNECTION_STATE::Closed;
    }

    inline CONNECTION_STATE GetState() const
    {
        return State;
    }

    //! \brief Adds special restriction on the connection
    DLLEXPORT void SetRestrictionMode(CONNECTION_RESTRICTION type);

    //! \brief Checks does the sender and port match our corresponding values
    //! \returns True if sender and sentport match the ones in this connection
    inline bool IsThisYours(const sf::IpAddress& sender, unsigned short sentport)
    {
        return sentport == TargetPortNumber && sender == TargetHost;
    }

    //! \brief Handles a packet
    DLLEXPORT void HandlePacket(sf::Packet& packet);

    //! \returns True if this is a connection to a port on localhost
    DLLEXPORT bool IsTargetHostLocalhost();

    //! \brief Ticks this connection, times out sent packets
    DLLEXPORT void UpdateListening();

    //! Send a request packet to this connection
    //! \returns nullptr If this connection is closed
    DLLEXPORT std::shared_ptr<SentRequest> SendPacketToConnection(
        const std::shared_ptr<NetworkRequest>& request, RECEIVE_GUARANTEE guarantee);

    //! \brief Sends a response that doesn't need to be confirmed to be received
    DLLEXPORT bool SendPacketToConnection(const NetworkResponse& response);

    //! \brief Sends a response that won't be resent but it will be tracked
    DLLEXPORT std::shared_ptr<SentResponse> SendPacketToConnectionWithTrackingWithoutGuarantee(
        const NetworkResponse& response);

    //! Sends a response packet to this connection
    //! \returns nullptr If this connection is closed
    DLLEXPORT std::shared_ptr<SentResponse> SendPacketToConnection(
        const std::shared_ptr<NetworkResponse>& response, RECEIVE_GUARANTEE guarantee);

    //! \brief Sends a keep alive packet if enough time has passed
    DLLEXPORT void SendKeepAlivePacket();

    //! \brief Sends a packet that tells the other side to disconnect
    //! \todo Add a message parameter for the reason
    DLLEXPORT void SendCloseConnectionPacket();

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
    DLLEXPORT void HandleRemoteAck(uint32_t localidconfirmedassent);


    //! \brief Basically a debug method (this is very slow)
    DLLEXPORT std::vector<uint32_t> GetCurrentlySentAcks();

    inline std::string GetRawAddress() const
    {
        return RawAddress;
    }

    //! \brief Returns a reference to a list of received packets that haven't been
    //! acknowledged successfully to the other side
    const auto& GetReceivedPackets() const
    {
        return ReceivedRemotePackets;
    }

    //! \brief Returns the pending requests
    const auto& GetPendingRequests() const
    {
        return PendingRequests;
    }

    //! \brief Returns responses that are tracked and haven't been acknowledged by the other
    //! side
    const auto& GetResponsesNeedingConfirmation() const
    {
        return ResponsesNeedingConfirmation;
    }

protected:
    //! \param alreadyreceived If true only the message is unpacked and discarded
    DLLEXPORT void _HandleRequestPacket(
        sf::Packet& packet, uint32_t messagenumber, bool alreadyreceived);

    //! \param alreadyreceived If true only the message is unpacked and discarded
    DLLEXPORT void _HandleResponsePacket(sf::Packet& packet, bool alreadyreceived);


    //! \brief Sets acks in a packet as properly sent in this
    //!
    //! Acks that were false in the packet are untouched
    DLLEXPORT void SetPacketsReceivedIfNotSet(NetworkAckField& acks);

    //! \brief Removes acks that were successful in the packet from target
    //!
    //! \note Should be called after the packet containing these acks is marked as
    //! successful
    DLLEXPORT void RemoveSucceededAcks(NetworkAckField& acks);

    //! \brief Sends actualpackettosend to our Owner's socket
    DLLEXPORT void _SendPacketToSocket(sf::Packet& actualpackettosend);


    //! Marks acks depending on packet to be lost
    DLLEXPORT void _FailPacketAcks(uint32_t packetid);

    //! \brief Returns true if we have already received message with number
    //! \note Will also store it for future lookups
    bool _IsAlreadyReceived(uint32_t messagenumber);

    //! \brief Closes the connection and reports an error
    DLLEXPORT void _OnRestrictFail(uint16_t type);

protected:
    DLLEXPORT bool _HandleInternalRequest(const std::shared_ptr<NetworkRequest>& request);

    DLLEXPORT bool _HandleInternalResponse(const std::shared_ptr<NetworkResponse>& response);

    //! \brief Sends a message again. Preserves message number but changes packet id
    //! \todo Create Resend method that pools together many failed messages
    void _Resend(SentRequest& toresend);

    void _Resend(SentResponse& toresend);

    template<class TSentType>
    void _HandleTimeouts(int64_t timems, std::vector<std::shared_ptr<TSentType>> sentthing);


    //! \brief Returns a request matching the response's reference ID or NULL
    std::shared_ptr<SentRequest> _GetPossibleRequestForResponse(
        const std::shared_ptr<NetworkResponse>& response);

    //! \brief Returns acks to be sent with a normal packet, or null if no acks to send
    //! \param autoaddtosent If true the generated ack field is added to SentAckPackets
    DLLEXPORT std::shared_ptr<NetworkAckField> _GetAcksToSend(
        uint32_t localpacketid, bool autoaddtosent = true);

    //! \brief Marks a remote id as received
    //!
    //! This function will also store the packetid for later checks
    //! \returns True If already received a packet with the id
    // bool _MarkNewAsReceived(uint32_t remotepacketid);

protected:
    //! The main state of connection
    CONNECTION_STATE State = CONNECTION_STATE::NothingReceived;

    //! Packets are handled by this object
    NetworkHandler* Owner = nullptr;

    //! Used to send acks for received remote packets
    NetworkAckField::PacketReceiveStatus ReceivedRemotePackets;

    //! Holds the ID of the last sent packet
    //! Incremented every time a packet is sent to keep local
    //! packet ids different
    //! \note The world will break once this wraps around and reaches 0
    //! \todo Fix that
    uint32_t LastUsedLocalID = PACKET_NUMBERING_OFFSET;

    //! Holds the number of last sent message (NetworkResponse or NetworkRequest object)
    uint32_t LastUsedMessageNumber = 0;

    //! Holds the id of last local sent packet that we have received an ack for
    uint32_t LastConfirmedSent = 0;


    //! Connections might have special restrictions on them
    //! This is mainly used to accept only remote console feature on clients
    CONNECTION_RESTRICTION RestrictType = CONNECTION_RESTRICTION::None;

    //! Flipped every time a packet is sent to toggle sending acks from the front or the back
    //! In normal operation doesn't matter but in exceptional circumstances
    //! allows more acks to be sent by sending 2 group of acks for each round while
    //! waiting for confirmation of ack receive
    bool FrontAcks = false;

    //! When acks pile up increase this value to send more acks
    //! \todo Implement this
    uint8_t ExtraAckCount = 0;

    //! These are used to time out the connection
    int64_t LastSentPacketTime = 0;
    int64_t LastReceivedPacketTime = 0;

    //! Sent requests that are waiting for a response
    std::vector<std::shared_ptr<SentRequest>> PendingRequests;

    //! Sent responses that need to be confirmed (or resent if they are lost)
    std::vector<std::shared_ptr<SentResponse>> ResponsesNeedingConfirmation;

    //! Holds sent ack groups until they are considered lost or
    //! received and then is used to mark the packets received by the
    //! other side as successfully sent
    std::vector<std::shared_ptr<SentAcks>> SentAckPackets;

    //! Numbers of messages that have been received before, used to skip processing duplicates
    //! \todo Implement a lower bound (under which everything is dropped) and make this smaller
    boost::circular_buffer<uint32_t> LastReceivedMessageNumbers{KEEP_IDS_FOR_DISCARD};

    //! The remote port
    uint16_t TargetPortNumber;

    //! Non-translated address string.
    //! Maybe empty
    std::string RawAddress;

    //! If the target remote was specified with an URL this stores it for
    //! later reconstruction (something like play.boostslair.com)
    std::string HostName;

    //! Resolved address of the remote
    sf::IpAddress TargetHost;

    //! True when TargetHost has been retrieved from HostName or TargetHost is
    //! made valid some other way
    bool AddressGot = false;

private:
    //! This holds the final packet data when sending. This is kept
    //! around to not need to allocate memory again for each sent
    //! packet
    sf::Packet StoredWireData;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Connection;
#endif
