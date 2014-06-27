#ifndef LEVIATHAN_NETWORKREQUEST
#define LEVIATHAN_NETWORKREQUEST
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SFML/Network/Packet.hpp"
#include "NetworkHandler.h"


#define MAX_SERVERCOMMAND_LENGTH		550

namespace Leviathan{

	enum NETWORKREQUESTTYPE {
		//! This is sent first, expected result is like "PongServer running version 0.5.1.0, status: 0/20"
		NETWORKREQUESTTYPE_IDENTIFICATION,
		NETWORKREQUESTTYPE_SERVERSTATUS,
		NETWORKREQUESTTYPE_OPENREMOTECONSOLETO,
		NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE,
		NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE,
		NETWORKREQUESTTYPE_JOINSERVER,
		NETWORKREQUESTTYPE_GETSINGLESYNCVALUE,
		NETWORKREQUESTTYPE_GETALLSYNCVALUES,
		//! Used to request the server to run a command, used for chat and other things
		NETWORKREQUESTTYPE_REQUESTEXECUTION,
		//! Sent when a player requests the server to connect a NetworkedInput
		NETWORKREQUESTTYPE_CONNECTINPUT,

		//! Used for game specific requests
		NETWORKREQUESTTYPE_CUSTOM
	};

	class BaseNetworkRequestData{
	public:

		DLLEXPORT virtual ~BaseNetworkRequestData(){};

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) = 0;
	};

	class RemoteConsoleOpenRequestDataTo : public BaseNetworkRequestData{
	public:
		DLLEXPORT RemoteConsoleOpenRequestDataTo(int token);
		DLLEXPORT RemoteConsoleOpenRequestDataTo(sf::Packet &frompacket);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		int SessionToken;
	};

	//! \todo add security to this
	class RemoteConsoleAccessRequestData : public BaseNetworkRequestData{
	public:
		DLLEXPORT RemoteConsoleAccessRequestData(int token);
		DLLEXPORT RemoteConsoleAccessRequestData(sf::Packet &frompacket);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		int SessionToken;
	};

	//! \todo Add security establishing functions
	//! \todo Add security to the ConnectionInfo class
	class JoinServerRequestData : public BaseNetworkRequestData{
	public:
		DLLEXPORT JoinServerRequestData(int outmasterid = -1);
		DLLEXPORT JoinServerRequestData(sf::Packet &frompacket);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		//! The ID given by the master server
		int MasterServerID;
	};


	class GetSingleSyncValueRequestData : public BaseNetworkRequestData{
	public:
		DLLEXPORT GetSingleSyncValueRequestData(const wstring &name);
		DLLEXPORT GetSingleSyncValueRequestData(sf::Packet &frompacket);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		//! The name of the wanted value
		wstring NameOfValue;
	};

	class RequestCommandExecutionData : public BaseNetworkRequestData{
	public:
		DLLEXPORT RequestCommandExecutionData(const string &commandstr);
		DLLEXPORT RequestCommandExecutionData(sf::Packet &frompacket);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		//! The command to execute
		string Command;
	};

	class RequestConnectInputData : public BaseNetworkRequestData{
	public:
		DLLEXPORT RequestConnectInputData(NetworkedInput &tosend);
		DLLEXPORT RequestConnectInputData(sf::Packet &frompacket);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		//! This contains the data required to create the object
		sf::Packet DataForObject;
	};

	class CustomRequestData : public BaseNetworkRequestData{
	public:
		DLLEXPORT CustomRequestData(GameSpecificPacketData* newddata);
		DLLEXPORT CustomRequestData(BaseGameSpecificRequestPacket* newddata);
		DLLEXPORT CustomRequestData(sf::Packet &frompacket);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		//! The actual data 
		shared_ptr<GameSpecificPacketData> ActualPacketData;
	};


	class NetworkRequest{
	public:
		DLLEXPORT NetworkRequest(NETWORKREQUESTTYPE type, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(RemoteConsoleOpenRequestDataTo* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(RemoteConsoleAccessRequestData* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(JoinServerRequestData* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(GetSingleSyncValueRequestData* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(CustomRequestData* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(RequestCommandExecutionData* newddata, int timeout = 10, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED);
		DLLEXPORT NetworkRequest(RequestConnectInputData* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		
		DLLEXPORT ~NetworkRequest();

		DLLEXPORT NetworkRequest(sf::Packet &frompacket);

		DLLEXPORT sf::Packet GeneratePacketForRequest();

		DLLEXPORT NETWORKREQUESTTYPE GetType();

		// Specific type data get functions //
		DLLEXPORT RemoteConsoleOpenRequestDataTo* GetRemoteConsoleOpenToData();
		DLLEXPORT RemoteConsoleAccessRequestData* GetRemoteConsoleAccessRequestData();
		DLLEXPORT CustomRequestData* GetCustomRequestData();
		DLLEXPORT RequestCommandExecutionData* GetCommandExecutionRequestData();

		DLLEXPORT int GetExpectedResponseID();

		DLLEXPORT int GetTimeOutValue();
		DLLEXPORT PACKET_TIMEOUT_STYLE GetTimeOutType();

	protected:

		int ResponseID;

		int TimeOutValue;
		PACKET_TIMEOUT_STYLE TimeOutStyle;

		NETWORKREQUESTTYPE TypeOfRequest;
		BaseNetworkRequestData* RequestData;
		
	private:
		NetworkRequest(const NetworkRequest &other){}
	};

}
#endif
