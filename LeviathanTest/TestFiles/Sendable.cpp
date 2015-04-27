#include "PartialEngine.h"

#include "Entities/Bases/BaseSendableEntity.h"
#include "Entities/GameWorld.h"
#include "Handlers/ObjectLoader.h"
#include "Entities/Objects/Brush.h"
#include "Common/SFMLPackets.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

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

        REQUIRE(first);
        REQUIRE(second);
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

        REQUIRE(first);
        REQUIRE(second);
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

        REQUIRE(first);
        REQUIRE(second);
        CHECK(progress == 0.125f);
        CHECK(first->Tick == 3);
        CHECK(second->Tick == 5);
    }

    SECTION("Correct exception is thrown"){

        for(auto i : {3, 5, 7, 8}){
        
            auto state = brush->CaptureState(i);

            sf::Packet packet;

            state->CreateUpdatePacket(firststate.get(), packet);

            REQUIRE(brush->LoadUpdateFromPacket(packet, i, i-1));
        }
        
        shared_ptr<ObjectDeltaStateData> first;
        shared_ptr<ObjectDeltaStateData> second;

        float progress = 0.25f;
        
        REQUIRE_THROWS_AS(brush->GetServerSentStates(first, second, 1, progress), InvalidState);
    }


    
    world.Release();
}


TEST_CASE("Brush listens to and applies client interpolation events", "[entity, networking]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    ObjectLoader loader(&engine);

    GameWorld world;
    world.Init(nullptr, nullptr);

    Entity::Brush* brush = nullptr;
    loader.LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0, &brush);

    brush->SetPos(Float3(0, 0, 0));
    
    REQUIRE(brush != nullptr);

    auto firststate = brush->CaptureState(0);
    
    // Create multiple instances of the same state with different ticks //

    SECTION("No states missing"){
        
        for(auto i : {1, 2, 3, 4, 5}){

            brush->SetPos(Float3(i*10, 0, 0));
            
            auto state = brush->CaptureState(i);

            sf::Packet packet;

            state->CreateUpdatePacket(firststate.get(), packet);

            REQUIRE(brush->LoadUpdateFromPacket(packet, i, i-1));
        }

        SECTION("Basic position updating"){
            
            brush->SetPos(Float3(0, 0, 0));

            EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_CLIENT_INTERPOLATION,
                    new ClientInterpolationEventData(3, 0.5f*TICKSPEED)));

            // Position should have changed //
            CHECK(brush->GetPosX() == Approx(35));
        }

        SECTION("Unlinking and relinking when new states arrive"){
            
            // This should unlink the listener //
            EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_CLIENT_INTERPOLATION,
                    new ClientInterpolationEventData(5, 0.5f*TICKSPEED)));

            brush->SetPos(Float3(0, 0, 0));

            // This shoulnd't apply //
            EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_CLIENT_INTERPOLATION,
                    new ClientInterpolationEventData(3, 0.5f*TICKSPEED)));

            CHECK(brush->GetPosX() == 0);

            // This should relink the listener //
            brush->SetPos(Float3(60, 0, 0));
            
            auto state = brush->CaptureState(6);

            sf::Packet packet;

            state->CreateUpdatePacket(firststate.get(), packet);

            REQUIRE(brush->LoadUpdateFromPacket(packet, 6, 5));

            // And this should now work //
            EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_CLIENT_INTERPOLATION,
                    new ClientInterpolationEventData(5, 0.5f*TICKSPEED)));

            CHECK(brush->GetPosX() == Approx(55));
        }
    }
    
    world.Release();
}
