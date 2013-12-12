#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TRIANGLEDELAUNAY
#include "TriangulateDelaunay.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "RayTracing.h"

DLLEXPORT  vector<Float3> Leviathan::DelaunayTriangulator::RunTriangulation(const vector<Float3> &inputface){
	// check for regular shapes for easy triangulation //

	bool CanUseSimple = true;

	// check for complex shape //
	for(unsigned int i = 0; i < inputface.size(); i++){
		int after = i+2;
		int before = i-2;
		// boundary roll check //
		if(after >= (int)inputface.size()){
			after -= inputface.size();
		}
		if(before < 0){
			after += (int)inputface.size();
		}

		// get points //
		const Float3& beforepoint = inputface[before];
		const Float3& currentpoint = inputface[i];
		const Float3& nextpoint = inputface[after];

		// check are lines between these points inside the polygon //
		if(!RayTracing::IsLineInsidePolygon(inputface, beforepoint, currentpoint) || !RayTracing::IsLineInsidePolygon(inputface, currentpoint, nextpoint)){
			CanUseSimple = false;
		}
		if(!CanUseSimple)
			break;
	}
	// check for z angle differing //
	float lastangle = -1;
	for(unsigned int i = 0; i < inputface.size(); i++){
		const Float3& point1 = inputface[i];
		for(unsigned int a = 0; a < inputface.size(); a++){
			const Float3& point2 = inputface[a];
			// get angle //
			float angle = (Float3(0,0, point1.Z)-Float3(0,0, point2.Z)).HAdd();
			if(angle < 0)
				angle *= -1;
			if(lastangle != -1){
				if(lastangle-angle > EPSILON || lastangle-angle < EPSILON){
					// not on same plane //
					CanUseSimple = false;
				}
			}
			if(!CanUseSimple)
				break;
		}
		if(!CanUseSimple)
			break;
	}

	if(CanUseSimple){
		// place a point in middle and draw triangles between 2 points on the outside ring and the middle point //
		vector<Float3> result;

		// this approach is wrong, don't use this //
#ifdef _WIN32
		throw exception("don't use this approach, just don't add new points");
#else
        throw bad_exception();
#endif

		//// get average point in polygon //
		//Float3 average(0);

		//for(unsigned int i = 0; i < inputface.size(); i++){
		//	average = average + inputface[i];
		//}
		//average /= (float)inputface.size();

		//result.push_back(average);

		//// create triangles //
		//for(unsigned int i = 0; i < inputface.size(); i++){
		//	int next = i+1;
		//	if(next >= (int)inputface.size())
		//		next = 0;
		//	// create //
		//	result.push_back(inputface[i]);
		//	result.push_back(inputface[next]);
		//	result.push_back(result[0]); // middle point //
		//}
		//// remove first and triangles are good to go //
		//result.erase(result.begin());
		//return result;
	}
	// needs to run more advanced algorithm //
#ifdef _WIN32
	throw exception("this neithers");
#else
    throw bad_exception();
#endif

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


