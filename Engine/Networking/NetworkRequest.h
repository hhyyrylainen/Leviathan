// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/SFMLPackets.h"
#include "CommonNetwork.h"
#include "Exceptions.h"

#include "GameSpecificPacketHandler.h"

#include <memory>

namespace Leviathan {

enum class NETWORK_REQUEST_TYPE : uint16_t {

    //! Opening a connection
    Connect,

    //! Only one side of the connection can send this request, usually the client
    Security,

    //! Only the client may make this call, after this the Connection won't restrict
    //! any packets from being received
    Authenticate,

    //! This may be sent after CONNECTION_STATE::Connected has been reached
    //! "PongServer running version 0.5.1.0, status: 0/20"
    Identification,

    Serverstatus,

    RemoteConsoleOpen,

    RemoteConsoleAccess,

    CloseRemoteConsole,

    //! The receiving side is now allowed to open a remote console with the token
    DoRemoteConsoleOpen,

    //! Client wants to join a server
    //! MasterServerToken The ID given by the master server
    JoinServer,

    //! Client requests to join active gamestate (if the options is not empty then the client
    //! is asking to join something specific)
    JoinGame,

    GetSingleSyncValue,

    GetAllSyncValues,

    //! Used to request the server to run a command, used for chat and other things
    //! \todo Implement 	if(Command.length() > MAX_SERVERCOMMAND_LENGTH)
    RequestCommandExecution,

    //! Sent when a player requests the server to connect a NetworkedInput
    ConnectInput,

    //! Sent by servers to ping (time the time a client takes to respond) clients
    Echo,

    //! Contains timing data to sync world clocks on a client
    //! Ticks The amount of ticks to set or change by
    //! Absolute Whether the tick count should be set to be the current or
    //! just added to the current tick
    //!
    //! EngineMSTweak The engine tick tweaking, this should only be
    //! applied by a single GameWorld
    WorldClockSync,

    //! Used for game specific requests
    Custom
};

//! Base class for all request objects
//! \note Even though it cannot be required by the base class, sub classes should
//! implement a constructor taking in an sf::Packet object
class NetworkRequest {
public:
    NetworkRequest(NETWORK_REQUEST_TYPE type, uint32_t receivedmessagenumber = 0) :
        Type(type), MessageNumber(receivedmessagenumber)
    {}

    virtual ~NetworkRequest(){};

    inline void AddDataToPacket(sf::Packet& packet) const
    {
        packet << static_cast<uint16_t>(Type);

        _SerializeCustom(packet);
    }

    inline NETWORK_REQUEST_TYPE GetType() const
    {
        return Type;
    }

    //! \returns Name of this type. For prettier debug printing
    DLLEXPORT std::string GetTypeStr() const;

    inline uint32_t GetMessageNumber() const
    {
        return MessageNumber;
    }

    //! \brief The id number for a response to this is the same as the message number that this
    //! request is in
    inline uint32_t GetIDForResponse() const
    {
        return MessageNumber;
    }

    DLLEXPORT static std::shared_ptr<NetworkRequest> LoadFromPacket(
        sf::Packet& packet, uint32_t messagenumber);

protected:
    //! \brief Base classes serialize their data
    DLLEXPORT virtual void _SerializeCustom(sf::Packet& packet) const = 0;

    const NETWORK_REQUEST_TYPE Type;

    //! This is only valid when this is received
    const uint32_t MessageNumber = 0;
};

class RequestCustom : public NetworkRequest {
public:
    RequestCustom(std::shared_ptr<GameSpecificPacketData> actualrequest) :
        NetworkRequest(NETWORK_REQUEST_TYPE::Custom), ActualRequest(actualrequest)
    {}

    void _SerializeCustom(sf::Packet& packet) const override
    {

        LEVIATHAN_ASSERT(0, "_SerializeCustom called on RequestCustom");
    }

    RequestCustom(GameSpecificPacketHandler& handler, sf::Packet& packet) :
        NetworkRequest(NETWORK_REQUEST_TYPE::Custom)
    {
        ActualRequest = handler.ReadGameSpecificPacketFromPacket(false, packet);

        if(!ActualRequest) {

            throw InvalidArgument("invalid packet format for user defined request");
        }
    }

    inline void AddDataToPacket(GameSpecificPacketHandler& handler, sf::Packet& packet)
    {

        packet << static_cast<uint16_t>(Type);

        handler.PassGameSpecificDataToPacket(ActualRequest.get(), packet);
    }

    std::shared_ptr<GameSpecificPacketData> ActualRequest;
};

//! \brief Empty request for ones that require no data
//!
//! Also used for all other request that don't need any data members
class RequestNone : public NetworkRequest {
public:
    RequestNone(NETWORK_REQUEST_TYPE actualtype) : NetworkRequest(actualtype) {}

    void _SerializeCustom(sf::Packet& packet) const override {}

    RequestNone(NETWORK_REQUEST_TYPE actualtype, uint32_t idforresponse, sf::Packet& packet) :
        NetworkRequest(actualtype, idforresponse)
    {}
};



// This file is generated by the script GenerateRequest.rb
// and contains implementations for all the response types
#include "../Generated/RequestImpl.h"


} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::NetworkRequest;
#endif
