//! \file Tests for various supporting GUI methods. Doesn't actually
//! try any rendering or anything like that

#include "Generated/StandardWorld.h"
#include "Newton/NewtonConversions.h"
#include "Newton/NewtonManager.h"
#include "Newton/PhysicalWorld.h"
#include "Newton/PhysicsMaterialManager.h"

#include "../PartialEngine.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("Matrix conversions work", "[physics]")
{
    // TODO: test the conversions
    NewtonManager newtonInstance;

    PhysicsMaterialManager physMan(&newtonInstance);

    PhysicalWorld phys(nullptr);

    NewtonCollision* collision = phys.CreateSphere(1);
    REQUIRE(collision);

    NewtonBody* body = phys.CreateBodyFromCollision(collision);
    REQUIRE(body);

    Ogre::Matrix4 ogreMatrix;
    const auto originalPos = Float3(1, 6, 19);
    ogreMatrix.makeTrans(originalPos);
    const auto setMatrix = PrepareOgreMatrixForNewton(ogreMatrix);

    NewtonBodySetMatrix(body, &setMatrix[0][0]);

    float matrix[16];
    NewtonBodyGetMatrix(body, &matrix[0]);

    const auto matrixBack = NewtonMatrixToOgre(matrix);

    CHECK(matrixBack == ogreMatrix);

    const Float3 pos = ExtractNewtonMatrixTranslation(matrix);

    CHECK(pos == originalPos);

    phys.DestroyCollision(collision);
}

TEST_CASE("Physics world creates collisions", "[physics]")
{

    PartialEngine<false> engine;

    NewtonManager newtonInstance;

    PhysicsMaterialManager physMan(&newtonInstance);

    REQUIRE(NewtonManager::Get());
    StandardWorld world;
    world.SetRunInBackground(true);

    REQUIRE(world.Init(NETWORKED_TYPE::Client, nullptr));

    PhysicalWorld* physWorld = world.GetPhysicalWorld();
    REQUIRE(physWorld);

    NewtonCollision* collision1 = physWorld->CreateCompoundCollision();

    REQUIRE(collision1);

    NewtonCollision* collision2 = physWorld->CreateSphere(1);

    REQUIRE(collision2);

    physWorld->DestroyCollision(collision1);
    physWorld->DestroyCollision(collision2);
}

TEST_CASE("Physics spheres fall down", "[physics][entity]")
{

    PartialEngine<false> engine;

    NewtonManager newtonInstance;

    PhysicsMaterialManager physMan(&newtonInstance);

    REQUIRE(NewtonManager::Get());
    StandardWorld world;
    world.SetRunInBackground(true);

    REQUIRE(world.Init(NETWORKED_TYPE::Client, nullptr));

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

std::atomic<int> TestHit = 0;

int TestAABBCallback(const NewtonMaterial* material, const NewtonBody* body0,
    const NewtonBody* body1, int threadIndex)
{
    ++TestHit;
    return 1;
}

TEST_CASE("Physical material callbacks work", "[physics][entity]")
{
    TestHit = 0;

    PartialEngine<false> engine;

    NewtonManager newtonInstance;

    PhysicsMaterialManager physMan(&newtonInstance);

    // Setup materials
    auto planeMaterial = std::make_shared<Leviathan::PhysicalMaterial>("PlaneMaterial");
    auto boxMaterial = std::make_shared<Leviathan::PhysicalMaterial>("BoxMaterial");

    // Set callbacks //
    planeMaterial->FormPairWith(*boxMaterial).SetCallbacks(TestAABBCallback, nullptr);

    physMan.LoadedMaterialAdd(planeMaterial);
    physMan.LoadedMaterialAdd(boxMaterial);


    REQUIRE(NewtonManager::Get());
    StandardWorld world;
    world.SetRunInBackground(true);

    REQUIRE(world.Init(NETWORKED_TYPE::Client, nullptr));

    PhysicalWorld* physWorld = world.GetPhysicalWorld();
    REQUIRE(physWorld);

    const auto boxMaterialID = world.GetPhysicalMaterial("BoxMaterial");
    const auto planeMaterialID = world.GetPhysicalMaterial("PlaneMaterial");

    CHECK(boxMaterialID >= 0);
    CHECK(planeMaterialID >= 0);

    auto box = world.CreateEntity();

    auto& pos1 = world.Create_Position(box, Float3(0, 5, 0), Float4::IdentityQuaternion());
    auto& physics1 = world.Create_Physics(box, &world, pos1, nullptr);
    physics1.SetCollision(physWorld->CreateSphere(1));
    physics1.CreatePhysicsBody(physWorld, boxMaterialID);
    physics1.SetMass(10);
    physics1.JumpTo(pos1);

    auto plane = world.CreateEntity();

    auto& pos2 = world.Create_Position(plane, Float3(0, 0, 0), Float4::IdentityQuaternion());
    auto& physics2 = world.Create_Physics(plane, &world, pos2, nullptr);
    physics2.SetCollision(physWorld->CreateBox(10, 1, 10));
    physics2.CreatePhysicsBody(physWorld, planeMaterialID);
    physics2.JumpTo(pos2);

    CHECK(TestHit == 0);

    // Let it run a bit
    world.Tick(1);

    // Actually it needs to run for quite a while so run physics here
    world.GetPhysicalWorld()->SimulateWorldFixed(1000, 10);

    // Box should fall //
    CHECK(pos1.Members._Position != Float3(0, 5, 0));
    CHECK(pos1.Members._Position.Y < 2.f);

    // And check that it has hit
    CHECK(TestHit > 0);

    world.Release();
}
