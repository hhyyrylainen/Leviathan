#include "../PartialEngine.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"
#include "Handlers/ObjectLoader.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("Manual component add and remove", "[entity]"){

    PartialEngine<false> engine;

    GameWorld TargetWorld(NETWORKED_TYPE::Client);

    auto brush = TargetWorld.CreateEntity();

    CHECK(TargetWorld.RemoveComponent<Sendable>(brush) == false);

    CHECK_NOTHROW(TargetWorld.CreateSendable(brush));

    CHECK_NOTHROW(TargetWorld.GetComponent<Sendable>(brush));

    CHECK(TargetWorld.RemoveComponent<Sendable>(brush) == true);

    CHECK_THROWS_AS(TargetWorld.GetComponent<Sendable>(brush), NotFound);
}

