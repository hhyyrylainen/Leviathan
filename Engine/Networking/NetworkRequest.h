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


namespace Leviathan{

	enum NETWORKREQUESTTYPE {
		//! This is sent first, expected result is like "PongServer running version 0.5.1.0, status: 0/20"
		NETWORKREQUESTTYPE_IDENTIFICATION,
		NETWORKREQUESTTYPE_SERVERSTATUS,
		NETWORKREQUESTTYPE_OPENREMOTECONSOLETO,
		NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE,
		NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE,
		NETWORKREQUESTTYPE_JOINSERVER
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


	class NetworkRequest{
	public:
		DLLEXPORT NetworkRequest(NETWORKREQUESTTYPE type, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(RemoteConsoleOpenRequestDataTo* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(RemoteConsoleAccessRequestData* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT NetworkRequest(JoinServerRequestData* newddata, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT ~NetworkRequest();

		DLLEXPORT NetworkRequest(sf::Packet &frompacket);

		DLLEXPORT sf::Packet GeneratePacketForRequest();

		DLLEXPORT NETWORKREQUESTTYPE GetType();

		// Specific type data get functions //
		DLLEXPORT RemoteConsoleOpenRequestDataTo* GetRemoteConsoleOpenToDataIfPossible();
		DLLEXPORT RemoteConsoleAccessRequestData* GetRemoteConsoleAccessRequestDataIfPossible();

		DLLEXPORT int GetExpectedResponseID();

		DLLEXPORT int GetTimeOutValue();
		DLLEXPORT PACKET_TIMEOUT_STYLE GetTimeOutType();

	protected:

		int ResponseID;

		int TimeOutValue;
		PACKET_TIMEOUT_STYLE TimeOutStyle;

		NETWORKREQUESTTYPE TypeOfRequest;
		BaseNetworkRequestData* RequestData;
	};

}
#endif
