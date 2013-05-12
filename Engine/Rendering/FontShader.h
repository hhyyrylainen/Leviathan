#ifndef LEVIATHAN_FONTSHADER
#define LEVIATHAN_FONTSHADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "FileSystem.h"
#include <d3dx11async.h>

#include "Window.h"
#include "ShaderDataTypes.h"


namespace Leviathan{

	class FontShader : EngineComponent{
	public:
		DLLEXPORT FontShader();
		DLLEXPORT ~FontShader();
		DLLEXPORT bool Init(ID3D11Device* device);
		DLLEXPORT void Release();
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture, Float4 textcolor);

	private:
		struct PixelBufferType
		{
			D3DXVECTOR4 pixelColor;
		};
	
		// --------------------- //
		bool InitShader(ID3D11Device* dev, const wstring &vsfilename, const wstring &psfilename);
		void ReleaseShader();

		bool SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture, Float4 color);
		void ShaderRender(ID3D11DeviceContext* devcont, int indexcount);

		// ----------------------- //
		bool Inited;

		ID3D11VertexShader* VertexShader;
		ID3D11PixelShader* PixelShader;
		ID3D11InputLayout* Layout;
		ID3D11Buffer* MatrixBuffer;
		ID3D11SamplerState* SamplerState;

		ID3D11Buffer* PixelColorBuffer;
	};
}
#endif