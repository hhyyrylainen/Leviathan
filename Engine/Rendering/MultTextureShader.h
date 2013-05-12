#ifndef LEVIATHAN_MULTITEXTURESHADER
#define LEVIATHAN_MULTITEXTURESHADER
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

	class MultiTextureShader : EngineComponent{
	public:
		DLLEXPORT MultiTextureShader();
		DLLEXPORT ~MultiTextureShader();
		DLLEXPORT bool Init(ID3D11Device* device);
		DLLEXPORT void Release();
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView** texture);

	private:
		bool Inited;
		//struct MatrixBufferType
		//{
		//	D3DXMATRIX world;
		//	D3DXMATRIX view;
		//	D3DXMATRIX projection;
		//};
		bool InitShader(ID3D11Device* dev, const wstring &vsfilename, const wstring &psfilename);
		void ReleaseShader();
		void PrintShaderError(ID3D10Blob* datadumb);

		bool SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView** texture);
		void ShaderRender(ID3D11DeviceContext* devcont, int indexcount);

		ID3D11VertexShader* VertexShader;
		ID3D11PixelShader* PixelShader;
		ID3D11InputLayout* Layout;
		ID3D11Buffer* MatrixBuffer;
		ID3D11SamplerState* SamplerState;


	};
}
#endif