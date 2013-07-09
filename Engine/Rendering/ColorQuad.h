#ifndef LEVIATHAN_RENDERING_QUAD
#define LEVIATHAN_RENDERING_QUAD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ResolutionScaling.h"
#include "RenderSavedResource.h"
#include <d3dx10math.h>

#include "BaseModel.h"
#include "FileSystem.h"
#include "ShaderDataTypes.h"


#define COLORQUAD_VERTEXCOUNT						6

namespace Leviathan{
	class ColorQuad : public SavedResource{
	public:
		DLLEXPORT ColorQuad();
		DLLEXPORT ~ColorQuad();
	
		//DLLEXPORT void SetColors(ID3D11DeviceContext* devcont, Float4& col1, Float4& col2);
		DLLEXPORT bool Init(ID3D11Device* device, int screenwidth, int screenheight, int colorstyle = 1);
		DLLEXPORT void Release();

		//DLLEXPORT void Resize(int screenwidth, int screenheight, int quadwidth, int quadheight);
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont, float posx, float posy, int screenwidth, int screenheight, float quadwidth, 
			float quadheight, int Coordtype, int colorstyle = 1);

		DLLEXPORT inline int GetIndexCount(){
			return COLORQUAD_VERTEXCOUNT;
		}


	private:
		bool InitBuffers(ID3D11Device* device);

		//bool QuickBUpdate(ID3D11DeviceContext* devcont);
		bool UpdateBuffers(ID3D11DeviceContext* devcont, float posx, float posy, int screenwidth, int screenheight, float quadwidth, float quadheight, 
			int Coordtype, int colorstyle = 1);
		void RenderBuffers(ID3D11DeviceContext* devcont);

		bool LoadTextures(ID3D11Device* dev, wstring file, wstring file2);
		// ---------------------- //
		bool Inited;

		// rendering buffers //
		ID3D11Buffer* VertexBuffer;
		ID3D11Buffer* IndexBuffer;

		int Colorstyle;
		int ScreenWidth, ScreenHeight;
		float QuadWidth, QuadHeight;
		float PreviousPosX, PreviousPosY;
	};
}
#endif