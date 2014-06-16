#ifndef LEVIATHAN_NETWORKCLIENTINTERFACE
#define LEVIATHAN_NETWORKCLIENTINTERFACE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/BaseNotifiable.h"

#define DEFAULT_MAXCONNECT_TRIES		5


//! Defines the interval between heartbeats
//! Should be the same as SERVER_HEARTBEATS_MILLISECOND
#define CLIENT_HEARTBEATS_MILLISECOND			180


namespace Leviathan{



	//! \brief Class that encapsulates common networking functionality required by client programs
	//!
	//! More specific version of NetworkInterface and should be included additionally in client network interface classes.
	//! \see NetworkInterface
	class NetworkClientInterface : public BaseNotifiableAll{
	public:
		DLLEXPORT NetworkClientInterface();
		DLLEXPORT virtual ~NetworkClientInterface();


		//! \brief Connects the client to a server
		//! \return Returns true when successfully started the join process, false if already connected (DisconnectFromServer should be called)
		//! \param connectiontouse The connection object should be retrieved by calling NetworkHandler::GetOrCreatePointerToConnection
		DLLEXPORT bool JoinServer(shared_ptr<ConnectionInfo> connectiontouse);

		//! \brief Disconnects the client from the server or does nothing
		//! \todo Add a check to not close the connection if it is used by RemoteConsole
		DLLEXPORT FORCE_INLINE void DisconnectFromServer(const wstring &reason, bool connectiontimedout = false){
			GUARD_LOCK_THIS_OBJECT();
			DisconnectFromServer(guard, reason, connectiontimedout);
		}

		//! \brief Actual implementation of DisconnectFromServer
		DLLEXPORT void DisconnectFromServer(ObjectLock &guard, const wstring &reason, bool connectiontimedout = false);


		//! \brief Called directly by SyncedVariables to update the status string
		DLLEXPORT void OnUpdateFullSynchronizationState(size_t variablesgot, size_t expectedvariables);


		//! \brief Sends a command string to the server
		//!
		//! It should always be assumed that this function works. If it doesn't it is guaranteed that the client kicks itself because 
		//! the connection is lost.
		//! \exception ExceptionInvalidState if not connected to a server
		//! \param messagestr The UTF8 encoded string containing the command. This specifically uses utf8 to save space when sending long chat messages
		//! The maximum length is MAX_SERVERCOMMAND_LENGTH should be around 550 characters.
		//! \exception ExceptionInvalidArgument when the message string is too long
		DLLEXPORT void SendCommandStringToServer(const string &messagestr);


		//! \brief Returns a pointer to an instance if this application is a client
		DLLEXPORT static NetworkClientInterface* GetIfExists();


		//! \brief Closes all client related things
		DLLEXPORT void OnCloseClient();

		//! \brief Returns true if the client is connected to a server
		//!
		//! This will be true when the server has responded to our join request and allowed us to join, we aren't actually yet playing on the server
		DLLEXPORT bool IsConnected() const;

	protected:

		//! \brief Utility function for subclasses to call for default handling of server packets
		//!
		//! Handles default packets that are meant to be processed by a client
		DLLEXPORT bool _HandleClientRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connectiontosendresult);

		//! \brief Utility function for subclasses to call for default handling of non-request responses
		//!
		//! Handles default types of response packages and returns true if processed.
		DLLEXPORT bool _HandleClientResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection, bool &dontmarkasreceived);

		//! \brief Updates status of the client to server connections
		//!  
		//! \note Should be called by NetworkInterface::TickIt
		DLLEXPORT void UpdateClientStatus();

		// Callbacks for child classes to implement //
		DLLEXPORT virtual void _OnDisconnectFromServer(const wstring &reasonstring, bool donebyus);
		DLLEXPORT virtual void _OnStartConnectToServer();
		DLLEXPORT virtual void _OnFailedToConnectToServer(const wstring &reason);
		DLLEXPORT virtual void _OnSuccessfullyConnectedToServer();
		//! \brief Called when this class generates a new update message
		DLLEXPORT virtual void _OnNewConnectionStatusMessage(const wstring &message);


		//! \brief Callback used to know when our connection is closed
		DLLEXPORT virtual void _OnNotifierDisconnected(BaseNotifierAll* parenttoremove);

		//! \brief Called when the server has confirmed the join and we are a player on the server
		//!
		//! By default this will synchronize game variables and call the _OnLobbyJoin function (which can then handle match joining)
		//! \todo Do what this should do
		DLLEXPORT virtual void _OnProperlyConnected();


		//! \brief Called when the player is on the server and everything that the Engine is concerned about is done
		//! \note Here the application's connect data should be sent. The application specific connection routine should be done here
		DLLEXPORT virtual void _OnStartApplicationConnect() = 0;


		//! \brief Returns the ID that the server has assigned to us
		DLLEXPORT virtual int GetOurID() const;


	private:
		
		void _SendConnectRequest(ObjectLock &guard);

		//! \brief Handles succeeded requests, removes clutter from other places
		void _ProcessCompletedRequest(shared_ptr<SentNetworkThing> tmpsendthing, ObjectLock &guard);

		//! \brief Handles failed requests, removes clutter from other places
		void _ProcessFailedRequest(shared_ptr<SentNetworkThing> tmpsendthing, ObjectLock &guard);

		//! \brief Internally called when server has accepted us
		//! \todo Call variable syncing from here
		void _ProperlyConnectedToServer(ObjectLock &guard);

		//! \brief Called when we receive a start heartbeat packet
		void _OnStartHeartbeats();


		//! \brief Called when a heartbeat is received
		void _OnHeartbeat();


		//! \brief Updates the heartbeat states
		void _UpdateHeartbeats();

	protected:

		//! This vector holds the made requests to allow using the response to do stuff
		std::vector<shared_ptr<SentNetworkThing>> OurSentRequests;

		shared_ptr<ConnectionInfo> ServerConnection;

		bool ConnectedToServer;

		int ConnectTriesCount;
		int MaxConnectTries;


		//! Marks whether heartbeats are in use
		bool UsingHeartbeats;

		//! The last time a heartbeat packet was received
		WantedClockType::time_point LastReceivedHeartbeat;

		//! The last time a heartbeat was sent
		WantedClockType::time_point LastSentHeartbeat;


		//! Holds the time for how long we have been without a heartbeat
		float SecondsWithoutConnection;


		//! Our player id, this is required for some requests
		int OurPlayerID;


		//! Static access for utility classes
		static NetworkClientInterface* Staticaccess;

	};

}
#endif