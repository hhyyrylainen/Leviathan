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

namespace Leviathan{
namespace Test{

class TestComponentCreation{

};
}
}

TEST_CASE("RenderingPositionSystem creates nodes", "[entity]"){
    
    // PartialEngineWithOgre engine; [xrequired]
    PartialEngine<false> engine;
    
    // Copied from the standard world //    
    ComponentHolder<Position> ComponentPosition;
    ComponentHolder<RenderNode> ComponentRenderNode;

    ObjectID id = 36;

    ComponentPosition.ConstructNew(id,
        Position::Data{Float3(0, 1, 2), Float4::IdentityQuaternion()});

    ComponentRenderNode.ConstructNew(id, TestComponentCreation());
    
    const auto& addedPosition = ComponentPosition.GetAdded();
    const auto& addedRenderNode = ComponentRenderNode.GetAdded();

    CHECK(addedPosition.size() == 1);
    CHECK(addedRenderNode.size() == 1);
    
    RenderingPositionSystem _RenderingPositionSystem;
    
    _RenderingPositionSystem.CreateNodes(
        addedRenderNode, addedPosition,
        ComponentRenderNode, ComponentPosition);    
    
    CHECK(_RenderingPositionSystem.GetNodeCount() == 1);

}

TEST_CASE("PositionStateSystem creates state objects", "[entity]"){

    PartialEngine<false> engine;

    StateHolder<PositionState> PositionStates;

    PositionStateSystem _PositionStateSystem;

    ComponentHolder<Position> ComponentPosition;

    StandardWorld dummyWorld;

    ObjectID id = 36;

    int tick = 0;

    auto pos = ComponentPosition.ConstructNew(id,
        Position::Data{Float3(0, 1, 2), Float4::IdentityQuaternion()});

    CHECK(PositionStates.GetNumberOfEntitiesWithStates() == 0);
    CHECK(!PositionStates.GetEntityStates(id));

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, ++tick);

    CHECK(PositionStates.GetNumberOfEntitiesWithStates() == 1);
    REQUIRE(PositionStates.GetEntityStates(id));
    CHECK(PositionStates.GetEntityStates(id)->GetNumberOfStates() == 1);

    // No new state is created
    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, ++tick);

    CHECK(PositionStates.GetNumberOfEntitiesWithStates() == 1);
    REQUIRE(PositionStates.GetEntityStates(id));
    CHECK(PositionStates.GetEntityStates(id)->GetNumberOfStates() == 1);

    // Even if marked
    pos->Marked = true;

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, ++tick);

    CHECK(PositionStates.GetNumberOfEntitiesWithStates() == 1);
    REQUIRE(PositionStates.GetEntityStates(id));
    CHECK(PositionStates.GetEntityStates(id)->GetNumberOfStates() == 1);

    // Until position is changed
    pos->Marked = true;
    pos->Members._Position = Float3(1, 1, 1);

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, ++tick);

    CHECK(PositionStates.GetNumberOfEntitiesWithStates() == 1);
    REQUIRE(PositionStates.GetEntityStates(id));
    CHECK(PositionStates.GetEntityStates(id)->GetNumberOfStates() == 2);
}


