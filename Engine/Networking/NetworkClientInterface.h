#ifndef LEVIATHAN_NETWORKCLIENTINTERFACE
#define LEVIATHAN_NETWORKCLIENTINTERFACE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{



	//! \brief Class that encapsulates common networking functionality required by client programs
	//!
	//! More specific version of NetworkInterface and should be included additionally in client network interface classes.
	//! \see NetworkInterface
	class NetworkClientInterface{
	public:
		DLLEXPORT NetworkClientInterface();
		DLLEXPORT virtual ~NetworkClientInterface();


		//! \brief Connects the client to a server
		//! \return Returns true when successfully started the join process, false if already connected (DisconnectFromServer should be called)
		//! \param connectiontouse The connection object should be retrieved by calling NetworkHandler::GetOrCreatePointerToConnection
		DLLEXPORT bool JoinServer(shared_ptr<ConnectionInfo> connectiontouse);

		//! \brief Disconnects the client from the server or does nothing
		DLLEXPORT void DisconnectFromServer();

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
		DLLEXPORT virtual void _OnDisconnectFromServer(const wstring &reasonstring);
		DLLEXPORT virtual void _OnStartConnectToServer();
		DLLEXPORT virtual void _OnFailedToConnectToServer(const wstring &reason);
		DLLEXPORT virtual void _OnSuccessfullyConnectedToServer();


		// ------------------------------------ //





	};

}
#endif