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
    
    StandardWorld world;

    NewtonManager newtonInstance;

    PhysicsMaterialManager physMan(&newtonInstance);

    PhysicalWorld physWorld(&world);

    NewtonCollision* collision1 = physWorld.CreateCompoundCollision();

    REQUIRE(collision1);

    NewtonCollision* collision2 = physWorld.CreateSphere(1);

    REQUIRE(collision2);

    physWorld.DestroyCollision(collision1);
    physWorld.DestroyCollision(collision2);
}

