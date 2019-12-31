// ------------------------------------ //
#include "Plane.h"

#include "Ray.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT std::tuple<bool, float> Plane::CalculateIntersection(const Ray& ray) const
{
    // This code is copied from bs::framework with modifications, see License.txt for details
    const float denom = Normal.Dot(ray.GetDirection());

    if(std::abs(denom) < EPSILON) {
        // Parallel
        return std::make_tuple(false, 0.0f);
    } else {
        const float nom = Normal.Dot(ray.GetOrigin()) - Distance;
        const float t = -(nom / denom);
        return std::make_tuple(t >= 0.f, t);
    }
}
