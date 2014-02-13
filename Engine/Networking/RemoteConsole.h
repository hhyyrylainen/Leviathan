#ifndef LEVIATHAN_REMOTECONSOLE
#define LEVIATHAN_REMOTECONSOLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SFML/Network/IpAddress.hpp"
#include "Common/BaseNotifiable.h"


namespace Leviathan{

	class RemoteConsoleSession{
		friend RemoteConsole;
	public:
		DLLEXPORT RemoteConsoleSession(const wstring &name, ConnectionInfo* connection, int token);
		DLLEXPORT ~RemoteConsoleSession();

		DLLEXPORT ConnectionInfo* GetConnection();
		DLLEXPORT void ResetConnection();

		//! \brief Sets the connection as closing
		DLLEXPORT void KillConnection();

	private:
		wstring ConnectionName;
		int SessionToken;
		ConnectionInfo* CorrespondingConnection;

		//! Marks if we can send stuff here //
		bool IsOpened;
		//! \brief Sets connection as terminating
		//!
		//! Actual termination will happen next time RemoteConsole::UpdateStatus is called
		bool TerminateSession;
	};


	//! Class used to handle remote server commands and receiving messages
	//!
	//! Doesn't actually use reference counting. Inherits BaseNotifiable to be able to receive messages when ConnectionInfo closes
	class RemoteConsole : public BaseNotifiableAll{
		friend Engine;

		struct RemoteConsoleExpect{
			RemoteConsoleExpect(const wstring &name, int token, bool onlylocalhost, const MillisecondDuration &timeout);


			wstring ConnectionName;
			int SessionToken;
			bool OnlyLocalhost;

			WantedClockType::time_point TimeoutTime;
		};
	public:
		DLLEXPORT RemoteConsole();
		DLLEXPORT ~RemoteConsole();

		//! \brief Called before packets are handled
		//!
		//! Checks statuses of remote connections and can perform some special tasks such as closing ConnectionInfo objects
		DLLEXPORT void UpdateStatus();


		// Handle functions for interface to use //
		DLLEXPORT void HandleRemoteConsoleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection);
		DLLEXPORT void HandleRemoteConsoleResponse(shared_ptr<NetworkResponse> response, ConnectionInfo* connection, shared_ptr<NetworkRequest> potentialrequest);

		//! Does everything needed to allow the client on the connection to connect to us
		DLLEXPORT void OfferConnectionTo(ConnectionInfo* connectiontouse, const wstring &connectionname, int token);

		//! \brief Returns true if connections are marked as awaiting
		//!
		//! Connections are marked as expected when the ExpectNewConnection function is called
		DLLEXPORT bool IsAwaitingConnections();


		//! \brief Returns active number of connections
		DLLEXPORT size_t GetActiveConnectionCount();


		//! \brief Gets the corresponding ConnectionInfo object from a RemoteConsoleSession session indicated by name
		//! \warning The object can be already released or maybe even deleted. Use NetworkHandler::GetSafePointerToConnection to make it safe to use
		DLLEXPORT ConnectionInfo* GetUnsafeConnectionForRemoteConsoleSession(const wstring &name);

		//! \brief Sets the remote console to close the game if there are no connections
		//!
		//! \see CloseIfNoRemoteConsole
		DLLEXPORT void SetCloseIfNoRemoteConsole(bool state);

		//! \brief Gets a matching RemoteConsoleSession from ConnectionInfo
		//!
		//! \return Returns a valid pointer to a RemoteConsoleSession or NULL
		//! \note The returned pointer will be guaranteed to be only valid while you have guard locked
		//! \param guard ObjectLock with this RemoteConsole instance
		DLLEXPORT RemoteConsoleSession* GetRemoteConsoleSessionForConnection(ConnectionInfo* connection, ObjectLock &guard);

		DLLEXPORT bool CanOpenNewConnection(ConnectionInfo* connection, shared_ptr<NetworkRequest> request);

		DLLEXPORT void ExpectNewConnection(int SessionToken, const wstring &assignname = L"", bool onlylocalhost = false, 
			const MillisecondDuration &timeout = boost::chrono::seconds(30));

		DLLEXPORT static RemoteConsole* Get();

	protected:

		//! \brief Called by Engine after command line has been processed
		void SetAllowClose();


		//! \brief Used to detect when a connection has been closed
		virtual void _OnNotifierDisconnected(BaseNotifierAll* parenttoremove);

	private:


		// ------------------------------------ //
		// We need to store the requests until we get a response //
		std::vector<shared_ptr<NetworkRequest>> WaitingRequests;

		std::vector<shared_ptr<RemoteConsoleSession>> RemoteConsoleConnections;


		std::vector<shared_ptr<RemoteConsoleExpect>> AwaitingConnections;

		// Special command variables //
		//! Sends a close signal to the application if has no AwaitingConnections or RemoteConsoleConnections
		bool CloseIfNoRemoteConsole;

		//! Prevents the program from closing before receiving the wanted connection info
		bool CanClose;


		static RemoteConsole* staticinstance;
	};

}
#endif