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

	class mpuint;

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

		/*--------------------------------------
		Original Function written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
		Code Edited by Henri Hyyryläinen

		This function uses Fermat's Theorem 100 times to test the primeness of a
		(large) positive integer.
		----------------------------------------------------------------------------*/
		DLLEXPORT static bool IsPrime(const mpuint &p);

		// float and double comparison //
		DLLEXPORT static bool IsEqual(double x, double y);
		DLLEXPORT static bool IsEqual(float x, float y);
	};







}




#endif