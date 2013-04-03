#ifndef LEVIATHAN_FONTSHADER
#define LEVIATHAN_FONTSHADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "FileSystem.h"
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
#ifndef LEVIATHAN_WINDOW
#include "Window.h"
#endif
#include <d3dx11async.h>


namespace Leviathan{

	class FontShader : EngineComponent{
	public:
		DLLEXPORT FontShader();
		DLLEXPORT ~FontShader();
		DLLEXPORT bool Init(ID3D11Device* device, Window* wind);
		DLLEXPORT void Release();
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture, Float4 textcolor);

	private:
		
		struct MatrixBufferType
		{
			D3DXMATRIX world;
			D3DXMATRIX view;
			D3DXMATRIX projection;
		};
		struct PixelBufferType
		{
			D3DXVECTOR4 pixelColor;
		};
	
		// --------------------- //
		bool InitShader(ID3D11Device* dev, Window* wind, wstring vsfilename, wstring psfilename);
		void ReleaseShader();
		void PrintShaderError(ID3D10Blob* datadumb);

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