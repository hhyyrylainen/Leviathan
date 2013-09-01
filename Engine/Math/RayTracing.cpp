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
	// normalize //
	Float3 direction = temp.Normalize();

	// go through all points in the line //
	Float3 currentpoint(point1.X, point1.Y, point1.Z);
	bool IsInside = true;
	// loop through locations //
	float distance = 9;
	while(!((distance < EPSILON) && (distance > -EPSILON))){

		// increment //
		currentpoint += direction;
		// check point //
		if(!MMath::IsPointInsidePolygon(polygon, currentpoint)){
			IsInside = false;
			break;
		}

		// check distance //
		distance = (currentpoint-point2).HAdd();
	}
	return IsInside;
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


