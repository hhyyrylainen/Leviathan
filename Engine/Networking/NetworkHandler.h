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


namespace Leviathan{

	void RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar);

	class NetworkHandler : public EngineComponent, public ThreadSafe{
		friend void RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar);
	public:
		// Either a client or a server handler //
		DLLEXPORT NetworkHandler(NetworkClient* clientside);
		DLLEXPORT NetworkHandler(NetworkServer* serverside);
		DLLEXPORT ~NetworkHandler();

		DLLEXPORT virtual bool Init(const MasterServerInformation &info);
		// This waits for all connections to terminate //
		DLLEXPORT virtual void Release();

		DLLEXPORT shared_ptr<boost::promise<wstring>> QueryMasterServer(const MasterServerInformation &info);
		
		// Common network functions //
		// For example if passed http://boostslair.com/Pong/MastersList.php returns http://boostslair.com/ //
		DLLEXPORT static wstring GetServerAddressPartOfAddress(const wstring &fulladdress, const wstring &regextouse = L"http://.*?/");

	protected:


		void _SaveMasterServerList();
		bool _LoadMasterServerList();

		// ------------------------------------ //

		NetworkClient* IsClient;
		NetworkServer* IsServer;


		// The master server list //
		std::vector<shared_ptr<wstring>> MasterServers;


		// This stores the master server connection //
		shared_ptr<ConnectionInfo> MasterServerConnection;

		MasterServerInformation StoredMasterServerInfo;

		// Makes sure that master server thread is graciously closed //
		boost::thread MasterServerConnectionThread;
		bool CloseMasterServerConnection;
		boost::condition_variable_any TaskQueueNotify;
	};

}
#endif