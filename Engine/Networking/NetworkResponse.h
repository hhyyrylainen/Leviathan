// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"
#include "Common/SFMLPackets.h"
#include "CommonNetwork.h"
#include "Exceptions.h"

#include "GameSpecificPacketHandler.h"

#include <memory>

namespace Leviathan {

//! \brief Defines the type of response that the packet contains
enum class NETWORK_RESPONSE_TYPE : uint16_t {

    //! Sent in response to a NETWORK_REQUEST_TYPE::Connect
    Connect,

    //! Sent in response to a NETWORK_REQUEST_TYPE::Security
    //! \todo Implemented packet breaking for this type
    Security,

    //! Sent in response to a NETWORK_REQUEST_TYPE::Authenticate
    Authenticate,

    //! Sent in response to a NETWORK_REQUEST_TYPE::Identification contains a user readable
    //! string, game name, game version and leviathan version strings
    Identification,

    //! Alias for None
    Keepalive,

    //! Immediately drops the connection when sent or received
    //! Uses class None
    CloseConnection,

    RemoteConsoleClosed,

    RemoteConsoleOpened,

    InvalidRequest,

    //! Sent by a server when it disallows a made request
    ServerDisallow,

    //! Sent by a server when a request is allowed
    ServerAllow,

    //! Returns anonymous data about the server
    //! members:
    //! ServerNameString Contains the name of the server, should be limited to max 100 letters
    //! Joinable States if the server is joinable (has started,
    //! doesn't take slots into account)
    //!
    //! JoinRestriction Defines the type of join authentication the server uses
    //!(restricts who can join)
    //!
    //! Players Current human players on the server
    //! MaxPlayers Maximum human players
    //! Bots Current bots on the server
    //! ServerStatus The current status of the server. Used to define what the server is doing
    //! AdditionalFlags The flags of the server. These can be used based on the game
    //! for example to define game mode or level requirements or something else
    ServerStatus,

    //! Sends a update/new SyncedValue
    SyncValData,

    //! Send after all SYNCVALDATA has been sent and indicates
    //! whether they should have arrived correctly
    SyncDataEnd,

    //! Contains SyncedResource update notification
    SyncResourceData,

    //! Contains a new NetworkedInput
    CreateNetworkedInput,

    //! Contains control state updates regarding a NetworkedInput
    UpdateNetworkedInput,

    //! Client sents this when they want input to be destroyed
    DisconnectInput,

    //! Contains information about a world that is about to be sent to the client
    StartWorldReceive,

    //! A new entity was created on the server
    EntityCreation,

    //! Contains update data for a single entity
    //! TickNumber The tick on which this was generated
    //!
    //!  ReferenceTick The tick number against which this update has been created
    //!  Special case is -1 which notes that there is no reference tick
    EntityUpdate,

    //! Contains (list) an ID for entity to be deleted
    EntityDestruction,

    //! Grants or revokes local control from a client (this is used to notify the clients what
    //! is the status)
    EntityLocalControlStatus,

    //! Contains an updated cache variable
    CacheUpdated,

    //! Contains the name of a removed cache variable
    CacheRemoved,

    // //! Instructs a world to create or destroy a constraint
    // //! Create When false the constraint is to be deleted
    // EntityConstraint,

    //! Sent when the server changes physics frozen state
    WorldFrozen,

    //! A server heartbeat packet
    ServerHeartbeat,

    //! Marks that the client is required to send heartbeats
    StartHeartbeats,

    //! Empty response, used for keeping alive/nothing
    None,

    //! The packet is a game specific packet!
    //! \see GameSpecificPacketHandler BaseGameSpecificFactory BaseGameSpecificResponsePacket
    Custom
};


//! Base class for all request objects
//! \note Even though it cannot be required by the base class, sub classes should
//! implement a constructor taking in an sf::Packet object
//! \todo Re-implement limiting string lengths in messages
class NetworkResponse {
public:
    NetworkResponse(NETWORK_RESPONSE_TYPE type, uint32_t responseid) :
        Type(type), ResponseID(responseid)
    {
        LEVIATHAN_ASSERT(responseid != static_cast<uint32_t>(-1),
            "Response should use 0 if in response to anything");
    }

    virtual ~NetworkResponse(){};

    inline void AddDataToPacket(sf::Packet& packet) const
    {
        packet << static_cast<uint16_t>(Type) << ResponseID;

        _SerializeCustom(packet);
    }

    inline NETWORK_RESPONSE_TYPE GetType() const
    {
        return Type;
    }

    //! \returns Name of this type. For prettier debug printing
    DLLEXPORT std::string GetTypeStr() const;

    inline auto GetResponseID() const
    {
        return ResponseID;
    }

    DLLEXPORT static std::shared_ptr<NetworkResponse> LoadFromPacket(sf::Packet& packet);

    //! \brief Limits size of response to avoid the application being used
    //! for DDoS amplification
    DLLEXPORT static void LimitResponseSize(
        ResponseIdentification& response, uint32_t maxsize);

protected:
    //! \brief Base classes serialize their data
    DLLEXPORT virtual void _SerializeCustom(sf::Packet& packet) const = 0;

    //! \brief Type of response. Specifies which subclass this object is
    const NETWORK_RESPONSE_TYPE Type;

    const uint32_t ResponseID = 0;
};

//! \brief Used for BaseGameSpecificResponsePacket storing
class ResponseCustom : public NetworkResponse {
public:
    ResponseCustom(
        uint32_t responseid, std::shared_ptr<GameSpecificPacketData> actualresponse) :
        NetworkResponse(NETWORK_RESPONSE_TYPE::Custom, responseid),
        ActualResponse(actualresponse)
    {}

    void _SerializeCustom(sf::Packet& packet) const override
    {

        LEVIATHAN_ASSERT(0, "_SerializeCustom called on ResponseCustom");
    }

    inline void AddDataToPacket(GameSpecificPacketHandler& handler, sf::Packet& packet)
    {

        packet << ResponseID << static_cast<uint16_t>(Type);

        handler.PassGameSpecificDataToPacket(ActualResponse.get(), packet);
    }

    ResponseCustom(
        GameSpecificPacketHandler& handler, uint32_t responseid, sf::Packet& packet) :
        NetworkResponse(NETWORK_RESPONSE_TYPE::Custom, responseid)
    {
        ActualResponse = handler.ReadGameSpecificPacketFromPacket(true, packet);
        if(!ActualResponse) {

            throw InvalidArgument("invalid packet format for user defined response");
        }
    }

    std::shared_ptr<GameSpecificPacketData> ActualResponse;
};

//! \brief Empty keep alive response
//!
//! Also used for all other responses that don't need any data members
class ResponseNone : public NetworkResponse {
public:
    ResponseNone(NETWORK_RESPONSE_TYPE actualtype, uint32_t responseid = 0) :
        NetworkResponse(actualtype, responseid)
    {}

    void _SerializeCustom(sf::Packet& packet) const override {}

    ResponseNone(NETWORK_RESPONSE_TYPE actualtype, uint32_t responseid, sf::Packet& packet) :
        NetworkResponse(actualtype, responseid)
    {}
};


// This file is generated by the script GenerateResponse.rb
// and contains implementations for all the response types
#include "../Generated/ResponseImpl.h"

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::NetworkResponse;
#endif
