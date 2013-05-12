#ifndef LEVIATHAN_TEXTURESHADER
#define LEVIATHAN_TEXTURESHADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "FileSystem.h"
#include "Window.h"
#include <d3dx11async.h>

#include "ShaderDataTypes.h"

namespace Leviathan{

	enum ShaderType { ShaderError, PIXELSHADER = 1, VERTEXSHADER };
	class TextureShader : EngineComponent{
	public:
		DLLEXPORT TextureShader();
		DLLEXPORT ~TextureShader();
		DLLEXPORT bool Init(ID3D11Device* device);
		DLLEXPORT void Release();
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture);

	private:
		bool InitShader(ID3D11Device* dev, const wstring &vsfilename, const wstring &psfilename);
		void ReleaseShader();
		void PrintShaderError(ID3D10Blob* datadumb);

		bool SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture);
		void ShaderRender(ID3D11DeviceContext* devcont, int indexcount);

		ID3D11VertexShader* VertexShader;
		ID3D11PixelShader* PixelShader;
		ID3D11InputLayout* Layout;
		ID3D11Buffer* MatrixBuffer;
		ID3D11SamplerState* SamplerState;


	};
}
#endif