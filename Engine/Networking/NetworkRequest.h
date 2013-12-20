#ifndef LEVIATHAN_NETWORKREQUEST
#define LEVIATHAN_NETWORKREQUEST
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SFML\Network\Packet.hpp"


namespace Leviathan{

	enum NETWORKREQUESTTYPE {
		// This is send first, expected result is like "PongServer running version 0.5.1.0, status: 0/20" //
		NETWORKREQUESTTYPE_IDENTIFICATION
	};


	class NetworkRequest{
	public:
		DLLEXPORT NetworkRequest(NETWORKREQUESTTYPE type, int timeout = 1000);
		DLLEXPORT ~NetworkRequest();


		DLLEXPORT sf::Packet GeneratePacketForRequest();

		DLLEXPORT int GetExpectedResponseID();
		DLLEXPORT int GetTimeoutMilliseconds();

	protected:

		int ResponseID;
		int TimeOutMS;
		NETWORKREQUESTTYPE TypeOfRequest;
		// TODO: data object //

	};

}
#endif