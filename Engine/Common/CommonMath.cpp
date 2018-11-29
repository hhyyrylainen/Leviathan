// ------------------------------------ //
#include "CommonMath.h"

#include "Utility/Random.h"
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT double MMath::AngleBetweenPoints(float x1, float x2, float y1, float y2) noexcept
{
    return atan2(y2 - y1, x2 - x1);
}

DLLEXPORT double MMath::AngleBetweenPoints(Float2 v1, Float2 v2) noexcept
{
    return MMath::AngleBetweenPoints(v1.X, v2.X, v1.Y, v2.Y);
}

DLLEXPORT bool Leviathan::MMath::IsPointInsidePolygon(
    const std::vector<Float3>& polygon, const Float3& point)
{
    // bool IsInside = false;
    // int i,j;
    // for(i = 0, j = polygon.size()-1; i < polygon.size(); j = i++){
    //	if(((polygon[i].Y > point.Y) != (polygon[j].Y > point.Y))
    //		&& (point.X < (polygon[j].X-polygon[i].X) * (point.Y-polygon[i].Y) /
    //(polygon[j].Y-polygon[i].Y) + polygon[i].X)) 		IsInside = !IsInside;
    //}
    // return IsInside;

    // better? one http://devmaster.net/forums/topic/5213-point-in-polygon-test/
    Float3 p = (polygon[polygon.size() - 1] - point).Cross(polygon[0] - point);
    for(unsigned int i = 0; i < polygon.size() - 1; i++) {

        Float3 q = (polygon[i] - point).Cross(polygon[i + 1] - point);
        if(p.Dot(q) < 0)
            return false;
    }
    return true;
}

DLLEXPORT Float3 Leviathan::MMath::CalculateNormal(
    const Float3& p1, const Float3& p2, const Float3& p3)
{
    // some vectors for calculating final normal //
    const Float3 VecU = p2 - p1;
    const Float3 VecV = p3 - p1;

    // normalize before returning //
    return VecU.Cross(VecV).Normalize();
}
