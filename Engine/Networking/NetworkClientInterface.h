// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "NetworkInterface.h"
#include "TimeIncludes.h"

#include <memory>
#include <vector>

namespace Leviathan {

class SentRequest;

//! \brief Class that encapsulates common networking functionality required by
//! client programs
//!
//! More specific version of NetworkInterface and should be included
//! additionally in client network interface classes.
//! \see NetworkInterface
class NetworkClientInterface : public NetworkInterface {
public:
    enum class CLIENT_CONNECTION_STATE {

        //! Not connected to a server
        None,

        //! JoinServer has been called and the client is now waiting for the connection
        //! object to become open
        WaitingForOpening,

        //! Sent join request, waiting for it to succeed
        SentJoinRequest,

        //! Properly connected
        Connected,

        //! Client is leaving a server
        Closed
    };

public:
    DLLEXPORT NetworkClientInterface();
    DLLEXPORT virtual ~NetworkClientInterface();


    DLLEXPORT virtual void HandleRequestPacket(
        const std::shared_ptr<NetworkRequest>& request, Connection& connection) override;

    DLLEXPORT virtual void HandleResponseOnlyPacket(
        const std::shared_ptr<NetworkResponse>& message, Connection& connection) override;

    //! \brief Connects the client to a server
    //! \return Returns true when successfully started the join process,
    //! false if already connected (and DisconnectFromServer should be called)
    //! \param connectiontouse The connection object should be retrieved by calling
    //! NetworkHandler::GetOrCreatePointerToConnection
    DLLEXPORT bool JoinServer(std::shared_ptr<Connection> connectiontouse);

    //! \brief Disconnects the client from the server or does nothing
    //! \todo Add a check to not close the connection if it is used by RemoteConsole
    DLLEXPORT void DisconnectFromServer(
        const std::string& reason, bool connectiontimedout = false);

    DLLEXPORT virtual std::vector<std::shared_ptr<Connection>>&
        GetClientConnections() override;

    //! \brief Called directly by SyncedVariables to update the status string
    DLLEXPORT void OnUpdateFullSynchronizationState(
        size_t variablesgot, size_t expectedvariables);


    //! \brief Sends a command string to the server
    //!
    //! It should always be assumed that this function works.
    //! If it doesn't it is guaranteed that the client kicks itself
    //! because the connection is lost.
    //! \exception ExceptionInvalidState if not connected to a server
    //! The maximum length is MAX_SERVERCOMMAND_LENGTH should be around 550 characters.
    //! \exception ExceptionInvalidArgument when the message string is too long
    DLLEXPORT void SendCommandStringToServer(const std::string& messagestr);


    //! \brief Closes all client related things
    DLLEXPORT virtual void CloseDown() override;

    //! \brief Returns true if the client is connected to a server
    //!
    //! This will be true when the server has responded to our join
    //! request and allowed us to join, we aren't actually yet playing
    //! on the server
    DLLEXPORT bool IsConnected() const;

    //! \brief Returns the ID that the server has assigned to us
    DLLEXPORT virtual int GetOurID() const;

    //! \brief Returns the active server connection or NULL
    DLLEXPORT virtual std::shared_ptr<Connection> GetServerConnection();

    //! \brief Marks a keep alive to be sent on next tick
    DLLEXPORT void MarkForNotifyReceivedStates();

    //! \brief Updates status of the client to server connections
    //!
    //! \note Should be called by NetworkInterface::TickIt
    DLLEXPORT void TickIt() override;

    auto GetServerConnectionState() const
    {
        return ConnectState;
    }

protected:
    // Callbacks for child classes to implement //
    DLLEXPORT virtual void _OnDisconnectFromServer(
        const std::string& reasonstring, bool donebyus)
    {}
    DLLEXPORT virtual void _OnStartConnectToServer() {}
    DLLEXPORT virtual void _OnFailedToConnectToServer(const std::string& reason) {}
    DLLEXPORT virtual void _OnSuccessfullyConnectedToServer() {}

    DLLEXPORT virtual void _OnCloseDown() {}

    //! \brief Called when this class generates a new update message
    DLLEXPORT virtual void _OnNewConnectionStatusMessage(const std::string& message) {}

