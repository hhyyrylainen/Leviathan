#include "Entities/GameWorld.h"
#include "Entities/CommonStateObjects.h"
#include "Handlers/ObjectLoader.h"
#include "Engine.h"
#include "Common/SFMLPackets.h"
#include "Entities/Components.h"

#include "PartialEngine.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;


TEST_CASE("Positonable delta state interpolation", "[networking, entity]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    GameWorld world;
    world.Init(nullptr, nullptr);

    auto brush = ObjectLoader::LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0,
        { Float3(10, 0, 0), Float4::IdentityQuaternion() });

    REQUIRE(brush != 0);

    auto& position = world.GetComponent<Position>(brush);
    
    auto firststate = PositionDeltaState::CaptureState(position, 5);

    SECTION("Position properly captured state"){
        
        CHECK(firststate->Position == Float3(10, 0, 0));
        CHECK(firststate->Rotation == Float4::IdentityQuaternion());
        CHECK(firststate->ValidFields == PRDELTA_ALL_UPDATED);
    }

    position._Position = Float3(20, 5, 0);

    auto secondstate = PositionDeltaState::CaptureState(position, 5);

    SECTION("Postionable rotationable captures updated state"){
        
        CHECK(secondstate->Position == Float3(20, 5, 0));
        CHECK(secondstate->Rotation == Float4::IdentityQuaternion());
        CHECK(secondstate->ValidFields == PRDELTA_ALL_UPDATED);
    }

    position.Interpolate(*firststate, *secondstate, 0.5f);

    CHECK(position._Position.X == Approx(15));
    CHECK(position._Position.Y == Approx(2.5));
    CHECK(position._Position.Z == Approx(0));
    CHECK(position._Orientation == Float4::IdentityQuaternion());

    // Another point //
    position.Interpolate(*firststate, *secondstate, 0.25f);

    CHECK(position._Position.X == Approx(12.5));
    CHECK(position._Position.Y == Approx(1.25));
    CHECK(position._Position.Z == Approx(0));
    CHECK(position._Orientation == Float4::IdentityQuaternion());

    // Only first //
    position.Interpolate(*firststate, *secondstate, 0);
    
    CHECK(position._Position == firststate->Position);
    CHECK(position._Orientation == firststate->Rotation);

    // Only second first //
    position.Interpolate(*firststate, *secondstate, 1);
    
    CHECK(position._Position == secondstate->Position);
    CHECK(position._Orientation == secondstate->Rotation);

    world.Release();
}

TEST_CASE("Position state through packet and interpolate", "[networking, entity]")
{

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    GameWorld world;
    world.Init(nullptr, nullptr);

    auto brush = ObjectLoader::LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0,
        { Float3(0, 0, 0), Float4::IdentityQuaternion() });

    REQUIRE(brush != 0);

    auto& position = world.GetComponent<Position>(brush);

    auto atoriginstate = PositionDeltaState::CaptureState(position, 0);

    position._Position = Float3(50, 25, 5);

    auto firststate = PositionDeltaState::CaptureState(position, 1);

    sf::Packet packet1;

    firststate->CreateUpdatePacket(atoriginstate.get(), packet1);

    auto reconstructed = make_shared<PositionDeltaState>(1, packet1);

    CHECK_FALSE((reconstructed->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK((reconstructed->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK((reconstructed->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK((reconstructed->ValidFields & PRDELTAUPDATED_POS_Z));

    position._Position = Float3(100, 0, 5);

    auto secondstate = PositionDeltaState::CaptureState(position, 2);

    sf::Packet packet2;

    secondstate->CreateUpdatePacket(firststate.get(), packet2);

    auto reconstructed2 = make_shared<PositionDeltaState>(2, packet2);

    CHECK_FALSE((reconstructed2->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK((reconstructed2->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK((reconstructed2->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK_FALSE((reconstructed2->ValidFields & PRDELTAUPDATED_POS_Z));


    reconstructed->FillMissingData(*PositionDeltaState::CaptureState(position, 0));

    
    position.Interpolate(*reconstructed, *reconstructed2, 0.5f);

    CHECK(position._Position.X == Approx(75));
    CHECK(position._Position.Y == Approx(12.5));
    CHECK(position._Position.Z == 5);
    CHECK(position._Orientation == Float4::IdentityQuaternion());

    world.Release();
}

TEST_CASE("Position state fill missing data", "[networking, entity"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    GameWorld world;
    world.Init(nullptr, nullptr);

    auto brush = ObjectLoader::LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0,
        { Float3(0, 0, 0), Float4::IdentityQuaternion() });

    REQUIRE(brush != 0);

    auto& position = world.GetComponent<Position>(brush);

    auto initialstate = PositionDeltaState::CaptureState(position, 0);

    // First position //
    position._Position = Float3(50, 25, 0);

    auto firstcheck = PositionDeltaState::CaptureState(position, 1);

    sf::Packet packet1;

    firstcheck->CreateUpdatePacket(initialstate.get(), packet1);

    auto firstresult = make_shared<PositionDeltaState>(1, packet1);

    CHECK_FALSE((firstresult->ValidFields & PRDELTA_ALL_UPDATED) == PRDELTA_ALL_UPDATED);

    CHECK((firstresult->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK((firstresult->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_POS_Z));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_ROT_Y));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_ROT_Z));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_ROT_W));

    
    // Second position //
    position._Position = Float3(100, 0, 0);

    auto secondcheck = PositionDeltaState::CaptureState(position, 2);

    sf::Packet packet2;

    secondcheck->CreateUpdatePacket(initialstate.get(), packet2);

    auto secondresult = make_shared<PositionDeltaState>(2, packet2);

    CHECK_FALSE(secondresult->ValidFields == PRDELTA_ALL_UPDATED);

    CHECK((secondresult->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_POS_Z));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_ROT_Y));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_ROT_Z));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_ROT_W));

    const int statesbefore = secondresult->ValidFields;

    bool filled = secondresult->FillMissingData(*firstresult);

    CHECK_FALSE(filled);

    const int statesafter = secondresult->ValidFields;

    CHECK(statesbefore != statesafter);

    CHECK((secondresult->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK((secondresult->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_POS_Z));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_ROT_Y));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_ROT_Z));
    CHECK_FALSE((secondresult->ValidFields & PRDELTAUPDATED_ROT_W));
    
    
    filled = secondresult->FillMissingData(*PositionDeltaState::CaptureState(position, 0));

    CHECK(filled);

    CHECK(secondresult->ValidFields == PRDELTA_ALL_UPDATED);
    CHECK((secondresult->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK((secondresult->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK((secondresult->ValidFields & PRDELTAUPDATED_POS_Z));
    CHECK((secondresult->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK((secondresult->ValidFields & PRDELTAUPDATED_ROT_Y));
    CHECK((secondresult->ValidFields & PRDELTAUPDATED_ROT_Z));
    CHECK((secondresult->ValidFields & PRDELTAUPDATED_ROT_W));

    world.Release();   
}



