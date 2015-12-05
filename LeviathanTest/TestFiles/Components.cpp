#include "PartialEngine.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"
#include "Handlers/ObjectLoader.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

TEST_CASE("Manual component add and remove", "[entity]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    GameWorld TargetWorld;

    auto brush = TargetWorld.CreateEntity();

    CHECK(TargetWorld.RemoveComponent<Sendable>(brush) == false);

    CHECK_NOTHROW(TargetWorld.CreateSendable(brush, SENDABLE_TYPE_BRUSH));

    CHECK(TargetWorld.GetComponent<Sendable>(brush).SendableHandleType == SENDABLE_TYPE_BRUSH);

    CHECK(TargetWorld.RemoveComponent<Sendable>(brush) == true);

    CHECK_THROWS_AS(TargetWorld.GetComponent<Sendable>(brush), NotFound);
}

