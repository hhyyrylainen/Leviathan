#ifndef LEVIATHAN_NETWORKCLIENT
#define LEVIATHAN_NETWORKCLIENT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ConnectionInfo.h"
#include "Threading\DelayedResult.h"

namespace Leviathan{

	// Used to pass master server info to the application //
	struct MasterServerInformation{
		MasterServerInformation() : RequireMaster(false){
		}
		MasterServerInformation(const wstring &masterslistfile, const wstring &identification, const wstring &masterserverlistaddress, 
			const wstring &loginsession, bool requireconnection = false) : MasterListFetchAddress(masterserverlistaddress), StoredListFile(masterslistfile),
			MasterServerIdentificationString(identification), LoginStoreFile(loginsession), RequireMaster(requireconnection)
		{

		}

		wstring MasterListFetchAddress;
		wstring StoredListFile;
		wstring MasterServerIdentificationString;
		wstring LoginStoreFile;
		bool RequireMaster;
	};


	class NetworkClient : public Object{
	public:
		DLLEXPORT NetworkClient();
		DLLEXPORT virtual ~NetworkClient();


		// Closes all active connections //
		DLLEXPORT void Release();

	protected:

		// Internal query servers //


		// ------------------------------------ //



		// Current server connection //
		shared_ptr<ConnectionInfo> GamePlayServer;


	};

}
#endif