// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "NetworkInterface.h"
#include "NetworkResponse.h"


#include "ConnectedPlayer.h"

#include "Entities/EntityCommon.h"


namespace Leviathan{


//! \brief Class that encapsulates common networking functionality
//! required by server programs
//!
//! More specific version of NetworkInterface and should be included additionally in
//! server network interface classes.
//! \see NetworkInterface
class NetworkServerInterface : public NetworkInterface{
    friend ConnectedPlayer;
public:
    //! \brief Initializes some values to defaults and requires others to be provided
    //! by the subclass that inherits from this.
    //!
    //! These can be later changed with various set functions
    //! \param maxplayers Sets the initial maximum player count
    //! \param servername Sets the server's name visible in various listings
    //! \param restricttype Controls who can join the server
    //! \param additionalflags Sets the application specific flags for this server
    DLLEXPORT NetworkServerInterface(int maxplayers, const std::string &servername,
        SERVER_JOIN_RESTRICT restricttype = SERVER_JOIN_RESTRICT::Localhost,
        int additionalflags = 0);
        
    //! Default destructor
    DLLEXPORT virtual ~NetworkServerInterface();


    //! \brief Updates status of the status of the server's clients
    //!  
    //! \note Should be called by NetworkInterface::TickIt
    DLLEXPORT void UpdateServerStatus();

    //! \brief Gets corresponding player from a connection
    //! \return Returns a pointer from PlayerList
    DLLEXPORT std::shared_ptr<ConnectedPlayer> GetPlayerForConnection(
        Connection &connection);

    DLLEXPORT virtual std::vector<std::shared_ptr<Connection>>& GetClientConnections()
        override;

    //! \brief Sends a response to a NETWORKREQUESTTYPE_SERVERSTATUS
    DLLEXPORT void RespondToServerStatusRequest(std::shared_ptr<NetworkRequest> request,
        Connection &connectiontouse);

    //! \brief Sets the status of the server to newstatus
    DLLEXPORT void SetServerStatus(SERVER_STATUS newstatus);

    //! \brief Sets whether the server allows new players
    DLLEXPORT void SetServerAllowPlayers(bool allowingplayers);

    //! \brief Call this before shutting down the server to kick all players properly
    //! \todo Actually call this, maybe make this an event listener
    DLLEXPORT virtual void CloseDown() override;

    //! \brief Sends a response packet to all players except for the player(s)
    //! whose connection matches skipme
    DLLEXPORT void SendToAllButOnePlayer(NetworkResponse &response,
        Connection* skipme, RECEIVE_GUARANTEE guarantee);

    //! \brief Sends a response packet to all of the players
    DLLEXPORT void SendToAllPlayers(NetworkResponse &response, RECEIVE_GUARANTEE guarantee);

    //! \brief Verifies that all current players are receiving world updates
    //! \note Prior to calling this (if your players will move)
    //! you should bind positionable objects to the players for them to receive updates
    //! based on their location
    DLLEXPORT virtual void VerifyWorldIsSyncedWithPlayers(std::shared_ptr<GameWorld> world);

protected:


    //! \brief Utility function for subclasses to call for default handling of server packets
    //!
    //! Handles default packets that are meant to be processed by a server
    DLLEXPORT bool _HandleServerRequest(std::shared_ptr<NetworkRequest> request,
        Connection &connectiontosendresult);

    //! \brief Utility function for subclasses to call for default handling of non-request responses
    //!
    //! Handles default types of response packages and returns true if processed.
    DLLEXPORT bool _HandleServerResponseOnly(std::shared_ptr<NetworkResponse> message,
        Connection &connection, bool &dontmarkasreceived);

    //! \brief Used to handle server join request packets
    //! \todo Check connection security status
    //! \todo Add basic connection checking and master server authentication check
    DLLEXPORT void _HandleServerJoinRequest(std::shared_ptr<NetworkRequest> request,
        Connection &connection);

    // Callback functions //

    //! \brief Called when a player is about to connect
    DLLEXPORT virtual void PlayerPreconnect(Connection &connection,
        std::shared_ptr<NetworkRequest> joinrequest);
    DLLEXPORT virtual void _OnPlayerConnected(Lock &guard, 
        std::shared_ptr<ConnectedPlayer> newplayer);
    DLLEXPORT virtual void _OnPlayerDisconnect(Lock &guard, 
        std::shared_ptr<ConnectedPlayer> newplayer);
    DLLEXPORT virtual bool PlayerPotentiallyKicked(ConnectedPlayer* player);

    DLLEXPORT virtual void _OnCloseDown(){}


    //! \brief Called when the application should register custom command handling providers
    //! \note If you actually want to use this
    //! call to this should be added to your subclass constructor
    DLLEXPORT virtual void RegisterCustomCommandHandlers(CommandHandler* addhere);

    //! \brief Called by _HandleServerJoinRequest to allow specific games to disallow players
    //! \return true to allow join false to disallow join
    //! \param message The error message to give back to the player
    DLLEXPORT virtual bool AllowPlayerConnectVeto(std::shared_ptr<NetworkRequest> request,
        Connection &connection, std::string &message);

    //! Internally called when a player is about to be deleted
    //!
    //! Will call virtual notify functions
    void _OnReportCloseConnection(std::shared_ptr<ConnectedPlayer> plyptr, Lock &guard);

    //! Internally used to detect when a new player has connected
    void _OnReportPlayerConnected(std::shared_ptr<ConnectedPlayer> plyptr, Connection &connection,
        Lock &guard);

    // ------------------------------------ //


    // Server variables //

    //! Holds the list of currently connected players
    std::vector<std::shared_ptr<ConnectedPlayer>> ServerPlayers;
    //! All the connections used in ServerPlayers
    std::vector<std::shared_ptr<Connection>> ServerPlayersConnections;

    //! Maximum allowed player count
    int MaxPlayers = 20;

    //! Currently active bots
    std::vector<int> ActiveBots;

    //! Name displayed in the server list
    std::string ServerName = "Default Name";

    //! Controls whether players can join
    //! Used to prevent players joining during start up
    bool AllowJoin = false;

    //! Lock this when changing the player list
    Mutex PlayerListLocked;
    
    //! Type of join restriction, defaults to NETWORKRESPONSE_SERVERJOINRESTRICT_NONE
    SERVER_JOIN_RESTRICT JoinRestrict = SERVER_JOIN_RESTRICT::Localhost;
    
    //! Server status, initially NETWORKRESPONSE_SERVERSTATUS_STARTING and
    //! should be set by the application when appropriate
    SERVER_STATUS ServerStatus = SERVER_STATUS::Starting;

    //! This can contain anything the specific game wants
    int ExtraServerFlags = 0;

    //! Player ID counter for assigning unique ids for all players
    int CurrentPlayerID = 1000;

    //! The object used to handle all player submitted commands
    std::shared_ptr<CommandHandler> _CommandHandler = nullptr;
};

}

