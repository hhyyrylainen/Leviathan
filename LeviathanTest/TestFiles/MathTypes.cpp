#include "Common/DiligentConversions.h"
#include "Common/Matrix.h"
#include "Common/Quaternion.h"
#include "Common/Types.h"
#include "Rendering/Scene.h"

#include "DiligentCore/Common/interface/BasicMath.hpp"

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

TEST_CASE("View matrix creation works", "[math]")
{
    SECTION("Rotation matrix creation")
    {
        Quaternion quat = Quaternion::IDENTITY;

        Matrix3 rot;
        quat.ToRotationMatrix(rot);

        CHECK(rot[0][0] == 1);
        CHECK(rot[0][1] == 0);
        CHECK(rot[0][2] == 0);

        CHECK(rot[1][0] == 0);
        CHECK(rot[1][1] == 1);
        CHECK(rot[1][2] == 0);

        CHECK(rot[2][0] == 0);
        CHECK(rot[2][1] == 0);
        CHECK(rot[2][2] == 1);
    }

    SECTION("Identity view")
    {
        const auto view = Matrix4::View(Float3(0, 0, 0), Quaternion::IDENTITY);

        CHECK(view[0][0] == 1);
        CHECK(view[0][1] == 0);
        CHECK(view[0][2] == 0);
        CHECK(view[0][3] == 0);

        CHECK(view[1][0] == 0);
        CHECK(view[1][1] == 1);
        CHECK(view[1][2] == 0);
        CHECK(view[1][3] == 0);

        CHECK(view[2][0] == 0);
        CHECK(view[2][1] == 0);
        CHECK(view[2][2] == 1);
        CHECK(view[2][3] == 0);

        CHECK(view[3][0] == 0);
        CHECK(view[3][1] == 0);
        CHECK(view[3][2] == 0);
        CHECK(view[3][3] == 1);
    }

    SECTION("Same result as diligent")
    {
        constexpr float dist = 0.9f;
        Diligent::float4x4 diligentView = Diligent::Quaternion{0, 0, 0, 1}.ToMatrix() *
                                          Diligent::float4x4::Translation(0.f, 0.0f, dist);

        const auto view = Matrix4::View(Float3(0, 0, dist), Quaternion::IDENTITY);

        CHECK(view == MatrixFromDiligent(diligentView));
    }
}

TEST_CASE("Perspective matrix creation works", "[math]")
{
    SECTION("Same result as diligent")
    {
        float nearPlane = 0.1f;
        float farPlane = 100.f;
        float aspectRatio = static_cast<float>(1280) / static_cast<float>(720);
        bool gl = false;

        const auto diligentProjection = MatrixFromDiligent(
            Diligent::float4x4::Projection(PI / 4.f, aspectRatio, nearPlane, farPlane, gl));

        const auto projection = Matrix4::ProjectionPerspective(
            Radian(PI / 4.f), aspectRatio, nearPlane, farPlane, gl);

        CHECK(projection == diligentProjection);
    }
}

TEST_CASE("Scene node parent orientation properly stacks", "[math]")
{
    auto scene = Scene::MakeShared<Scene>();

    auto root = scene->GetRootSceneNode();
    REQUIRE(root);

    auto child = scene->CreateSceneNode();

    REQUIRE(child);

    SECTION("No transforms")
    {
        auto transform = child->GetWorldTransform();

        CHECK(transform.Translation == Float3(0, 0, 0));
        CHECK(transform.Scale == Float3(1, 1, 1));
        CHECK(transform.Orientation == Quaternion::IDENTITY);
    }

    SECTION("Y axis rotation on child")
    {
        const Quaternion childRotation(Float3::UnitYAxis, Degree(90));
        child->SetOrientation(childRotation);
        auto transform = child->GetWorldTransform();

        CHECK(transform.Orientation == childRotation);
    }

    SECTION("Cumulative Y axis rotation")
    {
        const Quaternion childRotation(Float3::UnitYAxis, Degree(90));
        const auto cumulative = childRotation * childRotation;

        root->SetOrientation(childRotation);
        child->SetOrientation(childRotation);
        auto transform = child->GetWorldTransform();

        CHECK(transform.Orientation == cumulative);
    }

    SECTION("X axis rotation on child")
    {
        const Quaternion childRotation(Float3::UnitXAxis, Degree(90));
        child->SetOrientation(childRotation);
        auto transform = child->GetWorldTransform();

        CHECK(transform.Orientation == childRotation);
    }
}
