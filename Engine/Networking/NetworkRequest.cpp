// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKREQUEST
#include "NetworkRequest.h"
#endif
#include "../Handlers/IDFactory.h"
#include "../Utility/Convert.h"
#include "Exceptions.h"
#include "GameSpecificPacketHandler.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT std::shared_ptr<NetworkRequest> NetworkRequest::LoadFromPacket(
    sf::Packet& packet, uint32_t messagenumber)
{
    // Get the heading data //
    uint16_t tmpval;
    packet >> tmpval;

    if(!packet)
        throw InvalidArgument("packet has invalid format");

    const NETWORK_REQUEST_TYPE requesttype = static_cast<NETWORK_REQUEST_TYPE>(tmpval);

    // Try to create the additional data if required for this type //
    switch(requesttype) {
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
    case NETWORK_REQUEST_TYPE::JoinGame:
        return std::make_shared<RequestJoinGame>(messagenumber, packet);
    default: {
        Logger::Get()->Warning("NetworkRequest: unused type: " +
                               Convert::ToString(static_cast<int>(requesttype)));
        throw InvalidArgument("packet has request type that is missing from "
                              "switch(requesttype)");
    } break;
    }
}
// ------------------------------------ //
DLLEXPORT std::string NetworkRequest::GetTypeStr() const
{
    switch(Type) {
    case NETWORK_REQUEST_TYPE::Connect: return "RequestConnect";
    case NETWORK_REQUEST_TYPE::Security: return "RequestSecurity";
    case NETWORK_REQUEST_TYPE::Authenticate: return "RequestAuthenticate";
    case NETWORK_REQUEST_TYPE::Identification: return "RequestIdentification";
    case NETWORK_REQUEST_TYPE::Serverstatus: return "RequestServerstatus";
    case NETWORK_REQUEST_TYPE::RemoteConsoleOpen: return "RequestRemoteConsoleOpen";
    case NETWORK_REQUEST_TYPE::RemoteConsoleAccess: return "RequestRemoteConsoleAccess";
    case NETWORK_REQUEST_TYPE::CloseRemoteConsole: return "RequestCloseRemoteConsole";
    case NETWORK_REQUEST_TYPE::DoRemoteConsoleOpen: return "RequestDoRemoteConsoleOpen";
    case NETWORK_REQUEST_TYPE::JoinServer: return "RequestJoinServer";
    case NETWORK_REQUEST_TYPE::JoinGame: return "RequestJoinGame";
    case NETWORK_REQUEST_TYPE::GetSingleSyncValue: return "RequestGetSingleSyncValue";
    case NETWORK_REQUEST_TYPE::GetAllSyncValues: return "RequestGetAllSyncValues";
    case NETWORK_REQUEST_TYPE::RequestCommandExecution:
        return "RequestRequestCommandExecution";
    case NETWORK_REQUEST_TYPE::ConnectInput: return "RequestConnectInput";
    case NETWORK_REQUEST_TYPE::Echo: return "RequestEcho";
    case NETWORK_REQUEST_TYPE::WorldClockSync: return "RequestWorldClockSync";
    default: break;
    }

    if(Type > NETWORK_REQUEST_TYPE::Custom) {

        return "RequestCustom(+" +
               std::to_string(
                   static_cast<int>(Type) - static_cast<int>(NETWORK_REQUEST_TYPE::Custom)) +
               ")";

    } else {

        return std::to_string(static_cast<int>(Type));
    }
}
