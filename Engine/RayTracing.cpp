#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_MATH_RAYTRACING
#include "RayTracing.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT bool Leviathan::RayTracing::IsLineInsidePolygon(const vector<Float3>& polygon, const Float3& point1, const Float3& point2){
	// create vector //
	Float3 temp = point2-point1;
	D3DXVECTOR3 direction(temp.X, temp.Y, temp.Z);
	// normalize //
	D3DXVec3Normalize(&direction, &direction);

	// go through all points in the line //
	D3DXVECTOR3 currentpoint(point1.X, point1.Y, point1.Z);
	bool IsInside = true;
	// loop through locations //
	float distance = 9;
	while(!((distance < EPSILON) && (distance > -EPSILON))){

		// increment //
		currentpoint += direction;
		// check point //
		if(!MMath::IsPointInsidePolygon(polygon, Float3(currentpoint.x, currentpoint.y, currentpoint.z))){
			IsInside = false;
			break;
		}

		// check distance //
		distance = currentpoint.x-point2.X+currentpoint.y-point2.Y+currentpoint.z-point2.Z;
	}
	return IsInside;
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


