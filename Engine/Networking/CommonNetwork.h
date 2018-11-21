// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once

#include <stdint.h>

//! \file Common types for networking

namespace Leviathan {

// magic numbers
constexpr uint16_t LEVIATHAN_NORMAL_PACKET = 0x4C6E;

constexpr uint16_t LEVIATHAN_ACK_PACKET = 0x4C61;

constexpr uint8_t NORMAL_RESPONSE_TYPE = 0x12;

constexpr uint8_t NORMAL_REQUEST_TYPE = 0x28;


//! Type of networked application
enum class NETWORKED_TYPE {

    Client,
    Server,
    Master,
    //! Only set when the derived class forgot to set it
    Error
};

enum class PACKET_TIMEOUT_STYLE : uint8_t {

    //! Loss is detected by packets received after and if a second has passed
    //! This makes all other styles deprecated
    Unified

    ////! Loss is detected by measuring time taken to complete
    ////! Use for non-realtime packets like connection attempts etc.
    // Timed,
    //
    ////! This style marks packets lost after a specific number of packets sent AFTER
    ////! this packet are confirmed to have been received by the other side
    // PacketsAfterReceived
};



//! \brief Controls whether a packet is critical
enum class RECEIVE_GUARANTEE {

    //! Packet is sent once and if lost won't be resent
    None,

    //! Packet will get a single resend
    ResendOnce,

    //! Packet must be received within a certain time or the connection
    //! is considered broken. Will resend until connection is closed
    Critical
};

//! \brief State of a connection's encryption
enum class CONNECTION_ENCRYPTION {

    //! The connection has not reached Secured state
    Undecided,

    //! The connection doesn't need encryption
    //! Must be agreed on by both the server and the client. Otherwise a forced
    //! disconnect must occur
    None,

    //! The standard preferred encyption. With both ends knowing each other's
    //! public key + an agreed symmetric key (AES) for less secure messages
    Standard

    //! Strong. Forced use of public keys for all secure messages
};

//! Allows servers to control who can join
enum class SERVER_JOIN_RESTRICT : uint8_t {

    //! Everyone can join the server
    None,
    Localhost,
    //! \todo A proper lan detection
    LAN,
    Whitelist,
    Invite,
    Friends,
    Permissions,
    Custom
};

//! Allows servers to tell clients what they are doing
enum class SERVER_STATUS : uint8_t {

    Starting,
    Running,
    Shutdown,
    Restart
};

//! Defines in what way a request was invalid can also define why a server disallowed a request
enum class NETWORK_RESPONSE_INVALIDREASON : uint8_t {

    //! Returned when the connection is anonymous
    //! (the other client hasn't requested verified connection)
    Unauthenticated,

    //! Returned when we don't implement the wanted action
    //! for example if we are asked our server status and we aren't a server
    Unsupported,

    //! Server has maximum number of players
    ServerFull,

    //! Server is not accepting players
    ServerNotAcceptingPlayers,

    //! The client isn't properly authenticated for that action or the server
    //! received mismatching security / id numbers
    NotAuthorized,

    //! The client has already connected to the server, and must disconnect before trying again
    ServerAlreadyConnectedToYou,

    //! The client has made a request with invalid or unsupported options
    InvalidParameters,

    //! The server has used a custom rule to disallow this
    ServerCustom
};

//! Defines what request the server accepted and any potential data
enum class SERVER_ACCEPTED_TYPE : uint8_t {

    //! Server has accepted your join request
    ConnectAccepted,

    //! Server has accepted the request and will handle it soon
    RequestQueued,

    //! The request is done
    Done
};

} // namespace Leviathan
