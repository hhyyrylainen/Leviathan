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
		// This is send first, expected result is like "PongServer running version 0.5.1.0, status: 0/20" //
		NETWORKREQUESTTYPE_IDENTIFICATION
	};


	class NetworkRequest{
	public:
		DLLEXPORT NetworkRequest(NETWORKREQUESTTYPE type, int timeout = 1000, PACKET_TIMEOUT_STYLE style = PACKAGE_TIMEOUT_STYLE_TIMEDMS);
		DLLEXPORT ~NetworkRequest();


		DLLEXPORT sf::Packet GeneratePacketForRequest();

		DLLEXPORT int GetExpectedResponseID();
		DLLEXPORT int GetTimeOutValue();

		DLLEXPORT PACKET_TIMEOUT_STYLE GetTimeOutType();

	protected:

		int ResponseID;
		int TimeOutValue;

		PACKET_TIMEOUT_STYLE TimeOutStyle;

		NETWORKREQUESTTYPE TypeOfRequest;
		// TODO: data object //

	};

}
#endif
