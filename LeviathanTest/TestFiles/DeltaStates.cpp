#include "Entities/Bases/BaseSendableEntity.h"
#include "Entities/GameWorld.h"
#include "Entities/CommonStateObjects.h"
#include "Handlers/ObjectLoader.h"
#include "Engine.h"
#include "Entities/Objects/Brush.h"
#include "Common/SFMLPackets.h"

#include "PartialEngine.h"

#include "catch.hpp"

using namespace Leviathan;


TEST_CASE("Positonable delta state interpolation", "[networking, entity]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    ObjectLoader loader(&engine);

    GameWorld world;
    world.Init(nullptr, nullptr);

    Entity::Brush* brush = nullptr;
    loader.LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0, &brush);
    
    REQUIRE(brush != nullptr);

    brush->SetPos(Float3(10, 0, 0));

    auto firststate = PositionableRotationableDeltaState::CaptureState(*brush, 5);

    SECTION("Postionable rotationable properly captured state"){
        
        CHECK(firststate->Position == Float3(10, 0, 0));
        CHECK(firststate->Rotation == Float4::IdentityQuaternion());
        CHECK(firststate->ValidFields == PRDELTA_ALL_UPDATED);
    }

    brush->SetPos(Float3(20, 5, 0));

    auto secondstate = PositionableRotationableDeltaState::CaptureState(*brush, 5);

    SECTION("Postionable rotationable captures updated state"){
        
        CHECK(secondstate->Position == Float3(20, 5, 0));
        CHECK(secondstate->Rotation == Float4::IdentityQuaternion());
        CHECK(secondstate->ValidFields == PRDELTA_ALL_UPDATED);
    }

    brush->InterpolatePositionableState(*firststate, *secondstate, 0.5f);

    CHECK(brush->GetPosX() == Approx(15));
    CHECK(brush->GetPosY() == Approx(2.5));
    CHECK(brush->GetPosZ() == Approx(0));
    CHECK(brush->GetOrientation() == Float4::IdentityQuaternion());

    // Another point //
    brush->InterpolatePositionableState(*firststate, *secondstate, 0.25f);
    
    CHECK(brush->GetPosX() == Approx(12.5));
    CHECK(brush->GetPosY() == Approx(1.25));
    CHECK(brush->GetPosZ() == Approx(0));
    CHECK(brush->GetOrientation() == Float4::IdentityQuaternion());

    // Only first //
    brush->InterpolatePositionableState(*firststate, *secondstate, 0);
    
    CHECK(brush->GetPos() == firststate->Position);
    CHECK(brush->GetOrientation() == firststate->Rotation);

    // Only second first //
    brush->InterpolatePositionableState(*firststate, *secondstate, 1);
    
    CHECK(brush->GetPos() == secondstate->Position);
    CHECK(brush->GetOrientation() == secondstate->Rotation);

    world.Release();
}

