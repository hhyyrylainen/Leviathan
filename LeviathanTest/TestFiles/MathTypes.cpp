#include "Common/Types.h"

#include "bsfUtility/Math/BsQuaternion.h"
#include "bsfUtility/Math/BsVector3.h"

#include "catch.hpp"

using namespace Leviathan;


TEST_CASE("Quaternion math works", "[math]")
{

    SECTION("Required vector operations")
    {

        const Float3 vector1 = Float3(0, 1, 2).Normalize();
        const Float3 vector2 = Float3(2, 0.5, 0.125).Normalize();

        const bs::Vector3 bsV1 = vector1;
        const bs::Vector3 bsV2 = vector2;

        CHECK(vector1 == bsV1);
        CHECK(vector2 == bsV2);

        CHECK(vector1.Cross(vector2) == bsV1.cross(bsV2));
    }

    bs::Quaternion bsQuat(bs::Vector3::normalize(bs::Vector3(1, 0.5, 1)), bs::Radian(0.125f));

    Float4 levQuat = bsQuat;

    CHECK(levQuat.X == bsQuat.x);
    CHECK(levQuat.Y == bsQuat.y);
    CHECK(levQuat.Z == bsQuat.z);
    CHECK(levQuat.W == bsQuat.w);
    CHECK(levQuat == bsQuat);

    bs::Vector3 toRotate(0, 0, -1);

    const Float3 rotatedBs = bsQuat.rotate(toRotate);
    const Float3 rotatedLev = levQuat.RotateVector(toRotate);

    CHECK(rotatedBs == rotatedLev);
}
