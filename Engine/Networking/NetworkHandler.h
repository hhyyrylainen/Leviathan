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


	enum NETWORKED_TYPE {NETWORKED_TYPE_CLIENT, NETWORKED_TYPE_SERVER, NETWORKED_TYPE_MASTER};

	// Used to pass master server info to the application //
	struct MasterServerInformation{
		MasterServerInformation(bool iammaster, const wstring &identificationstr) : RequireMaster(false), IAmMyOwnMaster(true), 
			MasterServerIdentificationString(identificationstr)
		{

		}
		MasterServerInformation() : RequireMaster(false){
		}
		MasterServerInformation(const wstring &masterslistfile, const wstring &identification, const wstring &masterserverlistaddress, const wstring 
			&masterserverlistpagename, const wstring &loginsession, bool requireconnection = false) : 
		MasterListFetchServer(masterserverlistaddress), MasterListFetchPage(masterserverlistpagename), StoredListFile(masterslistfile),
			MasterServerIdentificationString(identification), LoginStoreFile(loginsession), RequireMaster(requireconnection)
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


	class NetworkHandler : public EngineComponent, public ThreadSafe{
		friend void RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar);
		friend void RunTemporaryUpdateConnections(NetworkHandler* instance);

		friend ConnectionInfo;
	public:
		// Either a client or a server handler //
		DLLEXPORT NetworkHandler(NETWORKED_TYPE ntype, NetworkInterface* packethandler);
		DLLEXPORT ~NetworkHandler();

		DLLEXPORT virtual bool Init(const MasterServerInformation &info);
		// This waits for all connections to terminate //
		DLLEXPORT virtual void Release();

		// Call as often as possible to receive responses //
		DLLEXPORT virtual void UpdateAllConnections();

		DLLEXPORT virtual void StopOwnUpdaterThread();
		DLLEXPORT virtual void StartOwnUpdaterThread();

		DLLEXPORT virtual void RemoveClosedConnections(ObjectLock &guard);

		DLLEXPORT shared_ptr<boost::promise<wstring>> QueryMasterServer(const MasterServerInformation &info);

		//! \brief Opens a new connection to the provided address
		//!
		//! The input should be in a form that has address:port in it. The address should be like 'google.fi' or '192.168.1.1'
		//! This function doesn't verify that there actually is something on the target. The connection will be managed by the handler
		//! and will close if no response is received to a keep alive packet (which is sent after a couple of minutes)
		DLLEXPORT shared_ptr<ConnectionInfo> OpenConnectionTo(const wstring &targetaddress);



		//! Returns the port to which our socket has been bind //
		DLLEXPORT USHORT GetOurPort();


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