    //! \brief This is used to get the target window for a world that we have been notified
    //! that we have joined and will receive updates
    DLLEXPORT virtual Window* GetWindowForWorldJoin(const std::string& extraoptions);

    //! \brief This is called to get physics materials for a world
    //!
    //! If this returns null physics won't be initialized for the received world.
    DLLEXPORT virtual std::shared_ptr<PhysicsMaterialManager>
        GetPhysicsMaterialsForReceivedWorld(
            int32_t worldtype, const std::string& extraoptions);

    //! \brief Called when the server has confirmed the join and we are a player on the
    //! server
    //!
    //! This is where the game needs to request joining a GameWorld or do some other game
    //! specific logic
    DLLEXPORT virtual void _OnProperlyConnected() = 0;

    //! \brief This is a helper for _OnProperlyConnected to
    DLLEXPORT std::shared_ptr<SentRequest> DoJoinDefaultWorld();

    //! \brief Called when joining a world has succeeded and the client has created the
    //! clientside world object. This should attach the world to be drawn and do any special
    //! setup required on it
    DLLEXPORT virtual void _OnWorldJoined(std::shared_ptr<GameWorld> world);

    //! \brief This is called when we receive a packet from the server indicating that we
    //! should create a new world and expect to receive updates for it
    DLLEXPORT virtual void _HandleWorldJoinResponse(
        int32_t worldid, int32_t worldtype, const std::string& extraoptions);

    //! \brief This needs to lookup a GameWorld for an entity update message by id
    //!
    //! The default implementation only looks at OurReceivedWorld
    DLLEXPORT virtual GameWorld* _GetWorldForEntityMessage(int32_t worldid);

    //! \brief Called when local control has been changed
    DLLEXPORT virtual void _OnLocalControlChanged(GameWorld* world);

    //! \brief Called when an entity creation packet has been passed to the GameWorld.
    DLLEXPORT virtual void _OnEntityReceived(GameWorld* world, ObjectID created);

private:
    //! \brief Helper for TickIt to handle server connection state
    //! \todo Why are both heartbeats and keepalives used?
    void _TickServerConnectionState();

    //! \brief Handles succeeded requests, removes clutter from other places
    void _ProcessCompletedRequest(
        std::shared_ptr<SentRequest> tmpsendthing, std::shared_ptr<NetworkResponse> response);

    //! \brief Handles failed requests, removes clutter from other places
    //! Only requests that aren't always sent as RECEIVE_GUARANTEE::Critical are handled here.
    //! That's because those will immediately drop the connection if they fail.
    void _ProcessFailedRequest(
        std::shared_ptr<SentRequest> tmpsendthing, std::shared_ptr<NetworkResponse> response);

    //! \brief Internally called when server has accepted us
    void _ProperlyConnectedToServer();

    //! \brief Called when we receive a start heartbeat packet
    void _OnStartHeartbeats();


    //! \brief Called when a heartbeat is received
    void _OnHeartbeat();


    //! \brief Updates the heartbeat states
    void _UpdateHeartbeats();

protected:
    //! This vector holds the made requests to allow using the response to do stuff
    std::vector<std::shared_ptr<SentRequest>> OurSentRequests;

    CLIENT_CONNECTION_STATE ConnectState = CLIENT_CONNECTION_STATE::None;
    std::shared_ptr<Connection> ServerConnection;

    //! Marks whether heartbeats are in use
    bool UsingHeartbeats = false;

    //! The last time a heartbeat packet was received
    WantedClockType::time_point LastReceivedHeartbeat;

    //! The last time a heartbeat was sent
    WantedClockType::time_point LastSentHeartbeat;

    //! True when a keep alive should be sent on next tick
    bool KeepAliveQueued = false;


    //! Holds the time for how long we have been without a heartbeat
    float SecondsWithoutConnection = 0.f;


    //! Our player id, this is required for some requests
    int OurPlayerID = -1;

    //! Our current received world. This is kept here to automatically apply update messages to
    //! it.
    std::shared_ptr<GameWorld> OurReceivedWorld;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::NetworkClientInterface;
#endif
