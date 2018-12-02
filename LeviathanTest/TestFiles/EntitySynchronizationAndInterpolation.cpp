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
    ConnectClientToServerWorld();

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


TEST_CASE_METHOD(WorldSynchronizationTestFixture,
    "Client receives states for interpolation from the server", "[networking]")
{
    ConnectClientToServerWorld();

    // Create entities here after joining
    auto box1 = ServerInterface.World->CreateEntity();

    REQUIRE_NOTHROW(ServerInterface.World->GetComponent_Sendable(box1));

    auto& pos1 = ServerInterface.World->Create_Position(
        box1, Float3(0, 0, 0), Float4::IdentityQuaternion());

    // Run systems to send entity creation messages etc.
    ServerInterface.World->Tick(0);

    pos1.Members._Position = Float3(1, 0, 10);
    pos1.Marked = true;

    // This should send the updated state to the client
    ServerInterface.World->Tick(1);

    // The client receives the initial message and all the state stuff here
    RunListeningLoop(1);

    // And they should exist now
    REQUIRE_NOTHROW(ClientInterface.GetWorld()->GetComponent<Position>(box1));
    auto& pos2 = ClientInterface.GetWorld()->GetComponent<Position>(box1);

    // TODO: there should be a test for seeing that the receiving of the states properly sets
    // StateMarked

    // Initial position check
    CHECK(pos2.Members._Position == Float3(0, 0, 0));

    // State receive check
    auto& clientStates = ClientInterface.GetWorld()->GetStatesFor<Position>();

    auto* clientBoxStates = clientStates.GetEntityStates(box1);
    REQUIRE(clientBoxStates);

    CHECK(clientBoxStates->GetNumberOfStates() == 2);

    // And the state should be marked before interpolation works
    CHECK(pos2.StateMarked);

    // Only RenderNodes are interpolated... so we can't test that here
    // TODO: do something about that
    // Interpolate to half way
    ClientInterface.GetWorld()->RunFrameRenderSystems(0, TICKSPEED / 2);

    // CHECK(pos2.Members._Position == Float3(0.5f, 0, 5.f));

    ClientInterface.GetWorld()->Release();
    CloseServerProperly();
}
