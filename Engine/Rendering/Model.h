#ifndef LEVIATHAN_RENDERING_MODEL
#define LEVIATHAN_RENDERING_MODEL
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseModel.h"
#include "FileSystem.h"
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
#include "TextureArray.h"

namespace Leviathan{
	class RenderModel : public BaseModel{
	public:
		DLLEXPORT RenderModel();
		DLLEXPORT ~RenderModel();
		
		DLLEXPORT bool Init(ID3D11Device* device, wstring modelfile, wstring texturefile, wstring texture2);
		DLLEXPORT void Release();
		DLLEXPORT void Render(ID3D11DeviceContext* devcont);

		DLLEXPORT int GetIndexCount();
		DLLEXPORT ID3D11ShaderResourceView* GetTexture();
		DLLEXPORT ID3D11ShaderResourceView** GetTextureArray();


		DLLEXPORT bool UsingMultitextures();
		//RENDERMODELTYPE mtype;

	private:
		struct VertexType{

			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
			D3DXVECTOR3 normal;
		};
		struct RenderModelType{

			float x, y, z;
			float tu, tv;
			float nx, ny, nz;
		};
		// loading //
		struct FaceType{
		public:
			int vIndex1, vIndex2, vIndex3;
			int tIndex1, tIndex2, tIndex3;
			int nIndex1, nIndex2, nIndex3;
		};
		struct VertexFormat{
			float x, y, z;
		};

		bool InitBuffers(ID3D11Device* device);
		void RenderBuffers(ID3D11DeviceContext* devcont);

		bool LoadTextures(ID3D11Device* dev, wstring file, wstring file2);
		bool LoadRenderModel(wstring file);
		// ---------------------- //
		bool Inited;



		ID3D11Buffer* Vertexbuffer;
		ID3D11Buffer* Indexbuffer;
		int VertexCount, IndexCount;
		TextureArray* pTexture;
		RenderModelType* pRenderModel;


	};
}
#endif