#include "../PartialEngine.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"
#include "Handlers/ObjectLoader.h"

#include "Generated/StandardWorld.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("Manual component add and remove", "[entity]"){

    PartialEngine<false> engine;

    StandardWorld TargetWorld;

    auto brush = TargetWorld.CreateEntity();

    CHECK(TargetWorld.RemoveComponent_Sendable(brush) == false);

    CHECK_NOTHROW(TargetWorld.Create_Sendable(brush));

    CHECK_NOTHROW(TargetWorld.GetComponent_Sendable(brush));

    CHECK(TargetWorld.RemoveComponent_Sendable(brush) == true);

    CHECK_THROWS_AS(TargetWorld.GetComponent_Sendable(brush), NotFound);
}

