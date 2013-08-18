#ifndef LEVIATHAN_RENDERINGPASSINFO
#define LEVIATHAN_RENDERINGPASSINFO
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ViewCamera.h"


namespace Leviathan{

	class RenderingPassInfo : public Object{
	public:
		DLLEXPORT RenderingPassInfo(const D3DXMATRIX &view, const D3DXMATRIX &proj, const D3DXMATRIX &world, ViewCamera* camera);
		DLLEXPORT ~RenderingPassInfo();


		DLLEXPORT inline D3DXMATRIX GetViewMatrix() const{
			return ViewMatrix;
		}
		DLLEXPORT inline D3DXMATRIX GetProjectionMatrix() const{
			return ProjectionMatrix;
		}
		DLLEXPORT inline D3DXMATRIX GetWorldMatrix() const{
			return WorldMatrix;
		}
		DLLEXPORT inline ViewCamera* GetCamera() const{
			return ActiveCamera;
		}
		DLLEXPORT inline RenderingPassInfo* SetViewMatrix(const D3DXMATRIX &view){
			ViewMatrix = view;
			return this;
		}
		DLLEXPORT inline RenderingPassInfo* SetProjectionMatrix(const D3DXMATRIX &proj){
			ProjectionMatrix = proj;
			return this;
		}
		DLLEXPORT inline RenderingPassInfo* SetWorldMatrix(const D3DXMATRIX &world){
			WorldMatrix = world;
			return this;
		}

	protected:


		D3DXMATRIX ViewMatrix;
		D3DXMATRIX ProjectionMatrix;
		D3DXMATRIX WorldMatrix;
		ViewCamera* ActiveCamera;
	};

}
#endif