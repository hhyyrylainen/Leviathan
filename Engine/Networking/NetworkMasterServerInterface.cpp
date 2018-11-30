// ------------------------------------ //
#include "NetworkMasterServerInterface.h"

#include "Connection.h"
#include "Exceptions.h"
#include "NetworkHandler.h"
#include "NetworkInterface.h"
#include "NetworkRequest.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkMasterServerInterface::NetworkMasterServerInterface() :
    NetworkInterface(NETWORKED_TYPE::Master)
{}

DLLEXPORT Leviathan::NetworkMasterServerInterface::~NetworkMasterServerInterface() {}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkMasterServerInterface::HandleRequestPacket(
    const std::shared_ptr<NetworkRequest>& request, Connection& connection)
{
    LEVIATHAN_ASSERT(request, "request is null");

    if(_HandleDefaultRequest(request, connection))
        return;

    // switch (request->GetType()) {
    // }

    if(_CustomHandleRequestPacket(request, connection))
        return;

    LOG_ERROR("NetworkMasterServerInterface: failed to handle request of type: " +
              Convert::ToString(static_cast<int>(request->GetType())));
}

DLLEXPORT void Leviathan::NetworkMasterServerInterface::HandleResponseOnlyPacket(
    const std::shared_ptr<NetworkResponse>& message, Connection& connection)
{
    LEVIATHAN_ASSERT(message, "message is null");

    if(_HandleDefaultResponseOnly(message, connection))
        return;

    // switch (message->GetType()) {
    // }

    if(_CustomHandleResponseOnlyPacket(message, connection))
        return;

    LOG_ERROR("NetworkMasterServerInterface: failed to handle response of type: " +
              Convert::ToString(static_cast<int>(message->GetType())));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkMasterServerInterface::TickIt() {}

DLLEXPORT void Leviathan::NetworkMasterServerInterface::CloseDown() {}

DLLEXPORT std::vector<std::shared_ptr<Leviathan::Connection>>&
    Leviathan::NetworkMasterServerInterface::GetClientConnections()
{
    DEBUG_BREAK;
    LEVIATHAN_ASSERT(false, "GetClientConnections called on a master server");
    throw Exception("GetClientConnections called on a master server");
}

// ------------------------------------ //
