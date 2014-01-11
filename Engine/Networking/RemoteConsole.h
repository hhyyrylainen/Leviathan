#ifndef LEVIATHAN_REMOTECONSOLE
#define LEVIATHAN_REMOTECONSOLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Bases/BaseNotifiable.h"
#include "SFML/Network/IpAddress.hpp"


namespace Leviathan{

	class RemoteConsoleSession{
	public:
		RemoteConsoleSession(const wstring &name, ConnectionInfo* connection, int token);
		


	private:
		wstring ConnectionName;
		int SessionToken;
		ConnectionInfo* CorrespondingConnection;

		// Marks if we can send stuff here //
		bool IsOpened;
	};


	//! Class used to handle remote server commands and receiving messages
	//!
	//! Doesn't actually use reference counting. Inherits BaseNotifiable to be able to receive messages when ConnectionInfo closes
	class RemoteConsole : public BaseNotifiable{
		struct RemoteConsoleExpect{
			RemoteConsoleExpect(const wstring &name, int token, bool onlylocalhost, const MillisecondDuration &timeout);


			wstring ConnectionName;
			int SessionToken;
			bool OnlyLocalhost;

			boost::chrono::steady_clock::time_point TimeoutTime;
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
		DLLEXPORT void OfferConnectionTo(const sf::IpAddress &targetip, USHORT port, const wstring &connectionname);

		//! \brief Returns true if connections are marked as awaiting
		//!
		//! Connections are marked as expected when the ExpectNewConnection function is called
		DLLEXPORT bool IsAwaitingConnections();

		DLLEXPORT bool CanOpenNewConnection(ConnectionInfo* connection, shared_ptr<NetworkRequest> request);

		DLLEXPORT void ExpectNewConnection(int SessionToken, const wstring &assignname = L"", bool onlylocalhost = false, 
			const MillisecondDuration &timeout = boost::chrono::seconds(30));

		DLLEXPORT static RemoteConsole* Get();

		//! Do not actually call this. TODO: create BaseNotifiable that is separate from BaseObject
		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

	private:

		// Used to detect when a connection has closed //
		virtual void _OnNotifierDisconnected(BaseNotifier* parenttoremove);
		// ------------------------------------ //
		// We need to store the requests until we get a response //
		std::vector<shared_ptr<NetworkRequest>> WaitingRequests;

		std::vector<shared_ptr<RemoteConsoleSession>> RemoteConsoleConnections;


		std::vector<shared_ptr<RemoteConsoleExpect>> AwaitingConnections;

		static RemoteConsole* staticinstance;
	};

}
#endif