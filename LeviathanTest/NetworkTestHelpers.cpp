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
