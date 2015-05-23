#include "PartialEngine.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"
#include "Handlers/ObjectLoader.h"
#include "Common/SFMLPackets.h"
#include "Entities/CommonStateObjects.h"
#include "Entities/Serializers/SendableEntitySerializer.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

TEST_CASE("Sendable get correct server states", "[entity, networking]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    SendableEntitySerializer serializer;

    GameWorld world;
    world.Init(nullptr, nullptr);

    auto brush = ObjectLoader::LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0,
        { Float3(0, 0, 0), Float4::IdentityQuaternion() });

    REQUIRE(brush != 0);

    auto& position = world.GetComponent<Position>(brush);

    // Fake Received type
    auto& received = world.CreateReceived(brush, SENDABLE_TYPE_BRUSH);

    auto firststate = PositionDeltaState::CaptureState(position, 0);
    
    // Create multiple instances of the same state with different ticks //

    SECTION("Only required are stored"){

        for(auto i : {1, 2}){
        
            auto state = PositionDeltaState::CaptureState(position, i);

            sf::Packet packet;

            packet << static_cast<int32_t>(SENDABLE_TYPE_BRUSH);

            state->CreateUpdatePacket(firststate.get(), packet);

            GUARD_LOCK_OTHER_NAME((&world), worldlock);
            REQUIRE(serializer.ApplyUpdateFromPacket(&world, worldlock, brush, i, i-1, packet));
        }

        REQUIRE(received.ClientStateBuffer.size() == 2);

        shared_ptr<ObjectDeltaStateData> first;
        shared_ptr<ObjectDeltaStateData> second;

        float progress = 0.35f;
        
        REQUIRE_NOTHROW(received.GetServerSentStates(first, second, 1, progress));

        REQUIRE(first);
        REQUIRE(second);
        CHECK(progress == 0.35f);
        CHECK(first->Tick == 1);
        CHECK(second->Tick == 2);
    }

    SECTION("No states in between missing"){
        
        for(auto i : {1, 2, 3, 4, 5}){
        
            auto state = PositionDeltaState::CaptureState(position, i);

            sf::Packet packet;

            packet << static_cast<int32_t>(SENDABLE_TYPE_BRUSH);

            state->CreateUpdatePacket(firststate.get(), packet);

            GUARD_LOCK_OTHER_NAME((&world), worldlock);
            REQUIRE(serializer.ApplyUpdateFromPacket(&world, worldlock, brush, i, i-1, packet));
        }

        shared_ptr<ObjectDeltaStateData> first;
        shared_ptr<ObjectDeltaStateData> second;

        float progress = 0.25f;
        
        REQUIRE_NOTHROW(received.GetServerSentStates(first, second, 3, progress));

        REQUIRE(first);
        REQUIRE(second);
        CHECK(progress == 0.25f);
        CHECK(first->Tick == 3);
        CHECK(second->Tick == 4);
    }


    SECTION("First second state is missing"){
        
        for(auto i : {1, 2, 3, 5}){
        
            auto state = PositionDeltaState::CaptureState(position, i);

            sf::Packet packet;

            packet << static_cast<int32_t>(SENDABLE_TYPE_BRUSH);

            state->CreateUpdatePacket(firststate.get(), packet);

            GUARD_LOCK_OTHER_NAME((&world), worldlock);
            REQUIRE(serializer.ApplyUpdateFromPacket(&world, worldlock, brush, i, i-1, packet));
        }

        shared_ptr<ObjectDeltaStateData> first;
        shared_ptr<ObjectDeltaStateData> second;

        float progress = 0.25f;
        
        REQUIRE_NOTHROW(received.GetServerSentStates(first, second, 3, progress));

        REQUIRE(first);
        REQUIRE(second);
        CHECK(progress == 0.125f);
        CHECK(first->Tick == 3);
        CHECK(second->Tick == 5);
    }

    SECTION("Correct exception is thrown"){

        for(auto i : {3, 5, 7, 8}){
        
            auto state = PositionDeltaState::CaptureState(position, i);

            sf::Packet packet;

            packet << static_cast<int32_t>(SENDABLE_TYPE_BRUSH);

            state->CreateUpdatePacket(firststate.get(), packet);

            GUARD_LOCK_OTHER_NAME((&world), worldlock);
            REQUIRE(serializer.ApplyUpdateFromPacket(&world, worldlock, brush, i, i-1, packet));
        }

        REQUIRE(received.ClientStateBuffer.size() == 4);
        
        shared_ptr<ObjectDeltaStateData> first;
        shared_ptr<ObjectDeltaStateData> second;

        float progress = 0.25f;
        
        REQUIRE_THROWS_AS(received.GetServerSentStates(first, second, 1, progress), InvalidState);
    }

    
    world.Release();
}


TEST_CASE("World interpolation system works with Brush", "[entity, networking]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    SendableEntitySerializer serializer;
    
    GameWorld world;
    world.Init(nullptr, nullptr);

    auto brush = ObjectLoader::LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0,
        { Float3(0, 0, 0), Float4::IdentityQuaternion() });

    REQUIRE(brush != 0);

    auto& position = world.GetComponent<Position>(brush);

    // Fake Received type
    auto& received = world.CreateReceived(brush, SENDABLE_TYPE_BRUSH);

    auto firststate = PositionDeltaState::CaptureState(position, 0);
    
    // Create multiple instances of the same state with different ticks //

    SECTION("No states missing"){
        
        for(auto i : {1, 2, 3, 4, 5}){

            position._Position = Float3(i*10, 0, 0);
            
            auto state = PositionDeltaState::CaptureState(position, i);

            sf::Packet packet;

            packet << static_cast<int32_t>(SENDABLE_TYPE_BRUSH);

            state->CreateUpdatePacket(firststate.get(), packet);

            GUARD_LOCK_OTHER_NAME((&world), worldlock);
            REQUIRE(serializer.ApplyUpdateFromPacket(&world, worldlock, brush, i, i-1, packet));
        }

        SECTION("Basic position updating"){
            
            position._Position = Float3(0, 0, 0);

            // Set tick
            world.Tick(3);

            // Engine progress
            engine.AdjustTickClock(0.5f*TICKSPEED);
            world.RunFrameRenderSystems();

            // Position should have changed //
            CHECK(position._Position.X == Approx(35));
        }

        SECTION("Unmarking and remarking when new states arrive"){
            
            // This should unlink the listener //
            world.Tick(3);

            // Engine progress
            engine.AdjustTickClock(0.5f*TICKSPEED);
            world.RunFrameRenderSystems();
            

            position._Position = Float3(0, 0, 0);

            // This shoulnd't apply //
            world.Tick(3);

            // Engine progress
            engine.AdjustTickClock(0.5f*TICKSPEED);
            world.RunFrameRenderSystems();

            CHECK(position._Position.X == 0);

            // This should relink the listener //
            position._Position = Float3(60, 0, 0);
            
            auto state = PositionDeltaState::CaptureState(position, 6);

            sf::Packet packet;

            packet << static_cast<int32_t>(SENDABLE_TYPE_BRUSH);

            state->CreateUpdatePacket(firststate.get(), packet);

            {
                GUARD_LOCK_OTHER_NAME((&world), worldlock);
                REQUIRE(serializer.ApplyUpdateFromPacket(&world, worldlock, brush, 6, 5, packet));
            }

            // And this should now work //
            world.Tick(5);

            // Engine progress
            engine.AdjustTickClock(0.5f*TICKSPEED);
            world.RunFrameRenderSystems();

            CHECK(position._Position.X == Approx(55));
        }
    }
    
    world.Release();
}
