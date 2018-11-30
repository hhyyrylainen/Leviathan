//! \file Tests for various supporting GUI methods. Doesn't actually
//! try any rendering or anything like that

#include "Generated/StandardWorld.h"
#include "Physics/PhysicalWorld.h"
#include "Physics/PhysicsMaterialManager.h"

#include "../PartialEngine.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("Physics world creates collisions", "[physics]")
{
    PartialEngine<false> engine;

    StandardWorld world(std::make_unique<PhysicsMaterialManager>());
    world.SetRunInBackground(true);

    REQUIRE(world.Init(WorldNetworkSettings::GetSettingsForClient(), nullptr));

    PhysicalWorld* physWorld = world.GetPhysicalWorld();
    REQUIRE(physWorld);

    auto collision1 = physWorld->CreateBox(1, 1, 1);

    REQUIRE(collision1);

    auto collision2 = physWorld->CreateSphere(1);

    REQUIRE(collision2);
}

TEST_CASE("Physics spheres fall down", "[physics][entity]")
{
    PartialEngine<false> engine;

    StandardWorld world(std::make_unique<PhysicsMaterialManager>());
    world.SetRunInBackground(true);

    REQUIRE(world.Init(WorldNetworkSettings::GetSettingsForClient(), nullptr));

    PhysicalWorld* physWorld = world.GetPhysicalWorld();
    REQUIRE(physWorld);

    auto object1 = world.CreateEntity();

    auto& pos = world.Create_Position(object1, Float3(0, 10, 0), Float4::IdentityQuaternion());
    auto& physics = world.Create_Physics(object1, pos);

    CHECK(physics.CreatePhysicsBody(physWorld, physWorld->CreateSphere(1), 10));

    // Let it run a bit
    world.Tick(1);
    world.Tick(2);
    world.Tick(3);
    world.Tick(4);
    world.Tick(5);

    // And check that it has fallen a bit
    CHECK(pos.Members._Position != Float3(0, 10, 0));
    CHECK(pos.Members._Position.Y < 9.9f);

    world.Release();
}

TEST_CASE("Plane constrainted physics sphere doesn't fall", "[physics][entity]")
{
    PhysicsMaterialManager materials;
    PhysicalWorld world(nullptr, &materials);

    auto body = world.CreateBodyFromCollision(world.CreateSphere(1), 10, nullptr);
    REQUIRE(body);

    body->ConstraintMovementAxises();

    body->SetPosition(Float3(0, 10, 0), Float4::IdentityQuaternion());

    // Let it run a bit
    world.SimulateWorld(0.1f);
    world.SimulateWorld(0.1f);
    world.SimulateWorld(0.1f);

    // And check that it hasn't fallen a bit
    CHECK(body->GetPosition() == Float3(0, 10, 0));

    world.DestroyBody(body.get());
}

std::atomic<int> TestHit = 0;

bool TestAABBCallback(PhysicalWorld& world, PhysicsBody& body1, PhysicsBody& body2)
{
    ++TestHit;
    return 1;
}

TEST_CASE("Physical material callbacks work", "[physics][entity]")
{
    TestHit = 0;

    PartialEngine<false> engine;

    auto physMan = std::make_unique<PhysicsMaterialManager>();

    // Setup materials
    auto planeMaterial = std::make_unique<Leviathan::PhysicalMaterial>("PlaneMaterial", 1);
    auto boxMaterial = std::make_unique<Leviathan::PhysicalMaterial>("BoxMaterial", 2);

    // Set callbacks //
    planeMaterial->FormPairWith(*boxMaterial).SetCallbacks(TestAABBCallback, nullptr);

    physMan->LoadedMaterialAdd(std::move(planeMaterial));
    physMan->LoadedMaterialAdd(std::move(boxMaterial));

    StandardWorld world(std::move(physMan));
    world.SetRunInBackground(true);

    REQUIRE(world.Init(WorldNetworkSettings::GetSettingsForClient(), nullptr));

    PhysicalWorld* physWorld = world.GetPhysicalWorld();
    REQUIRE(physWorld);

    const auto boxMaterialID = world.GetPhysicalMaterial("BoxMaterial");
    const auto planeMaterialID = world.GetPhysicalMaterial("PlaneMaterial");

    CHECK(boxMaterialID == 2);
    CHECK(planeMaterialID == 1);

    auto box = world.CreateEntity();

    const auto boxInitialPos = Float3(0, 5, 0);

    auto& pos1 = world.Create_Position(box, boxInitialPos, Float4::IdentityQuaternion());
    auto& physics1 = world.Create_Physics(box, pos1);

    CHECK(
        physics1.CreatePhysicsBody(physWorld, physWorld->CreateSphere(1), 10, boxMaterialID));

    // Verify initial spawn position
    CHECK(physics1.GetBody()->GetBody()->getCenterOfMassTransform().getOrigin() ==
          boxInitialPos);

    auto plane = world.CreateEntity();

    auto& pos2 = world.Create_Position(plane, Float3(0, 0, 0), Float4::IdentityQuaternion());
    auto& physics2 = world.Create_Physics(plane, pos2);

    CHECK(physics2.CreatePhysicsBody(
        physWorld, physWorld->CreateBox(10, 1, 10), 0, planeMaterialID));

    CHECK(TestHit == 0);

    // It needs to run for quite a while so run physics here
    world.GetPhysicalWorld()->SimulateWorld(1.f, 100);

    // Box should fall //
    CHECK(pos1.Members._Position != Float3(0, 5, 0));
    CHECK(pos1.Members._Position.Y < 2.f);
    // Doesn't fall through it
    CHECK(pos1.Members._Position.Y > -2.f);

    // And check that it has hit
    CHECK(TestHit > 0);

    world.Release();
}

