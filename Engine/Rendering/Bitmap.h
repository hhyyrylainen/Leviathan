#ifndef LEVIATHAN_RENDERING_BITMAP
#define LEVIATHAN_RENDERING_BITMAP
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <d3dx10math.h>

#include "BaseModel.h"
#include "FileSystem.h"
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
#include "TextureArray.h"

namespace Leviathan{
	class RenderingBitmap : public EngineComponent{
	public:
		DLLEXPORT RenderingBitmap();
		DLLEXPORT ~RenderingBitmap();
	
		DLLEXPORT bool Init(ID3D11Device* device,  int screenwidth, int screenheight, wstring texture, wstring texture2, int bitmapwidth, int bitmapheight);
		DLLEXPORT void Release();
		DLLEXPORT void Render(ID3D11DeviceContext* devcont, int posx, int posy);

		DLLEXPORT int GetIndexCount();
		DLLEXPORT ID3D11ShaderResourceView* GetTexture();
		DLLEXPORT ID3D11ShaderResourceView** GetTextureArray();


		DLLEXPORT bool UsingMultitextures();
		//RENDERMODELTYPE mtype;

	private:
		struct VertexType{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
		};


		bool InitBuffers(ID3D11Device* device);
		bool UpdateBuffers(ID3D11DeviceContext* devcont, int x, int y);
		void RenderBuffers(ID3D11DeviceContext* devcont);

		bool LoadTextures(ID3D11Device* dev, wstring file, wstring file2);
		// ---------------------- //
		bool Inited;



		ID3D11Buffer* Vertexbuffer;
		ID3D11Buffer* Indexbuffer;
		int VertexCount, IndexCount;
		TextureArray* pTexture;

		int ScreenWidth, ScreenHeight;
		int BitmapWidth, BitmapHeight;
		int PreviousPosX, PreviousPosY;


	};
}
#endif