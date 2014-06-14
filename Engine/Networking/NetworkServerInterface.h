#ifndef LEVIATHAN_NETWORKSERVERINTERFACE
#define LEVIATHAN_NETWORKSERVERINTERFACE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NetworkResponse.h"
#include "Common/BaseNotifiable.h"
#include "Gameplay/CommandHandler.h"

//! Defines the interval between heartbeats
//! Should be the same as CLIENT_HEARTBEATS_MILLISECOND
#define SERVER_HEARTBEATS_MILLISECOND			180


namespace Leviathan{

	//! \brief A class that represents a human player
	//! \todo Add a kick method and use it in NetworkServerInterface::CloseDownServer
	//! \todo Make _OnNotifierDisconnected set a disconnected flag that the server can kick based on upon
	class ConnectedPlayer : public BaseNotifiableAll, public CommandSender{
	public:
		DLLEXPORT ConnectedPlayer(ConnectionInfo* unsafeconnection, NetworkServerInterface* owninginstance);
		//! \brief Empty destructor for exporting
		DLLEXPORT ~ConnectedPlayer();

		//! \brief Checks is the given connection same as ours
		//! \warning This has bad performance see if IsConnectionYoursPtrCompare suits your use case
		DLLEXPORT bool IsConnectionYours(ConnectionInfo* checkconnection);

		//! \brief Checks is a connection this player's connection
		//! \note This compares the pointers and it should work but the IsConnectionYours is more secure (but it shouldn't be used)
		//! \see IsConnectionYours
		DLLEXPORT bool IsConnectionYoursPtrCompare(ConnectionInfo* checkconnection);

		//! \brief Returns whether the connection for this player is closed
		DLLEXPORT bool IsConnectionClosed() const;

		//! \brief Call this when the player is kicked
		//! \todo Add the reason to the packet
		DLLEXPORT void OnKicked(const wstring &reason);


		//! \brief Starts requiring the player to send heartbeats
		DLLEXPORT void StartHeartbeats();

		//! \brief Call this when a heartbeat is received
		DLLEXPORT void HeartbeatReceived();

		//! \brief Call this at any appropriate time to update heartbeat statistics
		DLLEXPORT void UpdateHeartbeats();


		DLLEXPORT virtual const string& GetUniqueName();

		DLLEXPORT virtual const string& GetNickname();

		DLLEXPORT virtual COMMANDSENDER_PERMISSIONMODE GetPermissionMode();

		DLLEXPORT virtual bool SendPrivateMessage(const string &message);

	protected:
		//! \brief Used to detect when a connection has been closed
		virtual void _OnNotifierDisconnected(BaseNotifierAll* parenttoremove);

		// ------------------------------------ //

		ConnectionInfo* CorrenspondingConnection;
		NetworkServerInterface* Owner;
		bool ConnectionStatus;


		//! The last time a heartbeat packet was received
		WantedClockType::time_point LastReceivedHeartbeat;

		//! Marks whether heartbeats are in use
		bool UsingHeartbeats;


		//! The unique identifier of the player, might be a steam id or something else
		string UniqueName;


		//! The display name of the player
		string DisplayName;


		//! Gets set when a heartbeat hasn't been received for a while, this will be set before the player is kicked
		bool IsControlLost;

		//! How long it has been since a heartbeat
		float SecondsWithoutConnection;

		//! The last time a heartbeat was sent
		WantedClockType::time_point LastSentHeartbeat;

	};

	//! \brief Class that encapsulates common networking functionality required by server programs
	//!
	//! More specific version of NetworkInterface and should be included additionally in server network interface classes.
	//! \see NetworkInterface
	class NetworkServerInterface : public virtual ThreadSafe{
		friend ConnectedPlayer;
	public:
		//! \brief Initializes some values to defaults and requires others to be provided by the subclass that inherits from this.
		//!
		//! These can be later changed with various set functions
		//! \param maxplayers Sets the initial maximum player count
		//! \param servername Sets the server's name visible in various listings
		//! \param restricttype Controls who can join the server
		//! \param additionalflags Sets the application specific flags for this server
		DLLEXPORT NetworkServerInterface(int maxplayers, const wstring &servername, NETWORKRESPONSE_SERVERJOINRESTRICT restricttype =
			NETWORKRESPONSE_SERVERJOINRESTRICT_NONE, int additionalflags = 0);
		//! Default destructor
		DLLEXPORT virtual ~NetworkServerInterface();


