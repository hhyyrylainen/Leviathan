#ifndef LEVIATHAN_RENDERING_BUMPMODEL
#define LEVIATHAN_RENDERING_BUMPMODEL
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseModel.h"
#include "logger.h"
#include <d3dx10math.h>
#include "TextureArray.h"

namespace Leviathan{

	class BumpModel : public BaseModel {
	public:
		DLLEXPORT BumpModel::BumpModel();
        DLLEXPORT BumpModel::~BumpModel();
		DLLEXPORT bool Init(ID3D11Device* dev, wstring modelfile, wstring texturefile1, wstring texturefile2);
		DLLEXPORT void Release();
		DLLEXPORT void Render(ID3D11DeviceContext* devcont);

		DLLEXPORT int GetIndexCount();
		DLLEXPORT ID3D11ShaderResourceView* GetColorTexture();
		DLLEXPORT ID3D11ShaderResourceView* GetNormalMapTexture();

		//RENDERMODELTYPE mtype;

	private:
		struct VertexType
		{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
			D3DXVECTOR3 normal;
			D3DXVECTOR3 tangent;
			D3DXVECTOR3 binormal;
		};

		struct ModelType
		{
			float x, y, z;
			float tu, tv;
			float nx, ny, nz;
			float tx, ty, tz;
			float bx, by, bz;
		};

		struct TempVertexType
		{
			float x, y, z;
			float tu, tv;
			float nx, ny, nz;
		};

		struct VectorType
		{
			float x, y, z;
		};
		// ------------------------ //
		bool InitBuffers(ID3D11Device* dev);

		void RenderBuffers(ID3D11DeviceContext* devcont);

		bool LoadTextures(ID3D11Device* dev, wstring texturefile1, wstring texturefile2);

		bool LoadModel(wstring modelfile);

		void CalculateModelVectors();
		void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType, VectorType&, VectorType&);
		// ------------------------ //
		bool Inited;

		ID3D11Buffer *VertexBuffer, *IndexBuffer;
		int VertexCount, IndexCount;
		ModelType* Model;
		TextureArray* ColorandMap;
		//Texture* ColorTexture;
		//Texture* NormalMapTexture;
	};

}
#endif