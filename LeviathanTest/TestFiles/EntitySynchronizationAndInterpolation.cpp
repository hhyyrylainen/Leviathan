#include "Entities/GameWorld.h"
#include "Generated/StandardWorld.h"
#include "Networking/Connection.h"
#include "Networking/NetworkRequest.h"
#include "Networking/NetworkResponse.h"

#include "../NetworkTestHelpers.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;


TEST_CASE_METHOD(WorldSynchronizationTestFixture,
    "Client receives world entities from the server created after joining", "[networking]")
{
    REQUIRE(ServerInterface.World);

    VerifyEstablishConnection();

    VerifyServerStarted();

    // Client requests to join game //
    REQUIRE(ClientInterface.JoinServer(ClientConnection));

    RunListeningLoop(8);

    CHECK(ClientInterface.GetServerConnectionState() ==
          NetworkClientInterface::CLIENT_CONNECTION_STATE::Connected);

    CHECK(ServerInterface.World->IsConnectionInWorld(*ServerConnection));

    REQUIRE(ClientInterface.GetWorld());
    CHECK(ClientInterface.GetWorld()->GetID() == ServerInterface.World->GetID());

    // Create entities here after joining
    auto box1 = ServerInterface.World->CreateEntity();

    REQUIRE_NOTHROW(ServerInterface.World->GetComponent_Sendable(box1));
    
    ServerInterface.World->Create_Position(
        box1, Float3(0, 0, 0), Float4::IdentityQuaternion());

    // Run systems to send entity creation messages etc.
    ServerInterface.World->Tick(0);

    // The client receives them here
    RunListeningLoop(1);

    // And they should exist now
    CHECK_NOTHROW(ClientInterface.GetWorld()->GetComponent<Position>(box1));
    CHECK_NOTHROW(ClientInterface.GetWorld()->GetComponent<Received>(box1));

    ClientInterface.GetWorld()->Release();
    CloseServerProperly();
}