TEST_CASE("Physical compound bodies work with callbacks", "[physics][entity]")
{
    TestHit = 0;

    PartialEngine<false> engine;

    auto physMan = std::make_unique<PhysicsMaterialManager>();

    constexpr auto material1 = 1;
    constexpr auto material2 = 2;

    // Setup materials
    auto material1Obj = std::make_unique<Leviathan::PhysicalMaterial>("1", material1);
    auto material2Obj = std::make_unique<Leviathan::PhysicalMaterial>("2", material2);

    // Set callbacks //
    material1Obj->FormPairWith(*material2Obj).SetCallbacks(TestAABBCallback, nullptr);

    physMan->LoadedMaterialAdd(std::move(material1Obj));
    physMan->LoadedMaterialAdd(std::move(material2Obj));

    StandardWorld world(std::move(physMan));
    world.SetRunInBackground(true);

    REQUIRE(world.Init(WorldNetworkSettings::GetSettingsForClient(), nullptr));

    PhysicalWorld* physWorld = world.GetPhysicalWorld();
    REQUIRE(physWorld);

    auto box = world.CreateEntity();

    auto& pos1 = world.Create_Position(box, Float3(-5, 5, 0), Float4::IdentityQuaternion());
    auto& physics1 = world.Create_Physics(box, pos1);

    auto shape1 = physWorld->CreateCompound();

    shape1->AddChildShape(physWorld->CreateBox(1, 1, 1));
    shape1->AddChildShape(physWorld->CreateBox(1, 1, 1), Float3(5, 0, 0));

    CHECK(physics1.CreatePhysicsBody(physWorld, shape1, 10, material1));

    auto plane = world.CreateEntity();

    auto shape2 = physWorld->CreateCompound();

    shape2->AddChildShape(physWorld->CreateBox(1, 1, 1));
    shape2->AddChildShape(physWorld->CreateBox(10, 1, 10));

    auto& pos2 = world.Create_Position(plane, Float3(5, 0, 0), Float4::IdentityQuaternion());
    auto& physics2 = world.Create_Physics(plane, pos2);

    CHECK(physics2.CreatePhysicsBody(physWorld, shape2, 0, material2));

    CHECK(TestHit == 0);

    // It needs to run for quite a while so run physics here
    world.GetPhysicalWorld()->SimulateWorld(1.f, 100);

    // Box should fall //
    CHECK(pos1.Members._Position != Float3(0, 5, 0));
    CHECK(pos1.Members._Position.Y < 2.f);

    // And check that it has hit
    CHECK(TestHit > 0);

    world.Release();
}

TEST_CASE(
    "Plane constrainted physics sphere can be moved with an impulse", "[physics][entity]")
{
    PhysicsMaterialManager materials;
    PhysicalWorld world(nullptr, &materials);

    auto body = world.CreateBodyFromCollision(world.CreateSphere(1), 10, nullptr);
    REQUIRE(body);

    body->ConstraintMovementAxises();

    body->SetPosition(Float3(0, 10, 0), Float4::IdentityQuaternion());

    // Let it run a bit
    world.SimulateWorld(0.1f);

    CHECK(body->GetPosition() == Float3(0, 10, 0));

    // Impulse
    body->GiveImpulse(Float3(42.6506f, 0, -90.4485f));

    // And run again to let it effect
    world.SimulateWorld(0.1f);
    world.SimulateWorld(0.1f);

    // And check that it hasn't fallen a bit
    CHECK(body->GetPosition().Y == 10);

    // And it has moved
    CHECK(body->GetPosition() != Float3(0, 10, 0));

    world.DestroyBody(body.get());
}
