#include "PartialEngine.h"

#include "Entities/Bases/BaseSendableEntity.h"
#include "Entities/GameWorld.h"
#include "Handlers/ObjectLoader.h"
#include "Entities/Objects/Brush.h"
#include "Common/SFMLPackets.h"

#include "catch.hpp"

using namespace Leviathan;

TEST_CASE("Sendable get correct server states", "[entity, networking]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    ObjectLoader loader(&engine);

    GameWorld world;
    world.Init(nullptr, nullptr);

    Entity::Brush* brush = nullptr;
    loader.LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0, &brush);
    
    REQUIRE(brush != nullptr);

    auto firststate = brush->CaptureState(0);
    
    // Create multiple instances of the same state with different ticks //

    SECTION("Only required are stored"){

        for(auto i : {1, 2}){
        
            auto state = brush->CaptureState(i);

            sf::Packet packet;

            state->CreateUpdatePacket(firststate.get(), packet);

            REQUIRE(brush->LoadUpdateFromPacket(packet, i, i-1));
        }

        shared_ptr<ObjectDeltaStateData> first;
        shared_ptr<ObjectDeltaStateData> second;

        float progress = 0.35f;
        
        REQUIRE_NOTHROW(brush->GetServerSentStates(first, second, 1, progress));

        CHECK(progress == 0.35f);
        CHECK(first->Tick == 1);
        CHECK(second->Tick == 2);
    }

    SECTION("No states in between missing"){
        
        for(auto i : {1, 2, 3, 4, 5}){
        
            auto state = brush->CaptureState(i);

            sf::Packet packet;

            state->CreateUpdatePacket(firststate.get(), packet);

            REQUIRE(brush->LoadUpdateFromPacket(packet, i, i-1));
        }

        shared_ptr<ObjectDeltaStateData> first;
        shared_ptr<ObjectDeltaStateData> second;

        float progress = 0.25f;
        
        REQUIRE_NOTHROW(brush->GetServerSentStates(first, second, 3, progress));

        CHECK(progress == 0.25f);
        CHECK(first->Tick == 3);
        CHECK(second->Tick == 4);
    }


    SECTION("First second state is missing"){
        
        for(auto i : {1, 2, 3, 5}){
        
            auto state = brush->CaptureState(i);

            sf::Packet packet;

            state->CreateUpdatePacket(firststate.get(), packet);

            REQUIRE(brush->LoadUpdateFromPacket(packet, i, i-1));
        }

        shared_ptr<ObjectDeltaStateData> first;
        shared_ptr<ObjectDeltaStateData> second;

        float progress = 0.25f;
        
        REQUIRE_NOTHROW(brush->GetServerSentStates(first, second, 3, progress));

        CHECK(progress == 0.125f);
        CHECK(first->Tick == 3);
        CHECK(second->Tick == 5);
    }
    
    world.Release();
}
