#ifndef LEVIATHAN_NETWORKRESPONSE
#define LEVIATHAN_NETWORKRESPONSE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SFML/Network/Packet.hpp"
#include "NetworkHandler.h"


namespace Leviathan{

	enum NETWORKRESPONSETYPE{
		// Sent in response to a NETWORKREQUESTTYPE_IDENTIFICATION contains a user readable string, game name, game version and leviathan version strings //
		NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS,
		NETWORKRESPONSETYPE_KEEPALIVE,
		NETWORKRESPONSETYPE_CLOSECONNECTION,
		NETWORKRESPONSETYPE_REMOTECONSOLEOPENED,
		NETWORKRESPONSETYPE_NONE
	};

	class BaseNetworkResponseData{
	public:

		DLLEXPORT virtual ~BaseNetworkResponseData(){};

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) = 0;
	};

	class NetworkResponseDataForIdentificationString : public BaseNetworkResponseData{
	public:
		DLLEXPORT NetworkResponseDataForIdentificationString(sf::Packet &frompacket);
		DLLEXPORT NetworkResponseDataForIdentificationString(const wstring &userreadableidentification, const wstring &gamename, const wstring &gameversion, 
			const wstring &leviathanversion);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);
		
	protected:
		// Data //
		wstring UserReadableData;
		wstring GameName;
		wstring GameVersionString;
		wstring LeviathanVersionString;


	};

	class NetworkResponse : public Object{
	public:
		DLLEXPORT NetworkResponse(int inresponseto, PACKET_TIMEOUT_STYLE timeout, int timeoutvalue);
		// This is for constructing these on the receiver side //
		DLLEXPORT NetworkResponse(sf::Packet &receivedresponse);
		DLLEXPORT ~NetworkResponse();

		// Named "constructors" for different types //
		DLLEXPORT void GenerateIdentificationStringResponse(NetworkResponseDataForIdentificationString* newddata);
		DLLEXPORT void GenerateKeepAliveResponse();
		DLLEXPORT void GenerateCloseConnectionResponse();
		DLLEXPORT void GenerateRemoteConsoleOpenedResponse();
		DLLEXPORT void GenerateEmptyResponse();

		DLLEXPORT NETWORKRESPONSETYPE GetTypeOfResponse();

		DLLEXPORT sf::Packet GeneratePacketForResponse();

		DLLEXPORT NetworkResponseDataForIdentificationString* GetResponseDataForIdentificationString();

		DLLEXPORT int GetTimeOutValue();
		DLLEXPORT PACKET_TIMEOUT_STYLE GetTimeOutType();

		// De-coding functions //

		DLLEXPORT int GetResponseID();

	protected:

		int ResponseID;


		int TimeOutValue;
		PACKET_TIMEOUT_STYLE TimeOutStyle;

		NETWORKRESPONSETYPE ResponseType;

		// Holds the pointer to the struct that holds the response data //
		BaseNetworkResponseData* ResponseData;
	};

}
#endif
