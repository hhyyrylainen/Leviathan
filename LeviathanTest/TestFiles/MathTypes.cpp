#include "Common/Quaternion.h"
#include "Common/Types.h"

#include "catch.hpp"

using namespace Leviathan;


TEST_CASE("Quaternion math works", "[math]")
{
    SECTION("Required vector operations")
    {

        const Float3 vector1 = Float3(0, 1, 2).Normalize();
        const Float3 vector2 = Float3(2, 0.5, 0.125).Normalize();

        const Float3 bsV1 = vector1;
        const Float3 bsV2 = vector2;

        CHECK(vector1 == bsV1);
        CHECK(vector2 == bsV2);

        CHECK(vector1.Cross(vector2) == bsV1.Cross(bsV2));
    }

    Quaternion bsQuat(Float3(1, 0.5, 1).Normalize(), Radian(0.125f));

    Quaternion levQuat = bsQuat;

    CHECK(levQuat.X == bsQuat.X);
    CHECK(levQuat.Y == bsQuat.Y);
    CHECK(levQuat.Z == bsQuat.Z);
    CHECK(levQuat.W == bsQuat.W);
    CHECK(levQuat == bsQuat);

    Float3 toRotate(0, 0, -1);

    const Float3 rotatedBs = bsQuat * toRotate;
    const Float3 rotatedLev = levQuat * toRotate;

    CHECK(rotatedBs == rotatedLev);
}
