#ifndef LEVIATHAN_NETWORKRESPONSE
#define LEVIATHAN_NETWORKRESPONSE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SFML/Network/Packet.hpp"


namespace Leviathan{

	enum NETWORKRESPONSETYPE{
		// Sent in response to a NETWORKREQUESTTYPE_IDENTIFICATION contains a user readable string, game name, game version and leviathan version strings //
		NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS
	};

	class NetworkResponse : public Object{
	public:
		DLLEXPORT NetworkResponse(NETWORKRESPONSETYPE type);
		// Constructors for different types //
		DLLEXPORT NetworkResponse(const wstring &userreadableidentification, const wstring &gamename, const wstring &gameversion, const wstring &leviathanversion);

		// This is for constructing these on the receiver side //
		DLLEXPORT NetworkResponse(sf::Packet &receivedresponse);

		DLLEXPORT ~NetworkResponse();

		// De-coding functions //
		DLLEXPORT bool DecodeIdentificationStringResponse(wstring &userreadableinfo, wstring &game, wstring &gameversion, wstring &leviathanversion);


	private:

	};

}
#endif
