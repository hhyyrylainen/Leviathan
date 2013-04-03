#ifndef LEVIATHAN_COLORGRADIENTSHADER
#define LEVIATHAN_COLORGRADIENTSHADER
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

	class GradientShader : EngineComponent{
	public:
		DLLEXPORT GradientShader();
		DLLEXPORT ~GradientShader();
		DLLEXPORT bool Init(ID3D11Device* device, Window* wind);
		DLLEXPORT void Release();
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, Float4 colorstart, Float4 colorend);

	private:
		bool Inited;
		//struct MatrixBufferType
		//{
		//	D3DXMATRIX world;
		//	D3DXMATRIX view;
		//	D3DXMATRIX projection;
		//};

		//struct ColorBuffer
		//{
		//	D3DXVECTOR4 ColorStart;
		//	D3DXVECTOR4 ColorEnd;
		//};
		// ------------------------------- //
		bool InitShader(ID3D11Device* dev, Window* wind, wstring vsfilename, wstring psfilename);
		void ReleaseShader();
		void PrintShaderError(ID3D10Blob* datadumb);

		bool SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, Float4 colorstart, Float4 colorend);
		void ShaderRender(ID3D11DeviceContext* devcont, int indexcount);

		ID3D11VertexShader* VertexShader;
		ID3D11PixelShader* PixelShader;
		ID3D11InputLayout* Layout;
		ID3D11Buffer* MatrixBuffer;
		ID3D11SamplerState* SamplerState;
		ID3D11Buffer* ColorsBuffer;

	};
}
#endif