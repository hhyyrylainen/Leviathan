#ifndef LEVIATHAN_NETWORKHANDLER
#define LEVIATHAN_NETWORKHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NetworkClient.h"
#include "NetworkServer.h"
#include <boost\thread\future.hpp>
#include "Common\ThreadSafe.h"
#include "ConnectionInfo.h"


namespace Leviathan{

	void RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar);
	void RunTemporaryUpdateConnections(NetworkHandler* instance);

	class NetworkHandler : public EngineComponent, public ThreadSafe{
		friend void RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar);
		friend void RunTemporaryUpdateConnections(NetworkHandler* instance);

		friend ConnectionInfo;
	public:
		// Either a client or a server handler //
		DLLEXPORT NetworkHandler(NetworkClient* clientside);
		DLLEXPORT NetworkHandler(NetworkServer* serverside);
		DLLEXPORT ~NetworkHandler();

		DLLEXPORT virtual bool Init(const MasterServerInformation &info);
		// This waits for all connections to terminate //
		DLLEXPORT virtual void Release();

		// Call as often as possible to receive responses //
		DLLEXPORT virtual void UpdateAllConnections();

		DLLEXPORT virtual void StopOwnUpdaterThread();
		DLLEXPORT virtual void StartOwnUpdaterThread();

		DLLEXPORT shared_ptr<boost::promise<wstring>> QueryMasterServer(const MasterServerInformation &info);
		
		// Common network functions //
		// For example if passed http://boostslair.com/Pong/MastersList.php returns http://boostslair.com/ //
		DLLEXPORT static wstring GetServerAddressPartOfAddress(const wstring &fulladdress, const wstring &regextouse = L"http://.*?/");

		DLLEXPORT static NetworkHandler* Get();

	protected:


		void _SaveMasterServerList();
		bool _LoadMasterServerList();

		void _RegisterConnectionInfo(ConnectionInfo* tomanage);
		void _UnregisterConnectionInfo(ConnectionInfo* unregisterme);

		// ------------------------------------ //
		
		// Internal listing of all connections //
		std::vector<ConnectionInfo*> ConnectionsToUpdate;


		NetworkClient* IsClient;
		NetworkServer* IsServer;


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

		// Static access //
		static NetworkHandler* instance;
	};

}
#endif