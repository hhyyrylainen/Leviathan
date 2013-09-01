#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_FRUSTRUM
#include "Frustrum.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Frustrum::Frustrum(){

}
// ------------------------------------ //
void Frustrum::ConstructFrustum(float screendepth, D3DXMATRIX projectionmatrix, D3DXMATRIX viewmatrix){

	// calculate minimum z //
	float MinZ = -projectionmatrix._43 / projectionmatrix._33;
	float r = screendepth / (screendepth - MinZ);
	projectionmatrix._33 = r;
	projectionmatrix._43 = -r * MinZ;

	// Create the frustum matrix from the view matrix and updated projection matrix.
	D3DXMATRIX matrix;
	D3DXMatrixMultiply(&matrix, &viewmatrix, &projectionmatrix);

	// Calculate near plane of frustum.
	Planes[0].a = matrix._14 + matrix._13;
	Planes[0].b = matrix._24 + matrix._23;
	Planes[0].c = matrix._34 + matrix._33;
	Planes[0].d = matrix._44 + matrix._43;
	D3DXPlaneNormalize(&Planes[0], &Planes[0]);

	// Calculate far plane of frustum.
	Planes[1].a = matrix._14 - matrix._13; 
	Planes[1].b = matrix._24 - matrix._23;
	Planes[1].c = matrix._34 - matrix._33;
	Planes[1].d = matrix._44 - matrix._43;
	D3DXPlaneNormalize(&Planes[1], &Planes[1]);

	// Calculate left plane of frustum.
	Planes[2].a = matrix._14 + matrix._11; 
	Planes[2].b = matrix._24 + matrix._21;
	Planes[2].c = matrix._34 + matrix._31;
	Planes[2].d = matrix._44 + matrix._41;
	D3DXPlaneNormalize(&Planes[2], &Planes[2]);

	// Calculate right plane of frustum.
	Planes[3].a = matrix._14 - matrix._11; 
	Planes[3].b = matrix._24 - matrix._21;
	Planes[3].c = matrix._34 - matrix._31;
	Planes[3].d = matrix._44 - matrix._41;
	D3DXPlaneNormalize(&Planes[3], &Planes[3]);

	// Calculate top plane of frustum.
	Planes[4].a = matrix._14 - matrix._12; 
	Planes[4].b = matrix._24 - matrix._22;
	Planes[4].c = matrix._34 - matrix._32;
	Planes[4].d = matrix._44 - matrix._42;
	D3DXPlaneNormalize(&Planes[4], &Planes[4]);

	// Calculate bottom plane of frustum.
	Planes[5].a = matrix._14 + matrix._12;
	Planes[5].b = matrix._24 + matrix._22;
	Planes[5].c = matrix._34 + matrix._32;
	Planes[5].d = matrix._44 + matrix._42;
	D3DXPlaneNormalize(&Planes[5], &Planes[5]);
}
// ------------------------------------ //
bool Frustrum::CheckPoint(float x, float y, float z){
	// Check if the point is inside all six planes
	for(int i=0; i < 6; i++){
		D3DXVECTOR3 point(x,y,z);
	
		//if(D3DXPlaneDotCoord(&Planes[i], &D3DXVECTOR3(x, y, z)) < 0.0f){
		if(D3DXPlaneDotCoord(&Planes[i], &point) < 0.0f){
			return false;
		}
	}

	return true;
}

bool Frustrum::CheckCube(float XCenter, float YCenter, float ZCenter, float radius){
	D3DXVECTOR3 calculationvector(0.f,0.f,0.f);
	// Check if any one point of the cube is in the view frustum.
	for(int i = 0; i < 6; i++){
		// update vector with copy constructor and pass it //

		calculationvector = D3DXVECTOR3((XCenter - radius), (YCenter - radius), (ZCenter - radius));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter + radius), (YCenter - radius), (ZCenter - radius));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter - radius), (YCenter + radius), (ZCenter - radius));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter + radius), (YCenter + radius), (ZCenter - radius));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter - radius), (YCenter - radius), (ZCenter + radius));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter + radius), (YCenter - radius), (ZCenter + radius));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter - radius), (YCenter + radius), (ZCenter + radius));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}
		
		calculationvector = D3DXVECTOR3((XCenter + radius), (YCenter + radius), (ZCenter + radius));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		return false;
	}

	return true;
}

bool Frustrum::CheckSphere(float XCenter, float YCenter, float ZCenter, float radius){
	
	// Check if the radius of the sphere is inside the view frustum.
	for(int i = 0; i < 6; i++){
		D3DXVECTOR3 calculationvector(XCenter, YCenter, ZCenter);
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) < -radius){
			return false;
		}
	}

	return true;
}

bool Frustrum::CheckRectangle(float XCenter, float YCenter, float ZCenter, float XSize, float YSize, float ZSize){
	D3DXVECTOR3 calculationvector(0.f,0.f,0.f);
	// Check if any of the 6 planes of the rectangle are inside the view frustum.
	for(int i = 0; i < 6; i++){
		calculationvector = D3DXVECTOR3((XCenter - XSize), (YCenter - YSize), (ZCenter - ZSize));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter + XSize), (YCenter - YSize), (ZCenter - ZSize));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter - XSize), (YCenter + YSize), (ZCenter - ZSize));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter - XSize), (YCenter - YSize), (ZCenter + ZSize));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter + XSize), (YCenter + YSize), (ZCenter - ZSize));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter + XSize), (YCenter - YSize), (ZCenter + ZSize));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter - XSize), (YCenter + YSize), (ZCenter + ZSize));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		calculationvector = D3DXVECTOR3((XCenter + XSize), (YCenter + YSize), (ZCenter + ZSize));
		if(D3DXPlaneDotCoord(&Planes[i], &calculationvector) >= 0.0f){
			continue;
		}

		return false;
	}

	return true;
}
// ------------------------------------ //

// ------------------------------------ //