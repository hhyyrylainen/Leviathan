#ifndef LEVIATHAN_MATH_RAYTRACING
#define LEVIATHAN_MATH_RAYTRACING
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class RayTracing : public Object{
	public:
		DLLEXPORT static bool IsLineInsidePolygon(const vector<Float3>& polygon, const Float3& point1, const Float3& point2);
		//DLLEXPORT static int CountRayIntersectCount(const vector<Float3>& polygon // not used//


	private:
		RayTracing();
		~RayTracing();
	};

}
#endif
