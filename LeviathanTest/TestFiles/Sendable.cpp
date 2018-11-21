#include "../PartialEngine.h"

#include "Common/SFMLPackets.h"
#include "Entities/CommonStateObjects.h"
#include "Entities/Components.h"
#include "Entities/GameWorld.h"
#include "Generated/StandardWorld.h"
#include "Networking/NetworkResponse.h"


#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("Sendable get correct server states", "[entity][networking]")
{

    PartialEngine<false> engine;


    StandardWorld world(nullptr);
    world.Init(WorldNetworkSettings::GetSettingsForClient(), nullptr);




    world.Release();
}


TEST_CASE("World interpolation system works with Brush", "[entity][networking]") {}

TEST_CASE("GameWorld properly loads and applies state packets", "[networking][entity]") {}
