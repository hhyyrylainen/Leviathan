#ifndef LEVIATHAN_BUMBMAPSHADER
#define LEVIATHAN_BUMBMAPSHADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "FileSystem.h"
#include "Window.h"
#include <d3dx10math.h>
#include <d3dx11async.h>

#include "ShaderDataTypes.h"
namespace Leviathan{

	class BumpMapShader : public EngineComponent{

	public:
		DLLEXPORT BumpMapShader::BumpMapShader();
		DLLEXPORT BumpMapShader::~BumpMapShader();
		DLLEXPORT bool Init(ID3D11Device* device);
		DLLEXPORT void Release();
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont, int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* bumpmap, 
			Float3 lightdirection, Float4 diffusecolor);

	private:
		// -------------------------- //
		bool InitShader(ID3D11Device* dev, const wstring &vsfilename, const wstring &psfilename);
		void ReleaseShader();
		void PrintShaderError(ID3D10Blob* datadumb);
	
		bool SetShaderParams(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, Float3 lightdirection, Float4 diffusecolor);
		void ShaderRender(ID3D11DeviceContext* devcont, int indexcount);
		// --------------------------- //
		bool Inited;

		ID3D11VertexShader* VertexShader;
		ID3D11PixelShader* PixelShader;
		ID3D11InputLayout* Layout;
		ID3D11Buffer* MatrixBuffer;
		ID3D11SamplerState* SampleState;
		ID3D11Buffer* LightBuffer;
	};

}
#endif
