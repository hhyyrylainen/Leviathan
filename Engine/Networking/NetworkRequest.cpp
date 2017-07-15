// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKREQUEST
#include "NetworkRequest.h"
#endif
#include "Exceptions.h"
#include "GameSpecificPacketHandler.h"
#include "../Handlers/IDFactory.h"
#include "../Utility/Convert.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT std::shared_ptr<NetworkRequest> NetworkRequest::LoadFromPacket(sf::Packet &packet,
    uint32_t messagenumber)
{
    // Get the heading data //
    uint16_t tmpval;
    packet >> tmpval;

    if(!packet)
        throw InvalidArgument("packet has invalid format");

    const NETWORK_REQUEST_TYPE requesttype = static_cast<NETWORK_REQUEST_TYPE>(tmpval);

    // Try to create the additional data if required for this type //
    switch(requesttype){
    case NETWORK_REQUEST_TYPE::Echo:
        return std::make_shared<RequestNone>(requesttype, messagenumber, packet);
    case NETWORK_REQUEST_TYPE::Connect:
        return std::make_shared<RequestConnect>(messagenumber, packet);
    case NETWORK_REQUEST_TYPE::Security:
        return std::make_shared<RequestSecurity>(messagenumber, packet);
    case NETWORK_REQUEST_TYPE::Authenticate:
        return std::make_shared<RequestAuthenticate>(messagenumber, packet);
    case NETWORK_REQUEST_TYPE::JoinServer:
        return std::make_shared<RequestJoinServer>(messagenumber, packet);
    default:
        {
            Logger::Get()->Warning("NetworkRequest: unused type: "+
                Convert::ToString(static_cast<int>(requesttype)));
            throw InvalidArgument("packet has request type that is missing from "
                "switch(requesttype)");
        }
        break;
    }
}





