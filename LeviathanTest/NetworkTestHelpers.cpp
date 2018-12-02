// ------------------------------------ //
#include "NetworkTestHelpers.h"

#include "Generated/StandardWorld.h"

using namespace Leviathan;
using namespace Leviathan::Test;
// ------------------------------------ //
TestWorldServerInterface::TestWorldServerInterface() : NetworkServerInterface(1, "TestServer")
{
    World = std::make_shared<StandardWorld>(nullptr);
    World->Init(WorldNetworkSettings::GetSettingsForServer(), nullptr);
    World->SetRunInBackground(true);
}

TestWorldServerInterface::~TestWorldServerInterface()
{
    if(World)
        World->Release();
}

std::shared_ptr<GameWorld> TestWorldServerInterface::_GetWorldForJoinTarget(
    const std::string& options)
{
    CHECK(options.empty());
    return World;
}
// ------------------------------------ //
void WorldSynchronizationTestFixture::ConnectClientToServerWorld()
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
}
