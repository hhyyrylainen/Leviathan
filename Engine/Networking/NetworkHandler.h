#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "NetworkInterface.h"
#include "SFML/Network/UdpSocket.hpp"
#include <future>
#include <thread>

namespace Leviathan{

	void RunGetResponseFromMaster(NetworkHandler* instance,
        std::shared_ptr<std::promise<std::string>> resultvar);
	
	enum PACKET_TIMEOUT_STYLE{
        

		PACKET_TIMEOUT_STYLE_TIMEDMS,
        
        //! This style marks packets lost after TimeOutMS amount of packets sent after
        //! this packet have been confirmed to received
		//! Example: If you set TimeOutMS to 1 this packet is resent after a single packet sent
        //! after this packet is received by the target host (and we have received an ack for it)
		PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED
	};

	// Used to pass master server info to the application //
	struct MasterServerInformation{
		MasterServerInformation(bool iammaster, const std::string &identificationstr) :
            MasterServerIdentificationString(identificationstr), RequireMaster(false),
            IAmMyOwnMaster(true)
        {

		}
		MasterServerInformation() : RequireMaster(false), IAmMyOwnMaster(false){
		}
		MasterServerInformation(const std::string &masterslistfile,
            const std::string &identification,
            const std::string &masterserverlistaddress,
            const std::string &masterserverlistpagename,
            const std::string &loginsession, bool requireconnection = false) :
			MasterListFetchServer(masterserverlistaddress),
            MasterListFetchPage(masterserverlistpagename),
            StoredListFile(masterslistfile), MasterServerIdentificationString(identification),
            LoginStoreFile(loginsession), RequireMaster(requireconnection), IAmMyOwnMaster(false)
        {

		}
        
        
		std::string MasterListFetchServer;
		std::string MasterListFetchPage;
		std::string StoredListFile;
		std::string MasterServerIdentificationString;
		std::string LoginStoreFile;
		bool RequireMaster;
		bool IAmMyOwnMaster;
	};

	//! \brief Handles everything related to connections
	class NetworkHandler : public ThreadSafe{
        friend void RunGetResponseFromMaster(NetworkHandler* instance,
            std::shared_ptr<std::promise<std::string>> resultvar);

		friend ConnectionInfo;
	public:
		// Either a client or a server handler //
		DLLEXPORT NetworkHandler(NETWORKED_TYPE ntype, NetworkInterface* packethandler);
		DLLEXPORT virtual ~NetworkHandler();

		DLLEXPORT virtual bool Init(const MasterServerInformation &info);
        
		// \note This waits for all connections to terminate
		DLLEXPORT virtual void Release();

		//! \note  Call as often as possible to receive responses
		DLLEXPORT virtual void UpdateAllConnections();

        //! \brief Called by Engine to stop own connection update thread
        DLLEXPORT void StopOwnUpdaterThread(Lock &guard);

		DLLEXPORT virtual void RemoveClosedConnections();

		DLLEXPORT std::shared_ptr<std::promise<std::string>> QueryMasterServer(Lock &guard,
            const MasterServerInformation &info);

		//! \brief Makes a raw pointer to an ConnectionInfo safe
		//! \return Returns a safe ptr to the passed ConnectionInfo for using it thread safely
		DLLEXPORT std::shared_ptr<ConnectionInfo> GetSafePointerToConnection(
            ConnectionInfo* unsafeptr);

		//! \brief Creates a new connection or returns an existing connection to address
		//! \warning This function is not aware of connections that are created without using NetworkHandler so
        //! there can be multiple connections to host
		//! \todo Fix the warning and disallow that, it shouldn't happen but bugs would be found
		//! \param address The address to connect to. Note this is compared by getting strings from
        //! ConnectionInfo objects
		//! \note This is quite an expensive function and should be called very rarely
		//! \see OpenConnectionTo
		DLLEXPORT std::shared_ptr<ConnectionInfo> GetOrCreatePointerToConnection(
            const std::string &address);


