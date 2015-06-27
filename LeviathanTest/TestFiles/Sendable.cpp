#include "PartialEngine.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"
#include "Handlers/ObjectLoader.h"
#include "Handlers/EntitySerializerManager.h"
#include "Common/SFMLPackets.h"
#include "Entities/CommonStateObjects.h"
#include "Entities/Serializers/SendableEntitySerializer.h"
#include "Networking/NetworkResponse.h"


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

        const Received::StoredState* first;
        const Received::StoredState* second;

        float progress = 0.35f;
        
        REQUIRE_NOTHROW(received.GetServerSentStates(&first, &second, 1, progress));

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

        const Received::StoredState* first;
        const Received::StoredState* second;

        float progress = 0.25f;
        
        REQUIRE_NOTHROW(received.GetServerSentStates(&first, &second, 3, progress));

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

        const Received::StoredState* first;
        const Received::StoredState* second;

        float progress = 0.25f;
        
        REQUIRE_NOTHROW(received.GetServerSentStates(&first, &second, 3, progress));

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
        
        const Received::StoredState* first;
        const Received::StoredState* second;

        float progress = 0.25f;
        
        REQUIRE_THROWS_AS(received.GetServerSentStates(&first, &second, 1, progress),
            InvalidState);
    }

    SECTION("Specified tick is missing but older exist"){

        for(auto i : {1, 2, 4}){
        
            auto state = PositionDeltaState::CaptureState(position, i);

            sf::Packet packet;

            packet << static_cast<int32_t>(SENDABLE_TYPE_BRUSH);

            state->CreateUpdatePacket(firststate.get(), packet);

            GUARD_LOCK_OTHER_NAME((&world), worldlock);
            REQUIRE(serializer.ApplyUpdateFromPacket(&world, worldlock, brush, i, i-1, packet));
        }

        const Received::StoredState* first = nullptr;
        const Received::StoredState* second = nullptr;

        float progress = 0.25f;
        
        REQUIRE_NOTHROW(received.GetServerSentStates(&first, &second, 3, progress));

        REQUIRE(first);
        REQUIRE(second);
        CHECK(progress == 0.625f);
        CHECK(first->Tick == 2);
        CHECK(second->Tick == 4);

        progress = 0.75f;
        
        REQUIRE_NOTHROW(received.GetServerSentStates(&first, &second, 3, progress));

        CHECK(progress == 0.875);
    }

    
    world.Release();
}


TEST_CASE("World interpolation system works with Brush", "[entity, networking]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    SendableEntitySerializer serializer;

    REQUIRE(Engine::Get() != nullptr);
    
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

            // Engine progress
            world.RunFrameRenderSystems(3, 0.5f*TICKSPEED);

            // Position should have changed //
            CHECK(position._Position.X == Approx(35));
        }

        SECTION("Unmarking and remarking when new states arrive"){
            
            // This should unlink the listener //

            // Engine progress
            world.RunFrameRenderSystems(5, 0.5f*TICKSPEED);
            

            position._Position = Float3(0, 0, 0);

            // This shoulnd't apply //
            // Engine progress
            world.RunFrameRenderSystems(3, 0.5f*TICKSPEED);

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

            // Engine progress
            world.RunFrameRenderSystems(5, 0.5f*TICKSPEED);

            CHECK(position._Position.X == Approx(55));
        }
    }
    
    world.Release();
}

TEST_CASE("GameWorld properly loads and applies state packets", "[networking, entity]"){

    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;

    SendableEntitySerializer serializer;
    EntitySerializerManager manager;
    manager.AddSerializer(&serializer);

    REQUIRE(Engine::Get() != nullptr);
    
    GameWorld world;
    world.Init(nullptr, nullptr);

    REQUIRE(world.GetObjectCount() == 0);

    auto brush = ObjectLoader::LoadBrushToWorld(&world, "none", Float3(1, 1, 1), 50, 0,
        { Float3(0, 0, 0), Float4::IdentityQuaternion() });

    // TODO: make fake clients not register objects twice
    REQUIRE(world.GetObjectCount() == 2);

    REQUIRE(brush != 0);

    auto& position = world.GetComponent<Position>(brush);

    auto& sendable = world.GetComponent<Sendable>(brush);

    // Fake Received type
    auto& received = world.CreateReceived(brush, SENDABLE_TYPE_BRUSH);

    position._Position = Float3(12, 5, 2);

    // Create packet "as the server" //
    auto serverstate = PositionDeltaState::CaptureState(position, 1);

    shared_ptr<NetworkResponse> packet;

    SECTION("No reference tick"){

        auto datapacket = make_shared<sf::Packet>();

        (*datapacket) << static_cast<int32_t>(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY) <<
            static_cast<int32_t>(sendable.SendableHandleType);

        serverstate->CreateUpdatePacket(nullptr, *datapacket);

        packet = move(make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000));
        packet->GenerateEntityUpdateResponse(new NetworkResponseDataForEntityUpdate(
                0, brush, 1, -1, datapacket));
    }

    SECTION("With reference tick"){

        // First send the reference tick //
        position._Position = Float3(24, 0, 2);

        auto referencestate = PositionDeltaState::CaptureState(position, 0);
        
        auto datapacket = make_shared<sf::Packet>();

        (*datapacket) << static_cast<int32_t>(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY) <<
            static_cast<int32_t>(sendable.SendableHandleType);

        serverstate->CreateUpdatePacket(nullptr, *datapacket);

        auto referencepacket = move(make_shared<NetworkResponse>(-1,
                PACKET_TIMEOUT_STYLE_TIMEDMS, 1000));
        referencepacket->GenerateEntityUpdateResponse(new NetworkResponseDataForEntityUpdate(
                0, brush, 0, -1, datapacket));

        world.HandleEntityUpdatePacket(referencepacket,
            referencepacket->GetResponseDataForEntityUpdate());

        // Then the actual packet //
        datapacket = make_shared<sf::Packet>();

        (*datapacket) << static_cast<int32_t>(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY) <<
            static_cast<int32_t>(sendable.SendableHandleType);

        serverstate->CreateUpdatePacket(referencestate.get(), *datapacket);
        
        packet = move(make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000));
        packet->GenerateEntityUpdateResponse(new NetworkResponseDataForEntityUpdate(
                0, brush, 1, 0, datapacket));
    }

    REQUIRE(packet);
    REQUIRE(packet->GetResponseDataForEntityUpdate() != nullptr);

    // Then try to load it as the client //
    
    world.HandleEntityUpdatePacket(packet, packet->GetResponseDataForEntityUpdate());

    // This actually loads it //
    {
        GUARD_LOCK_OTHER(world);

        world.ApplyQueuedPackets(guard);
    }

    if(received.ClientStateBuffer.size() < 2){

        position._Position = Float3(10);

        auto firststate = PositionDeltaState::CaptureState(position, 0);

        received.ClientStateBuffer.push_back(Received::StoredState(firststate, firststate.get(),
                SENDABLE_TYPE_BRUSH));
    }

    position._Position = Float3(0);

    CHECK(received.ClientStateBuffer.size() == 2);

    // This may not get marked //
    received.Marked = true;
    
    // Now if it successfully loaded the position should change //
    world.RunFrameRenderSystems(0, 0.5*TICKSPEED);

    CHECK(position._Position != Float3(0));
}
