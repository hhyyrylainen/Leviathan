#ifndef LEVIATHAN_COMMON_MATH
#define LEVIATHAN_COMMON_MATH
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <cmath>  /* for std::abs(double) */



namespace Leviathan{
	class MMath{
	public:
		static DLLEXPORT long double CoordinateDistance(POINT pos1,POINT pos2);
		static DLLEXPORT float CoordinateDistance(float x1, float x2,float y1, float y2);
		static DLLEXPORT double AngleBetweenPoints(POINT pos1, POINT pos2);
		static DLLEXPORT double AngleBetweenPoints(float x1, float x2,float y1, float y2);
		static DLLEXPORT POINT GetRelativeMousePos(HWND Hwnd);
		static DLLEXPORT float RandomNumber(float Min, float Max);

		DLLEXPORT static bool IsPointInsidePolygon(const vector<Float3>& polygon, const Float3& point);

		// greatest common divisor, courtesy of Wikipedia
		DLLEXPORT static int GreatestCommonDivisor(int a, int b);


		// float and double comparison //
		DLLEXPORT static bool IsEqual(double x, double y);
		DLLEXPORT static bool IsEqual(float x, float y);
	};







}




#endif