TEST_CASE("Positionable rotationable state through packet and interpolate", "[networking, entity]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    ObjectLoader loader(&engine);

    GameWorld world;
    world.Init(nullptr, nullptr);

    Entity::Brush* brush = nullptr;
    loader.LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0, &brush);
    
    REQUIRE(brush != nullptr);

    brush->SetPos(Float3(0, 0, 0));

    auto atoriginstate = PositionableRotationableDeltaState::CaptureState(*brush, 0);

    brush->SetPos(Float3(50, 25, 5));

    auto firststate = PositionableRotationableDeltaState::CaptureState(*brush, 1);

    sf::Packet packet1;

    firststate->CreateUpdatePacket(atoriginstate.get(), packet1);

    auto reconstructed = make_shared<PositionableRotationableDeltaState>(1, packet1);

    CHECK_FALSE((reconstructed->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK((reconstructed->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK((reconstructed->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK((reconstructed->ValidFields & PRDELTAUPDATED_POS_Z));

    brush->SetPos(Float3(100, 0, 5));

    auto secondstate = PositionableRotationableDeltaState::CaptureState(*brush, 2);

    sf::Packet packet2;

    secondstate->CreateUpdatePacket(firststate.get(), packet2);

    auto reconstructed2 = make_shared<PositionableRotationableDeltaState>(2, packet2);

    CHECK_FALSE((reconstructed2->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK((reconstructed2->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK((reconstructed2->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK_FALSE((reconstructed2->ValidFields & PRDELTAUPDATED_POS_Z));

    reconstructed->FillMissingData(*brush->CaptureState(0));
    
    brush->InterpolatePositionableState(*reconstructed, *reconstructed2, 0.5f);

    CHECK(brush->GetPosX() == Approx(75));
    CHECK(brush->GetPosY() == Approx(12.5));
    CHECK(brush->GetPosZ() == 5);
    CHECK(brush->GetOrientation() == Float4::IdentityQuaternion());

    world.Release();
}

TEST_CASE("Positionable rotationable state fill missing data", "[networking, entity"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    ObjectLoader loader(&engine);

    GameWorld world;
    world.Init(nullptr, nullptr);

    Entity::Brush* brush = nullptr;
    loader.LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0, &brush);
    
    REQUIRE(brush != nullptr);

    brush->SetPos(Float3(0, 0, 0));

    auto initialstate = PositionableRotationableDeltaState::CaptureState(*brush, 0);

    // First position //
    brush->SetPos(Float3(50, 25, 0));

    auto firstcheck = PositionableRotationableDeltaState::CaptureState(*brush, 1);

    sf::Packet packet1;

    firstcheck->CreateUpdatePacket(initialstate.get(), packet1);

    auto firstresult = make_shared<PositionableRotationableDeltaState>(1, packet1);

    CHECK_FALSE((firstresult->ValidFields & PRDELTA_ALL_UPDATED) == PRDELTA_ALL_UPDATED);

    CHECK((firstresult->ValidFields & PRDELTAUPDATED_POS_X));
    CHECK((firstresult->ValidFields & PRDELTAUPDATED_POS_Y));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_POS_Z));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_ROT_X));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_ROT_Y));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_ROT_Z));
    CHECK_FALSE((firstresult->ValidFields & PRDELTAUPDATED_ROT_W));

    
    // Second position //
    brush->SetPos(Float3(100, 0, 0));

    auto secondcheck = PositionableRotationableDeltaState::CaptureState(*brush, 2);

    sf::Packet packet2;

    secondcheck->CreateUpdatePacket(initialstate.get(), packet2);

    auto secondresult = make_shared<PositionableRotationableDeltaState>(2, packet2);

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
    
    
    filled = secondresult->FillMissingData(*brush->CaptureState(0));

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

void ConsumeTime(int reduce, int &time){
    EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_FRAME_BEGIN,
            new IntegerEventData(reduce)));

    time -= reduce;
}

TEST_CASE("TimedInterpolation correctly ticks through interpolations", "[entity, networking]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    ObjectLoader loader(&engine);

    GameWorld world;
    world.Init(nullptr, nullptr);

    Entity::Brush* brush = nullptr;
    loader.LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0, &brush);
    
    REQUIRE(brush != nullptr);

    brush->SetPos(Float3(0, 0, 0));

    auto initialstate = shared_ptr<PositionableRotationableDeltaState>(
        PositionableRotationableDeltaState::CaptureState(*brush, 0).release());

    CHECK_FALSE(brush->IsCurrentlyInterpolating());

    SECTION("Interpolate between all"){

        // Store states //
        auto olderstate = initialstate;

        for(int i = 0; i < 10; i++){

            brush->SetPos(Float3(5 + (10*i), 0, 0));
        
            auto currentcreated = shared_ptr<PositionableRotationableDeltaState>(
                PositionableRotationableDeltaState::CaptureState(*brush, i+1).release());

            brush->QueueInterpolation(olderstate, currentcreated, TICKSPEED);

            olderstate = currentcreated;
        }

        brush->SetPos(Float3(0, 0, 0));

        CHECK(brush->IsCurrentlyInterpolating());

        // Now we need to fake frames worth of time //
        int time = TICKSPEED*10;

        int counter = 0;

        auto numbers = {12, 30, 20, 50, 10, 5, 6, 8, 12, 84, 120, 10, 20, 20, 20,
                        40, 33};

        for(auto currentreduce : numbers){
            
            ConsumeTime(currentreduce, time);
            counter += currentreduce;
        }

        CHECK(counter == TICKSPEED*10);
        CHECK(time <= 0);

        CHECK(brush->GetPosX() == Approx(95));
        CHECK_FALSE(brush->IsCurrentlyInterpolating());
    }

    SECTION("Interpolate between every 2 states"){


        
    }

    world.Release();
}



