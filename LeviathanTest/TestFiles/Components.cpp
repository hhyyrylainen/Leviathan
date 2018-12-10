#include "../PartialEngine.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"
#include "Entities/StateInterpolator.h"
#include "Handlers/ObjectLoader.h"

#include "Generated/StandardWorld.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("Manual component add and remove", "[entity]"){

    PartialEngine<false> engine;

    StandardWorld TargetWorld(nullptr);

    auto brush = TargetWorld.CreateEntity();

    CHECK(TargetWorld.RemoveComponent_Sendable(brush) == false);

    CHECK_NOTHROW(TargetWorld.Create_Sendable(brush));

    CHECK_NOTHROW(TargetWorld.GetComponent_Sendable(brush));

    CHECK(TargetWorld.RemoveComponent_Sendable(brush) == true);

    CHECK_THROWS_AS(TargetWorld.GetComponent_Sendable(brush), NotFound);

    TargetWorld.Release();
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
    
    CHECK(_RenderingPositionSystem.GetCachedComponentCollectionCount() == 1);

}

TEST_CASE("PositionStateSystem creates state objects", "[entity]"){

    PartialEngine<false> engine;

    StateHolder<PositionState> PositionStates;

    PositionStateSystem _PositionStateSystem;

    ComponentHolder<Position> ComponentPosition;

    StandardWorld dummyWorld(nullptr);
    dummyWorld.Init(WorldNetworkSettings::GetSettingsForHybrid(), nullptr);

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

    SECTION("Generated states have correct data"){

        auto* entityStates = PositionStates.GetEntityStates(id);
        REQUIRE(entityStates);

        PositionState* firstState = entityStates->GetState(1);
        REQUIRE(firstState);

        // Tick should be 4 here
        PositionState* secondState = entityStates->GetState(tick);
        REQUIRE(secondState);

        CHECK(firstState != secondState);
        CHECK(firstState->_Position == Float3(0, 1, 2));
        CHECK(secondState->_Position == Float3(1, 1, 1));
    }
}

TEST_CASE("PositionStateSystem single state is interpolated", "[entity]"){

    PartialEngine<false> engine;

    StateHolder<PositionState> PositionStates;

    PositionStateSystem _PositionStateSystem;

    ComponentHolder<Position> ComponentPosition;

    StandardWorld dummyWorld(nullptr);
    dummyWorld.Init(WorldNetworkSettings::GetSettingsForHybrid(), nullptr);

    ObjectID id = 36;

    const auto initialPos = Float3(5, 8, -5);

    // Create 2 positions 
    auto pos = ComponentPosition.ConstructNew(id,
        Position::Data{initialPos, Float4::IdentityQuaternion()});

    // State marked by default to always apply the initial position even if there are no states
    //CHECK(!pos->StateMarked);
    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, 1);
    CHECK(pos->StateMarked);

    REQUIRE(PositionStates.GetEntityStates(id));
    CHECK(PositionStates.GetEntityStates(id)->GetNumberOfStates() == 1);
    
    pos->Members._Position = Float3(0);

    SECTION("First interpolation"){
        const auto interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            1, 0);

        REQUIRE(std::get<0>(interpolated));
        CHECK(std::get<1>(interpolated)._Position == initialPos);
        CHECK(!pos->StateMarked);
    }

    SECTION("Second interpolation, should still be the same pos"){
        const auto interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            1, 0);

        REQUIRE(std::get<0>(interpolated));
        CHECK(std::get<1>(interpolated)._Position == initialPos);
    }

    SECTION("Third iteration, time passed, but should be the same"){
        const auto interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            1, 15);

        REQUIRE(std::get<0>(interpolated));
        CHECK(std::get<1>(interpolated)._Position == initialPos);
        CHECK(!pos->StateMarked);
    }    

}

