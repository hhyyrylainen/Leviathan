// ----------------------------------- // 
#include "NetworkResponse.h"

#include "Common/DataStoring/NamedVars.h"
#include "GameSpecificPacketHandler.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

DLLEXPORT std::shared_ptr<NetworkResponse> NetworkResponse::LoadFromPacket(sf::Packet &packet){

    // First thing is the type, based on which we handle the rest of the data
    // This is before responseid to look more like a request packet
    uint16_t rawtype;
    packet >> rawtype;

    // Second thing is the response ID //
    uint32_t responseid = 0;
    packet >> responseid;

    if(!packet)
        throw InvalidArgument("packet has invalid format");
    
    const auto responsetype = static_cast<NETWORK_RESPONSE_TYPE>(rawtype);

    // Process based on the type //
    switch(responsetype){
    case NETWORK_RESPONSE_TYPE::CloseConnection:
    case NETWORK_RESPONSE_TYPE::Keepalive:
    case NETWORK_RESPONSE_TYPE::None:
        return std::make_shared<ResponseNone>(responsetype, responseid, packet);
    case NETWORK_RESPONSE_TYPE::Connect:
        return std::make_shared<ResponseConnect>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::Authenticate:
        return std::make_shared<ResponseAuthenticate>(responseid, packet);
    default:
        {
            Logger::Get()->Warning("NetworkResponse: unused type: "+
                Convert::ToString(static_cast<int>(responsetype)));
            throw InvalidArgument("packet has response type that is missing from "
                "switch(responsetype)");
        }
        break;
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkResponse::LimitResponseSize(ResponseIdentification &response,
    uint32_t maxsize) 
{
    if (response.GameName.length() + response.GameVersionString.length() +
        response.LeviathanVersionString.length() + response.UserReadableData.length() >
        maxsize)
    {
        // Need to trim something //
        DEBUG_BREAK;
        response.UserReadableData = "";
    }
}
