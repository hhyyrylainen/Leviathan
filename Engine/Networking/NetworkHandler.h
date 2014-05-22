#ifndef LEVIATHAN_NETWORKHANDLER
#define LEVIATHAN_NETWORKHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NetworkInterface.h"
#include <boost/thread/future.hpp>
#include "Common/ThreadSafe.h"
#include "SFML/Network/UdpSocket.hpp"


namespace Leviathan{

	void RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar);
	void RunTemporaryUpdateConnections(NetworkHandler* instance);

	enum PACKET_TIMEOUT_STYLE {PACKAGE_TIMEOUT_STYLE_TIMEDMS, 
		// This style marks packets lost after TimeOutMS amount of packets sent after this packet have been confirmed to received
		// So if you set this to 1 this packet is resend if even a single packet send after this is received by the target host
		PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED};

	// Used to pass master server info to the application //
	struct MasterServerInformation{
		MasterServerInformation(bool iammaster, const wstring &identificationstr) : RequireMaster(false), IAmMyOwnMaster(true), 
			MasterServerIdentificationString(identificationstr)
		{

		}
		MasterServerInformation() : RequireMaster(false), IAmMyOwnMaster(false){
		}
		MasterServerInformation(const wstring &masterslistfile, const wstring &identification, const wstring &masterserverlistaddress, const wstring 
			&masterserverlistpagename, const wstring &loginsession, bool requireconnection = false) : 
		MasterListFetchServer(masterserverlistaddress), MasterListFetchPage(masterserverlistpagename), StoredListFile(masterslistfile),
			MasterServerIdentificationString(identification), LoginStoreFile(loginsession), RequireMaster(requireconnection),
            IAmMyOwnMaster(false)
		{

		}
		wstring MasterListFetchServer;
		wstring MasterListFetchPage;
		wstring StoredListFile;
		wstring MasterServerIdentificationString;
		wstring LoginStoreFile;
		bool RequireMaster;
		bool IAmMyOwnMaster;
	};

	//! \brief Handles everything related to connections
	class NetworkHandler : public EngineComponent, public ThreadSafe{
		friend void RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar);
		friend void RunTemporaryUpdateConnections(NetworkHandler* instance);

		friend ConnectionInfo;
	public:
		// Either a client or a server handler //
		DLLEXPORT NetworkHandler(NETWORKED_TYPE ntype, NetworkInterface* packethandler);
		DLLEXPORT ~NetworkHandler();

		DLLEXPORT virtual bool Init(const MasterServerInformation &info);
		// \note This waits for all connections to terminate
		DLLEXPORT virtual void Release();

		// Call as often as possible to receive responses //
		DLLEXPORT virtual void UpdateAllConnections();

		DLLEXPORT virtual void StopOwnUpdaterThread();
		DLLEXPORT virtual void StartOwnUpdaterThread();

		DLLEXPORT virtual void RemoveClosedConnections(ObjectLock &guard);

		DLLEXPORT shared_ptr<boost::promise<wstring>> QueryMasterServer(const MasterServerInformation &info);

		//! \brief Makes a raw pointer to an ConnectionInfo safe
		//! \return Returns a safe ptr to the passed ConnectionInfo for using it thread safely
		DLLEXPORT shared_ptr<ConnectionInfo> GetSafePointerToConnection(ConnectionInfo* unsafeptr);

		//! \brief Creates a new connection or returns an existing connection to address
		//! \warning This function is not aware of connections that are created without using NetworkHandler so there can be multiple connections to host
		//! \todo Fix the warning and disallow that, it shouldn't happen but bugs would be found
		//! \param address The address to connect to. Note this is compared by getting strings from ConnectionInfo objects
		//! \note This is quite an expensive function and should be called very rarely
		//! \see OpenConnectionTo
		DLLEXPORT shared_ptr<ConnectionInfo> GetOrCreatePointerToConnection(const wstring &address);


		//! \brief Opens a new connection to the provided address
		//!
		//! \param targetaddress The input should be in a form that has address:port in it. The address should be like 'google.fi' or '192.168.1.1'
		//! \note This function doesn't verify that there actually is something on the target. The connection will be managed by the handler
		//! and will close if no response is received to a keep alive packet (which is sent after a couple of minutes)
		//! \warning This will always open a new connection. To avoid multiple connections to same target (and breaking both connections) see
		//! GetOrCreatePointerToConnection
		DLLEXPORT FORCE_INLINE shared_ptr<ConnectionInfo> OpenConnectionTo(const wstring &targetaddress){
			// Lock and call the real function //
			GUARD_LOCK_THIS_OBJECT();
			return OpenConnectionTo(targetaddress, guard);
		}

		//! \brief Actual version of OpenConnectionTo
		//! \see OpenConnectionTo
		DLLEXPORT shared_ptr<ConnectionInfo> OpenConnectionTo(const wstring &targetaddress, ObjectLock &guard);

		//! Returns the port to which our socket has been bind
		DLLEXPORT USHORT GetOurPort();

		//! \brief Gets the type of network this program uses
		//!
		//! Will usually be NETWORKED_TYPE_CLIENT or NETWORKED_TYPE_SERVER
		DLLEXPORT NETWORKED_TYPE GetNetworkType();

		//! \brief Marks a connection as closing
		//!
		//! The connection will actually close sometime before next packet handling.
		//! \note If you don't want to segfault you should always call this when you want to close a connection
		DLLEXPORT virtual void SafelyCloseConnectionTo(ConnectionInfo* to);

		// Common network functions //
		// For example if passed http://boostslair.com/Pong/MastersList.php returns http://boostslair.com/ //
		DLLEXPORT static wstring GetServerAddressPartOfAddress(const wstring &fulladdress, const wstring &regextouse = L"http://.*?/");

		DLLEXPORT static NetworkHandler* Get();
		DLLEXPORT static NetworkInterface* GetInterface();

	protected:

		shared_ptr<boost::strict_lock<boost::basic_lockable_adapter<boost::recursive_mutex>>> LockSocketForUse();

		// Closes the socket //
		void _ReleaseSocket();


		void _SaveMasterServerList();
		bool _LoadMasterServerList();

		void _RegisterConnectionInfo(ConnectionInfo* tomanage);
		void _UnregisterConnectionInfo(ConnectionInfo* unregisterme);

		// ------------------------------------ //

		// Internal listing of all connections //
		std::vector<ConnectionInfo*> ConnectionsToUpdate;
		std::vector<ConnectionInfo*> ConnectionsToTerminate;

		std::vector<shared_ptr<ConnectionInfo>> AutoOpenedConnections;

		NETWORKED_TYPE AppType;
		sf::UdpSocket _Socket;
		USHORT PortNumber;

		//! The syncable variable holder associated with this instance
		SyncedVariables* VariableSyncer;

		//! Game specific packet handler that allows programs to register their own packets
		GameSpecificPacketHandler* _GameSpecificPacketHandler;

		// Used to control the locking of the socket //
		boost::basic_lockable_adapter<boost::recursive_mutex> SocketMutex;

		// The master server list //
		std::vector<shared_ptr<wstring>> MasterServers;

		// Stores a "working" (meaning the server has responded something) master server address //
		shared_ptr<ConnectionInfo> MasterServerConnection;

		MasterServerInformation StoredMasterServerInfo;

		// Makes sure that master server thread is graciously closed //
		boost::thread MasterServerConnectionThread;
		bool CloseMasterServerConnection;

		// Temporary thread for getting responses while the game is starting //
		boost::thread TempGetResponsesThread;
		bool StopGetResponsesThread;

		wstring MasterServerMustPassIdentification;

		// Static access //
		static NetworkHandler* instance;
		static NetworkInterface* interfaceinstance;
	};

}
#endif
