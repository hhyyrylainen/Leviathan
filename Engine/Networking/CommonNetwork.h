// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen

//! \file Common types for networking

namespace Leviathan{

//! Type of networked application
enum class NETWORKED_TYPE {
    
    Client,
    Server,
    Master, 
    //! Only set when the derived class forgot to set it
    Error
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
enum class SERVER_JOIN_RESTRICT {

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
enum class SERVER_STATUS {

    Starting,
    Running,
    Shutdown,
    Restart
 };


}