		//! \brief Updates status of the status of the server's clients
		//!  
		//! \note Should be called by NetworkInterface::TickIt
		DLLEXPORT void UpdateServerStatus();

		//! \brief Gets corresponding player from a connection
		//! \return Returns a pointer from PlayerList
		DLLEXPORT ConnectedPlayer* GetPlayerForConnection(ConnectionInfo* connection);


		//! \brief Sends a response to a NETWORKREQUESTTYPE_SERVERSTATUS
		DLLEXPORT void RespondToServerStatusRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connectiontouse);

		//! \brief Sets the status of the server to newstatus
		DLLEXPORT void SetServerStatus(NETWORKRESPONSE_SERVERSTATUS newstatus);

		//! \brief Sets whether the server allows new players
		DLLEXPORT void SetServerAllowPlayers(bool allowingplayers);

		//! \brief Call this before shutting down the server to kick all players properly
		//! \todo Actually call this, maybe make this an event listener
		DLLEXPORT void CloseDownServer();

	protected:


		//! \brief Utility function for subclasses to call for default handling of server packets
		//!
		//! Handles default packets that are meant to be processed by a server
		DLLEXPORT bool _HandleServerRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connectiontosendresult);

		//! \brief Utility function for subclasses to call for default handling of non-request responses
		//!
		//! Handles default types of response packages and returns true if processed.
		DLLEXPORT bool _HandleServerResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection, bool &dontmarkasreceived);

		//! \brief Used to handle server join request packets
		//! \todo Check connection security status
		//! \todo Add basic connection checking and master server authentication check
		DLLEXPORT void _HandleServerJoinRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connection);

		// Callback functions //

		//! \brief Called when a player is about to connect
		DLLEXPORT virtual void PlayerPreconnect(ConnectionInfo* connection, shared_ptr<NetworkRequest> joinrequest);
		DLLEXPORT virtual void _OnPlayerConnected(ConnectedPlayer* newplayer);
		DLLEXPORT virtual void _OnPlayerDisconnect(ConnectedPlayer* newplayer);
		DLLEXPORT virtual bool PlayerPotentiallyKicked(ConnectedPlayer* player);

		//! \brief Called when the application should register custom command handling providers
		//! \note If you actually want to use this call to this should be added to your subclass constructor
		DLLEXPORT virtual void RegisterCustomCommandHandlers(CommandHandler* addhere);

		//! \brief Called by _HandleServerJoinRequest to allow specific games to disallow players
		//! \return true to allow join false to disallow join
		//! \param message The error message to give back to the player
		DLLEXPORT virtual bool AllowPlayerConnectVeto(shared_ptr<NetworkRequest> request, ConnectionInfo* connection, wstring &message);


		//! \brief Called by ConnectedPlayer when it's connection closes
		std::vector<ConnectedPlayer*>::iterator _OnReportCloseConnection(const std::vector<ConnectedPlayer*>::iterator &iter, ObjectLock &guard);

		// ------------------------------------ //


		// Server variables //

		//! Holds the list of currently connected players
		std::vector<ConnectedPlayer*> PlayerList;
		//! Maximum allowed player count
		int MaxPlayers;
		//! Currently active bots
		vector<int> ActiveBots;

		//! Name displayed in the server list
		wstring ServerName;

		//! Controls whether players can join
		bool AllowJoin;

		//! Type of join restriction, defaults to NETWORKRESPONSE_SERVERJOINRESTRICT_NONE
		NETWORKRESPONSE_SERVERJOINRESTRICT JoinRestrict;
		//! Server status, initially NETWORKRESPONSE_SERVERSTATUS_STARTING and should be set by the application when appropriate
		NETWORKRESPONSE_SERVERSTATUS ServerStatus;

		//! This can contain anything the specific game wants
		int ExtraServerFlags;


		//! The object used to handle all player submitted commands
		CommandHandler* _CommandHandler;

	};

}
#endif
