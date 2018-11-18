#include "Networking/Connection.h"
#include "Networking/NetworkRequest.h"
#include "Networking/NetworkResponse.h"
#include "Entities/GameWorld.h"
#include "Generated/StandardWorld.h"

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

    // Create entities here after joining
    


    CloseServerProperly();
}
