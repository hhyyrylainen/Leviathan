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
	D3DXVECTOR3 direction(temp.Val[0], temp.Val[1], temp.Val[2]);
	// normalize //
	D3DXVec3Normalize(&direction, &direction);

	// go through all points in the line //
	D3DXVECTOR3 currentpoint(point1.Val[0], point1.Val[1], point1.Val[2]);
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
		distance = currentpoint.x-point2.Val[0]+currentpoint.y-point2.Val[1]+currentpoint.z-point2.Val[2];
	}
	return IsInside;
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


