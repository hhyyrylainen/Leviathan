// ------------------------------------ //
#include "Ray.h"

#include "Plane.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT std::tuple<bool, float> Ray::CalculateIntersection(const Plane& plane) const
{
    return plane.CalculateIntersection(*this);
}
