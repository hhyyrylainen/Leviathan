//! \file Tests for various supporting GUI methods. Doesn't actually
//! try any rendering or anything like that

#include "Newton/PhysicalWorld.h"
#include "Newton/PhysicsMaterialManager.h"
#include "Newton/NewtonManager.h"
#include "Generated/StandardWorld.h"

#include "../PartialEngine.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;


TEST_CASE("Physics world creates collisions", "[physics]"){

    PartialEngine<false> engine;

    NewtonManager newtonInstance;

    PhysicsMaterialManager physMan(&newtonInstance);

    REQUIRE(NewtonManager::Get());
    StandardWorld world;

    REQUIRE(world.Init(NETWORKED_TYPE::Client, nullptr, nullptr));

    PhysicalWorld* physWorld = world.GetPhysicalWorld();
    REQUIRE(physWorld);

    NewtonCollision* collision1 = physWorld->CreateCompoundCollision();

    REQUIRE(collision1);

    NewtonCollision* collision2 = physWorld->CreateSphere(1);

    REQUIRE(collision2);

    physWorld->DestroyCollision(collision1);
    physWorld->DestroyCollision(collision2);
}

TEST_CASE("Physics spheres fall down", "[physics][entity]"){

    PartialEngine<false> engine;

    NewtonManager newtonInstance;

    PhysicsMaterialManager physMan(&newtonInstance);

    REQUIRE(NewtonManager::Get());
    StandardWorld world;

    REQUIRE(world.Init(NETWORKED_TYPE::Client, nullptr, nullptr));

    PhysicalWorld* physWorld = world.GetPhysicalWorld();
    REQUIRE(physWorld);

    auto object1 = world.CreateEntity();

    auto& pos = world.Create_Position(object1, Float3(0, 10, 0), Float4::IdentityQuaternion());
    auto& physics = world.Create_Physics(object1, &world, pos, nullptr);
    physics.SetCollision(physWorld->CreateSphere(1));
    physics.CreatePhysicsBody(physWorld);
    physics.SetMass(10);

    // Let it run a bit
    world.Tick(1);
    world.Tick(2);

    // And check that it has fallen a bit
    CHECK(pos.Members._Position != Float3(0, 10, 0));
    CHECK(pos.Members._Position.Y < 9.9f);

    world.Release();
}

