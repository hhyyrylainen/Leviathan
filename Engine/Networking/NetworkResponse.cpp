// ----------------------------------- // 
#include "NetworkResponse.h"

#include "Common/DataStoring/NamedVars.h"
#include "GameSpecificPacketHandler.h"
#include "NetworkedInput.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

DLLEXPORT std::shared_ptr<NetworkResponse> NetworkResponse::LoadFromPacket(sf::Packet &packet){

    // First thing is the response ID //
    uint32_t responseid = 0;
	packet >> responseid;

	// Second is the type, based on which we handle the rest of the data //
	uint16_t rawtype;
    packet >> rawtype;

    if(!packet)
        throw InvalidArgument("packet has invalid format");
    
	const auto responsetype = static_cast<NETWORK_RESPONSE_TYPE>(rawtype);

	// Process based on the type //
	switch(responsetype){
    case NETWORK_RESPONSE_TYPE::CloseConnection:
    case NETWORK_RESPONSE_TYPE::Keepalive:
    case NETWORK_RESPONSE_TYPE::None:
        return std::make_shared<ResponseNone>(responsetype, responseid, packet);

    default:
		{
            Logger::Get()->Warning("NetworkResponse: unused type: "+
                Convert::ToString(responsetype));
            throw InvalidArgument("packet has invalid response type");
		}
		break;
	}
}
