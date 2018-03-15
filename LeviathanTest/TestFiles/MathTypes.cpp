#include "Common/Types.h"

#include "catch.hpp"

using namespace Leviathan;


TEST_CASE("Quaternion math works", "[math]"){

    SECTION("Required vector operations"){

        const Float3 vector1 = Float3(0, 1, 2).Normalize();
        const Float3 vector2 = Float3(2, 0.5, 0.125).Normalize();

        const Ogre::Vector3 ogreV1 = vector1;
        const Ogre::Vector3 ogreV2 = vector2;

        CHECK(vector1 == ogreV1);
        CHECK(vector2 == ogreV2);

        CHECK(vector1.Cross(vector2) == ogreV1.crossProduct(ogreV2));
    }

    Ogre::Quaternion ogreQuat(Ogre::Radian(0.125f), Ogre::Vector3(1, 0.5, 1).normalisedCopy());

    Float4 levQuat = ogreQuat;

    CHECK(levQuat.X == ogreQuat.x);
    CHECK(levQuat.Y == ogreQuat.y);
    CHECK(levQuat.Z == ogreQuat.z);
    CHECK(levQuat.W == ogreQuat.w);
    CHECK(levQuat == ogreQuat);
    
    Ogre::Vector3 toRotate(0, 0, -1);

    const Float3 rotatedOgre = ogreQuat * toRotate;
    const Float3 rotatedLev = levQuat.RotateVector(toRotate);

    CHECK(rotatedOgre == rotatedLev);
}
