#ifndef LEVIATHAN_RENDERING_FRUSTRUM
#define LEVIATHAN_RENDERING_FRUSTRUM
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class Frustrum : public Object{
	public:
		Frustrum();

		void ConstructFrustum(float screendepth, D3DXMATRIX projectionmatrix, D3DXMATRIX viewmatrix);

		bool CheckPoint(float x, float y, float z);
		bool CheckCube(float XCenter, float YCenter, float ZCenter, float radius);
		bool CheckSphere(float XCenter, float YCenter, float ZCenter, float radius);
		bool CheckRectangle(float XCenter, float YCenter, float ZCenter, float XSize, float YSize, float ZSize);

	private:
		D3DXPLANE Planes[6];
	};


}
#endif