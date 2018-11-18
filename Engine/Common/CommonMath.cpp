// ------------------------------------ //
#include "CommonMath.h"

#include "Utility/Random.h"
using namespace Leviathan;
// ------------------------------------ //
float MMath::CoordinateDistance(float x1, float x2, float y1, float y2)
{
    const auto distance = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

    return distance;
}

double MMath::AngleBetweenPoints(float x1, float x2, float y1, float y2)
{
    return atan2(y2 - y1, x2 - x1);
}
// ------------------------------------ //
int MMath::GreatestCommonDivisor(int a, int b)
{
    return (b == 0 ? a : GreatestCommonDivisor(b, a % b));
}
// ------------------------------------ //
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
    // according to OpenGL wiki //

    // some vectors for calculating final normal //
    const Float3 VecU = p2 - p1;
    const Float3 VecV = p3 - p1;

    // normalize before returning //
    return Float3(VecU.Y * VecV.Z - VecU.Z * VecV.Y, VecU.Z * VecV.X - VecU.X * VecV.Z,
        VecU.X * VecV.Y - VecU.Y * VecV.X)
        .Normalize();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::MMath::IsEqual(double x, double y)
{
    return abs(x - y) <= EPSILON * abs(x);
}

DLLEXPORT bool Leviathan::MMath::IsEqual(float x, float y)
{
    return abs(x - y) <= EPSILON * abs(x);
}
// ------------------------------------ //

/*--------------------------------------
Original Function written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
Code Edited by Henri Hyyryläinen
----------------------------------------*/
// DLLEXPORT  bool Leviathan::MMath::IsPrime(const mpuint &p){
//	mpuint pminus1(p);
//	pminus1 -= 1;
//	unsigned count = 101;
//	while (--count != 0)
//	{
//		mpuint r(p.length);
//		mpuint x(p.length);
//		{
//			for (unsigned i = 0; i < x.length; i++)
//				x.value[i] = (USHORT)Random::Get()->GetNumber(0, USHRT_MAX) << 8 |
//(USHORT)Random::Get()->GetNumber(0, USHRT_MAX);
//		}
//		x %= p;
//		if (x != 0)
//		{
//			mpuint::Power(x, pminus1, p, r);
//			if (r != 1)
//				return false;
//		}
//	}
//	return true;
//}