TEST_CASE("PositionStateSystem created states can be interpolated", "[entity]"){

    PartialEngine<false> engine;

    StateHolder<PositionState> PositionStates;

    PositionStateSystem _PositionStateSystem;

    ComponentHolder<Position> ComponentPosition;

    StandardWorld dummyWorld(nullptr);
    dummyWorld.Init(WorldNetworkSettings::GetSettingsForHybrid(), nullptr);

    ObjectID id = 36;

    // Create 2 positions 
    auto pos = ComponentPosition.ConstructNew(id,
        Position::Data{Float3(1, 6, 0), Float4::IdentityQuaternion()});

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, 1);

    pos->Marked = true;
    pos->Members._Position = Float3(3, 12, 1);

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, 2);

    REQUIRE(PositionStates.GetEntityStates(id));
    CHECK(PositionStates.GetEntityStates(id)->GetNumberOfStates() == 2);

    auto* entityStates = PositionStates.GetEntityStates(id);
    REQUIRE(entityStates);

    PositionState* firstState = entityStates->GetState(1);
    REQUIRE(firstState);
    CHECK(firstState->_Position == Float3(1, 6, 0));

    PositionState* secondState = entityStates->GetState(2);
    REQUIRE(secondState);
    CHECK(secondState->_Position == Float3(3, 12, 1));

    Position interpolated({Float3(2, 9, 0.5f), Float4::IdentityQuaternion()});

    // This sets the starting time of the interpolation //
    // and returns the first state
    const auto shouldBeFirstState = StateInterpolator::Interpolate(PositionStates, id, pos,
        1, 0);

    REQUIRE(std::get<0>(shouldBeFirstState));
    CHECK(firstState->_Position == std::get<1>(shouldBeFirstState)._Position);

    // Then we jump half a tick forward to be between the 2 states
    const auto interpolationResult = StateInterpolator::Interpolate(PositionStates, id, pos,
        1, TICKSPEED / 2);

    REQUIRE(std::get<0>(interpolationResult));
    CHECK(interpolated.Members._Position == std::get<1>(interpolationResult)._Position);
    CHECK(interpolated.Members._Orientation == std::get<1>(interpolationResult)._Orientation);

    // And should be the later state //
    const auto shouldBeLast = StateInterpolator::Interpolate(PositionStates, id, pos,
        1, TICKSPEED);

    REQUIRE(std::get<0>(shouldBeLast));
    CHECK(secondState->_Position == std::get<1>(shouldBeLast)._Position);
}


TEST_CASE("PositionStateSystem multiple states with gaps can be interpolated", "[entity]"){

    PartialEngine<false> engine;

    StateHolder<PositionState> PositionStates;

    PositionStateSystem _PositionStateSystem;

    ComponentHolder<Position> ComponentPosition;

    StandardWorld dummyWorld(nullptr);
    dummyWorld.Init(WorldNetworkSettings::GetSettingsForHybrid(), nullptr);

    ObjectID id = 12;

    auto pos = ComponentPosition.ConstructNew(id,
        Position::Data{Float3(0, 0, 0), Float4::IdentityQuaternion()});
    
    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, 1);

    
    pos->Members._Position = Float3(1, 0, 0);
    pos->Marked = true;

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, 2);

    
    pos->Members._Position = Float3(2, 0, 0);
    pos->Marked = true;

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, 3);

    
    pos->Members._Position = Float3(3, 0, 0);
    pos->Marked = true; 

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, 5);

    
    pos->Members._Position = Float3(4, 0, 0);
    pos->Marked = true; 

    _PositionStateSystem.Run(dummyWorld, ComponentPosition.GetIndex(), PositionStates, 6);

    // Initial time set
    StateInterpolator::Interpolate(PositionStates, id, pos, 1, 0);

    SECTION("Tick 1 to 2"){

        const auto interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            1, TICKSPEED / 2);
        REQUIRE(std::get<0>(interpolated));
        CHECK(Float3(0.5f, 0, 0) == std::get<1>(interpolated)._Position);
    }

    SECTION("Tick 2 to 3"){

        const auto interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            2, TICKSPEED / 2);
        REQUIRE(std::get<0>(interpolated));
        CHECK(Float3(1.5f, 0, 0) == std::get<1>(interpolated)._Position);
    }

    SECTION("Tick 3 to 5 (4 missing)"){

        auto interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            3, 0);
        REQUIRE(std::get<0>(interpolated));
        CHECK(Float3(2.f, 0, 0) == std::get<1>(interpolated)._Position);

        interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            3, TICKSPEED / 2);
        REQUIRE(std::get<0>(interpolated));
        CHECK(Float3(2.25f, 0, 0) == std::get<1>(interpolated)._Position);

        interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            4, 0);
        REQUIRE(std::get<0>(interpolated));
        CHECK(Float3(2.5f, 0, 0) == std::get<1>(interpolated)._Position);        

        interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            4, TICKSPEED / 2);
        REQUIRE(std::get<0>(interpolated));
        CHECK(Float3(2.75f, 0, 0) == std::get<1>(interpolated)._Position);

        interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            5, 0);
        REQUIRE(std::get<0>(interpolated));
        CHECK(Float3(3, 0, 0) == std::get<1>(interpolated)._Position);        
    }

    SECTION("Tick 5 to 6"){

        const auto interpolated = StateInterpolator::Interpolate(PositionStates, id, pos,
            5, TICKSPEED / 2);
        REQUIRE(std::get<0>(interpolated));
        CHECK(Float3(3.5f, 0, 0) == std::get<1>(interpolated)._Position);
    }    
}


