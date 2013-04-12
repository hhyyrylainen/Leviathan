#ifndef LEVIATHAN_SHADERMANAGER
#define LEVIATHAN_SHADERMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "3DRenderer.h"
#include "MultTextureShader.h"
#include "TextureShader.h"
#include "LightShader.h"
#include "BumpShader.h"
#include "SkinnedShader.h"
#include "ColorGradientShader.h"
#include "..\SkeletonRig.h"

namespace Leviathan{

	class ShaderManager : public EngineComponent{
	public:
		DLLEXPORT ShaderManager::ShaderManager();
		DLLEXPORT ShaderManager::~ShaderManager();
		DLLEXPORT bool Init(ID3D11Device* dev, Window* wind);
		DLLEXPORT void Release();

		DLLEXPORT bool RenderMultiTextureShader(ID3D11DeviceContext* devcont, int indexcount, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, D3DXMATRIX, ID3D11ShaderResourceView** textures);
		DLLEXPORT bool RenderTextureShader(ID3D11DeviceContext* devcont, int indexcount, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, D3DXMATRIX, ID3D11ShaderResourceView* texture);
		DLLEXPORT bool RenderLightShader(ID3D11DeviceContext* devcont, int indexcount, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, D3DXMATRIX, ID3D11ShaderResourceView* texture, 
							   Float3 lightDirection, Float4 ambient, Float4 diffuse, Float3 cameraPosition, Float4 specular, float specularPower);

		DLLEXPORT bool RenderBumpMapShader(ID3D11DeviceContext* devcont, int indexcount, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, D3DXMATRIX, ID3D11ShaderResourceView* texture, 
								 ID3D11ShaderResourceView*, Float3, Float4);

		DLLEXPORT bool RenderGradientShader(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, Float4& colorstart, Float4& colorend);

		DLLEXPORT bool RenderSkinnedShader(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, 
			D3DXMATRIX projectionmatrix, GameObject::SkeletonRig* Bones, ID3D11ShaderResourceView* texture, Float3 lightDirection, Float4 ambientColor, 
			Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower);

		DLLEXPORT static void PrintShaderError(const wstring &shader, ID3D10Blob* datadump);

		DLLEXPORT static UINT GetShaderCompileFlags();

		// array of NULL pointers for clearing shader resources//
		static ID3D11Buffer* NULLBufferBlob[20];

	private:
		MultiTextureShader* _MultTextureShader;
		TextureShader* _TextureShader;
		LightShader* _LightShader;
		BumpMapShader* _BumpMapShader;
		GradientShader* _GradientShader;
		SkinnedShader* _SkinnedShader;
	};
// --------- ShaderDesc --------- //
	class ShaderDesc{
	public:
		ShaderDesc();
		ShaderDesc(ShaderType type, wstring file, int shaderid);

		int ID;
		wstring File;
		ShaderType Type;

	};
}
#endif

	