		//! \brief Opens a new connection to the provided address
		//!
		//! \param targetaddress The input should be in a form that has address:port in it. The address should be like
        //! 'google.fi' or '192.168.1.1'
		//! \note This function doesn't verify that there actually is something on the target.
        //! \note The connection will be managed by the handler
		//! and will close if no response is received to a keep alive packet (which is sent after a couple of minutes)
		//! \warning This will always open a new connection. To avoid multiple connections to same target
        //! (and breaking both connections) see GetOrCreatePointerToConnection
		DLLEXPORT std::shared_ptr<ConnectionInfo> OpenConnectionTo(
            const std::string &targetaddress);

		//! Returns the port to which our socket has been bind
		DLLEXPORT unsigned short GetOurPort();

		//! \brief Gets the type of network this program uses
		//!
		//! Will usually be NETWORKED_TYPE_CLIENT or NETWORKED_TYPE_SERVER
		DLLEXPORT NETWORKED_TYPE GetNetworkType() const;

		//! \brief Marks a connection as closing
		//!
		//! The connection will actually close sometime before next packet handling.
		//! \note If you don't want to segfault you should always call this when you want to close a connection
		DLLEXPORT virtual void SafelyCloseConnectionTo(ConnectionInfo* to);

		// Common network functions //
		// For example if passed http://boostslair.com/Pong/MastersList.php returns http://boostslair.com/ //
		DLLEXPORT static std::string GetServerAddressPartOfAddress(const std::string &fulladdress,
            const std::string &regextouse = "http://.*?/");

		DLLEXPORT static NetworkHandler* Get();
		DLLEXPORT static NetworkInterface* GetInterface();

	protected:

		Lock LockSocketForUse();

		// Closes the socket //
		void _ReleaseSocket();

        //! \brief Constantly listens for packets in a blocked state
        void _RunListenerThread();

        //! \brief Does temporary connection updating
        void _RunTemporaryUpdaterThread();

		void _SaveMasterServerList();
		bool _LoadMasterServerList();

        //! \brief Registers a connection to be updated when UpdateAllConnections is called
        void _RegisterConnectionInfo(ConnectionInfo* tomanage);
        
		void _UnregisterConnectionInfo(ConnectionInfo* unregisterme);

		// ------------------------------------ //

		// Internal listing of all connections //

        Mutex ConnectionsToUpdateMutex;
		std::vector<ConnectionInfo*> ConnectionsToUpdate;

        Mutex ConnectionsToTerminateMutex;
		std::vector<ConnectionInfo*> ConnectionsToTerminate;

        Mutex AutoOpenedConnectionsMutex;
		std::vector<std::shared_ptr<ConnectionInfo>> AutoOpenedConnections;

		NETWORKED_TYPE AppType;
		sf::UdpSocket _Socket;
		unsigned short PortNumber;

		//! The syncable variable holder associated with this instance
		SyncedVariables* VariableSyncer;

		//! Game specific packet handler that allows programs to register their own packets
		GameSpecificPacketHandler* _GameSpecificPacketHandler;

		// Used to control the locking of the socket //
		Mutex SocketMutex;

		// The master server list //
		std::vector<std::shared_ptr<std::string>> MasterServers;

		//! Stores a "working" (meaning the server has responded something) master server address
        std::shared_ptr<ConnectionInfo> MasterServerConnection;

		MasterServerInformation StoredMasterServerInfo;

		//! Makes sure that master server thread is graciously closed //
		std::thread MasterServerConnectionThread;
		bool CloseMasterServerConnection;

		//! THread that constantly blocks on the socket and waits for packets
		std::thread ListenerThread;

        //! Temporary thread for getting responses while the game is starting
        std::thread TemporaryUpdateThread;
        bool UpdaterThreadStop;

        std::condition_variable_any NotifyTemporaryUpdater;

        std::string MasterServerMustPassIdentification;

		// Static access //
		static NetworkHandler* instance;
		static NetworkInterface* interfaceinstance;
	};

}

