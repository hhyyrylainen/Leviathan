// ------------------------------------ //
#include "NetworkInterface.h"

#include "../Utility/Convert.h"
#include "Application/AppDefine.h"
#include "Connection.h"
#include "Engine.h"
#include "Exceptions.h"
#include "NetworkHandler.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
#include "RemoteConsole.h"
#include "SyncedVariables.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT NetworkInterface::NetworkInterface(NETWORKED_TYPE type) : OurNetworkType(type) {}

DLLEXPORT NetworkInterface::~NetworkInterface() {}
// ------------------------------------ //
DLLEXPORT bool NetworkInterface::PreHandleResponse(
    const std::shared_ptr<NetworkResponse>& response, SentNetworkThing* originalrequest,
    Connection& connection)
{
    return true;
}
// ------------------------------------ //
DLLEXPORT void NetworkInterface::HandleRequestPacket(
    const std::shared_ptr<NetworkRequest>& request, Connection& connection)
{
    // We can only try the default handle function //
    if(!_HandleDefaultRequest(request, connection)) {
        // We couldn't handle it //

        throw InvalidArgument("could not handle request with default handler");
    }
}
// ------------------------------------ //
bool NetworkInterface::_HandleDefaultRequest(
    const std::shared_ptr<NetworkRequest>& request, Connection& connection)
{
    // Switch based on type //

    // See if it is a sync packet //
    if(Owner->GetSyncedVariables()->HandleSyncRequests(request, &connection))
        return true;

    switch(request->GetType()) {
    case NETWORK_REQUEST_TYPE::Identification: {
        // Avoid allowing DDOS amplification
        const uint32_t maxlength = static_cast<uint32_t>(
            static_cast<RequestIdentification*>(request.get())->DDOSBlock.size());

        // Fetch the data from the configuration object //
        std::string userreadable, gamename, gameversion;

        AppDef::GetDefault()->GetGameIdentificationData(userreadable, gamename, gameversion);

        auto response = ResponseIdentification(
            0, userreadable, gamename, gameversion, LEVIATHAN_VERSION_ANSIS);

        NetworkResponse::LimitResponseSize(response, maxlength);

        // Should be critical, but it is not to avoid DDoS amplification
        connection.SendPacketToConnection(response);

        return true;
    }
    case NETWORK_REQUEST_TYPE::Echo: {
        // Send an empty response back //
        connection.SendPacketToConnection(
            std::make_shared<ResponseNone>(NETWORK_RESPONSE_TYPE::None),
            RECEIVE_GUARANTEE::None);

        return true;
    }
    case NETWORK_REQUEST_TYPE::RemoteConsoleAccess:
    case NETWORK_REQUEST_TYPE::CloseRemoteConsole: {
        Engine::Get()->GetRemoteConsole()->HandleRemoteConsoleRequestPacket(
            request, Owner->GetConnection(&connection));

        return true;
    }
    case NETWORK_REQUEST_TYPE::ConnectInput: {
        DEBUG_BREAK;
        // if (!Owner->GetInputHandler())
        //    return false;

        // Owner->GetInputHandler()->HandleInputPacket(request, connection);

        return true;
    }
    default: return false;
    }

    // Unhandled //
    return false;
}
// ------------------------------------ //
bool NetworkInterface::_HandleDefaultResponseOnly(
    const std::shared_ptr<NetworkResponse>& message, Connection& connection)
{
    // See if it is a sync packet //
    if(Owner->GetSyncedVariables()->HandleResponseOnlySync(message, &connection))
        return true;

    // Switch on type //
    switch(message->GetType()) {
    case NETWORK_RESPONSE_TYPE::Keepalive:
    case NETWORK_RESPONSE_TYPE::None: {
        // Empty packets without a matching request are just ignored,
        // but marked as received
        return true;
    }
    case NETWORK_RESPONSE_TYPE::CloseConnection: {
        // This connection should be closed //
        Logger::Get()->Info("NetworkInterface: dropping connection due to "
                            "receiving a connection close packet (" +
                            connection.GenerateFormatedAddressString() + ")");

        Owner->CloseConnection(connection);
        return true;
    }
    case NETWORK_RESPONSE_TYPE::RemoteConsoleOpened:
    case NETWORK_RESPONSE_TYPE::RemoteConsoleClosed: {
        // Pass to remote console //
        Engine::Get()->GetRemoteConsole()->HandleRemoteConsoleResponse(
            message, connection, NULL);
        return true;
    }
    case NETWORK_RESPONSE_TYPE::CreateNetworkedInput:
    case NETWORK_RESPONSE_TYPE::UpdateNetworkedInput: {
        DEBUG_BREAK;

        // if (!Owner->GetInputHandler())
        //    return false;

        // Owner->GetInputHandler()->HandleInputPacket(request, connection);

        return true;
    }

    default: return false;
    }

    // Not handled //
    return false;
}
// ------------------------------------ //
DLLEXPORT bool NetworkInterface::CanConnectionTerminate(Connection& connection)
{
    // By default allow connections to close //
    return true;
}
// ------------------------------------ //
DLLEXPORT void NetworkInterface::VerifyType(NETWORKED_TYPE type) const
{
    LEVIATHAN_ASSERT(type == OurNetworkType, "NetworkInterface::VerifyType doesn't match");
}
// ------------------------------------ //
DLLEXPORT void NetworkInterface::SetOwner(NetworkHandler* owner)
{
    Owner = owner;
}
