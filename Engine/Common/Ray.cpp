// ------------------------------------ //
#include "Ray.h"

#include "Matrix.h"
#include "Plane.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT std::tuple<bool, float> Ray::CalculateIntersection(const Plane& plane) const
{
    return plane.CalculateIntersection(*this);
}
// ------------------------------------ //
DLLEXPORT void Ray::Transform(const Matrix4& matrix)
{
    Origin = matrix.Multiply(Origin);
    const Float3 end = matrix.Multiply(GetPoint(1.0f));

    Direction = (end - Origin).Normalize();
}

DLLEXPORT void Ray::TransformAffine(const Matrix4& matrix)
{
    Origin = matrix.MultiplyAffine(Origin);
    const Float3 end = matrix.MultiplyAffine(GetPoint(1.0f));

    Direction = (end - Origin).Normalize();
}
