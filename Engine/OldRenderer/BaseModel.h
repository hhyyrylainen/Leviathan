#ifndef LEVIATHAN_RENDER_BASEMODEL
#define LEVIATHAN_RENDER_BASEMODEL
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{
	enum RENDERMODELTYPE { MBASETYPE, MTEXTURETYPE, MBUMPTYPE };




	class BaseModel : public EngineComponent{
	public:
		DLLEXPORT BaseModel::BaseModel();
        DLLEXPORT BaseModel::~BaseModel();
		DLLEXPORT bool Init(ID3D11Device* dev, wstring modelfile, wstring texturefile1);
		DLLEXPORT void Release();
		DLLEXPORT void Render(ID3D11DeviceContext* devcont);

		DLLEXPORT int GetIndexCount();
		DLLEXPORT ID3D11ShaderResourceView* GetColorTexture();

		RENDERMODELTYPE mtype;
	};

}
#endif