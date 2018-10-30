#include "../PartialEngine.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"
#include "Handlers/ObjectLoader.h"

#include "Generated/StandardWorld.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("Entity parent destruction deletes children", "[entity]"){
    // TODO: recursive check of parenting that has weird order
    PartialEngine<false> engine;

    StandardWorld TargetWorld(nullptr);

    auto parent = TargetWorld.CreateEntity();

    TargetWorld.Create_Position(parent, Float3(0), Float4::IdentityQuaternion());

    CHECK_NOTHROW(TargetWorld.GetComponent_Position(parent));

    auto child1 = TargetWorld.CreateEntity();

    TargetWorld.Create_Position(child1, Float3(0), Float4::IdentityQuaternion());

    CHECK_NOTHROW(TargetWorld.GetComponent_Position(child1));

    auto child2 = TargetWorld.CreateEntity();

    TargetWorld.Create_Position(child2, Float3(0), Float4::IdentityQuaternion());

    CHECK_NOTHROW(TargetWorld.GetComponent_Position(child2));

    auto other = TargetWorld.CreateEntity();

    TargetWorld.Create_Position(other, Float3(0), Float4::IdentityQuaternion());

    CHECK_NOTHROW(TargetWorld.GetComponent_Position(other));

    CHECK(TargetWorld.GetEntityCount() == 4);

    TargetWorld.SetEntitysParent(child1, parent);
    TargetWorld.SetEntitysParent(child2, parent);
    
    TargetWorld.DestroyEntity(parent);

    CHECK(TargetWorld.GetEntityCount() == 1);

    CHECK_THROWS_AS(TargetWorld.GetComponent_Position(parent), Leviathan::NotFound);
    CHECK_THROWS_AS(TargetWorld.GetComponent_Position(child1), Leviathan::NotFound);
    CHECK_THROWS_AS(TargetWorld.GetComponent_Position(child2), Leviathan::NotFound);

    CHECK_NOTHROW(TargetWorld.GetComponent_Position(other));

    TargetWorld.Release();
    CHECK(TargetWorld.GetEntityCount() == 0);
}
