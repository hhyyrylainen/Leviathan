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


namespace Leviathan{

	class NetworkHandler : public EngineComponent{
	public:
		// Either a client or a server handler //
		DLLEXPORT NetworkHandler(NetworkClient* clientside);
		DLLEXPORT NetworkHandler(NetworkServer* serverside);
		DLLEXPORT ~NetworkHandler();

		DLLEXPORT virtual bool Init(const MasterServerInformation &info);
		// This waits for all connections to terminate //
		DLLEXPORT virtual void Release();

		DLLEXPORT shared_ptr<DelayedResult> QueryMasterServer(const MasterServerInformation &info);

	protected:


		NetworkClient* IsClient;
		NetworkServer* IsServer;


		// This stores the master server connection //
		shared_ptr<ConnectionInfo> MasterServerConnection;

	};

}
#endif