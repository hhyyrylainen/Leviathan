// ----------------------------------- //
#include "NetworkResponse.h"

#include "Common/DataStoring/NamedVars.h"
#include "GameSpecificPacketHandler.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT std::shared_ptr<NetworkResponse> NetworkResponse::LoadFromPacket(sf::Packet& packet)
{
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
    switch(responsetype) {
    case NETWORK_RESPONSE_TYPE::Connect:
        return std::make_shared<ResponseConnect>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::Authenticate:
        return std::make_shared<ResponseAuthenticate>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::Security:
        return std::make_shared<ResponseSecurity>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::ServerAllow:
        return std::make_shared<ResponseServerAllow>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::ServerDisallow:
        return std::make_shared<ResponseServerDisallow>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::StartWorldReceive:
        return std::make_shared<ResponseStartWorldReceive>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::EntityCreation:
        return std::make_shared<ResponseEntityCreation>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::EntityDestruction:
        return std::make_shared<ResponseEntityDestruction>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::EntityUpdate:
        return std::make_shared<ResponseEntityUpdate>(responseid, packet);
    case NETWORK_RESPONSE_TYPE::EntityLocalControlStatus:
        return std::make_shared<ResponseEntityLocalControlStatus>(responseid, packet);
    // None based types
    case NETWORK_RESPONSE_TYPE::CloseConnection:
    case NETWORK_RESPONSE_TYPE::Keepalive:
    case NETWORK_RESPONSE_TYPE::None:
        return std::make_shared<ResponseNone>(responsetype, responseid, packet);

    default: {
        Logger::Get()->Warning("NetworkResponse: unused type: " +
                               Convert::ToString(static_cast<int>(responsetype)));
        throw InvalidArgument("packet has response type that is missing from "
                              "switch(responsetype)");
    } break;
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkResponse::LimitResponseSize(
    ResponseIdentification& response, uint32_t maxsize)
{
    if(response.GameName.length() + response.GameVersionString.length() +
            response.LeviathanVersionString.length() + response.UserReadableData.length() >
        maxsize) {
        // Need to trim something //
        DEBUG_BREAK;
        response.UserReadableData = "";
    }
}
// ------------------------------------ //
DLLEXPORT std::string NetworkResponse::GetTypeStr() const
{
    switch(Type) {
    case NETWORK_RESPONSE_TYPE::Connect: return "ResponseConnect";
    case NETWORK_RESPONSE_TYPE::Security: return "ResponseSecurity";
    case NETWORK_RESPONSE_TYPE::Authenticate: return "ResponseAuthenticate";
    case NETWORK_RESPONSE_TYPE::Identification: return "ResponseIdentification";
    case NETWORK_RESPONSE_TYPE::Keepalive: return "ResponseKeepalive";
    case NETWORK_RESPONSE_TYPE::CloseConnection: return "ResponseCloseConnection";
    case NETWORK_RESPONSE_TYPE::RemoteConsoleClosed: return "ResponseRemoteConsoleClosed";
    case NETWORK_RESPONSE_TYPE::RemoteConsoleOpened: return "ResponseRemoteConsoleOpened";
    case NETWORK_RESPONSE_TYPE::InvalidRequest: return "ResponseInvalidRequest";
    case NETWORK_RESPONSE_TYPE::ServerDisallow: return "ResponseServerDisallow";
    case NETWORK_RESPONSE_TYPE::ServerAllow: return "ResponseServerAllow";
    case NETWORK_RESPONSE_TYPE::ServerStatus: return "ResponseServerStatus";
    case NETWORK_RESPONSE_TYPE::SyncValData: return "ResponseSyncValData";
    case NETWORK_RESPONSE_TYPE::SyncDataEnd: return "ResponseSyncDataEnd";
    case NETWORK_RESPONSE_TYPE::SyncResourceData: return "ResponseSyncResourceData";
    case NETWORK_RESPONSE_TYPE::CreateNetworkedInput: return "ResponseCreateNetworkedInput";
    case NETWORK_RESPONSE_TYPE::UpdateNetworkedInput: return "ResponseUpdateNetworkedInput";
    case NETWORK_RESPONSE_TYPE::DisconnectInput: return "ResponseDisconnectInput";
    case NETWORK_RESPONSE_TYPE::StartWorldReceive: return "ResponseStartWorldReceive";
    case NETWORK_RESPONSE_TYPE::EntityCreation: return "ResponseEntityCreation";
    case NETWORK_RESPONSE_TYPE::EntityUpdate: return "ResponseEntityUpdate";
    case NETWORK_RESPONSE_TYPE::EntityDestruction: return "ResponseEntityDestruction";
    case NETWORK_RESPONSE_TYPE::EntityLocalControlStatus:
        return "ResponseEntityLocalControlStatus";
    case NETWORK_RESPONSE_TYPE::CacheUpdated: return "ResponseCacheUpdated";
    case NETWORK_RESPONSE_TYPE::CacheRemoved: return "ResponseCacheRemoved";
    case NETWORK_RESPONSE_TYPE::WorldFrozen: return "ResponseWorldFrozen";
    case NETWORK_RESPONSE_TYPE::ServerHeartbeat: return "ResponseServerHeartbeat";
    case NETWORK_RESPONSE_TYPE::StartHeartbeats: return "ResponseStartHeartbeats";
    case NETWORK_RESPONSE_TYPE::None: return "ResponseNone";
    default: break;
    }

    if(Type > NETWORK_RESPONSE_TYPE::Custom) {

        return "ResponseCustom(+" +
               std::to_string(
                   static_cast<int>(Type) - static_cast<int>(NETWORK_RESPONSE_TYPE::Custom)) +
               ")";

    } else {

        return std::to_string(static_cast<int>(Type));
    }
}
