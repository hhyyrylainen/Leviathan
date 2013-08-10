#ifndef LEVIATHAN_CAMERA
#define LEVIATHAN_CAMERA
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

	class ViewCamera{
	public:
		DLLEXPORT ViewCamera();
		DLLEXPORT ~ViewCamera();
		
		DLLEXPORT void SetPosition(Float3 pos);
		DLLEXPORT void SetRotation(Float3 rotation);

		DLLEXPORT Float3 GetPosition();
		DLLEXPORT Float3 GetRotation();

		DLLEXPORT void UpdateMatrix();
		DLLEXPORT void GetViewMatrix(D3DXMATRIX& get);
		DLLEXPORT void GetStaticViewMatrix(D3DXMATRIX& get);
		DLLEXPORT void CreateStaticViewMatrix();
	private:
		Float3 Position;
		Float3 Rotation;

		D3DXMATRIX ViewMatrix;
		D3DXMATRIX StaticView;

		bool StaticViewCreated;

	};

}
#endif