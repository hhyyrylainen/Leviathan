#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SHADERMANAGER
#include "ShaderManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
// --------- ShaderDesc --------- //

ShaderDesc::ShaderDesc(){
	File = L"";
	Type = ShaderError;
}

ShaderDesc::ShaderDesc(ShaderType type, wstring file, int shaderid){
	File = file;
	Type = type;
	ID = shaderid;
}

// ------------------------------------ //
ShaderManager::ShaderManager(){

	_TextureShader = NULL;
	_LightShader = NULL;
	_BumpMapShader = NULL;
	_MultTextureShader = NULL;
	_GradientShader = NULL;
	_SkinnedShaderSmall = NULL;
}

ShaderManager::~ShaderManager(){

}
// ------------------------------------ //
bool ShaderManager::Init(ID3D11Device* device, Window* wind){

	// create texture shader
	_TextureShader = new TextureShader;
	if(!_TextureShader){

		return false;
	}

	// init texture shader
	if(!_TextureShader->Init(device, wind)){
		Logger::Get()->Error(L"ShaderManager: Init: could not init texture shader");
		return false;
	}
	// create multitexture shader //
	
	_MultTextureShader = new MultiTextureShader;
	if(!_MultTextureShader){
		return false;
	}
	// init it
	if(!_MultTextureShader->Init(device, wind)){
		Logger::Get()->Error(L"ShaderManager: Init: could not init multitexture shader");
		return false;
	}

	// create light shader
	_LightShader = new LightShader;
	if(!_LightShader){
		return false;
	}

	// init light shader
	if(!_LightShader->Init(device, wind)){
		Logger::Get()->Error(L"ShaderManager: Init: could not init light shader");
		return false;
	}

	// create bump map shader
	_BumpMapShader = new BumpMapShader;
	if(!_BumpMapShader){
		return false;
	}

	// init bumpmap shader
	if(!_BumpMapShader->Init(device, wind)){
		Logger::Get()->Error(L"ShaderManager: Init: could not init bump map shader");
		return false;
	}

	// create and init Gradientshader //
	_GradientShader = new GradientShader();
	if(!_GradientShader){

		Logger::Get()->Error(L"ShaderManager: 008");
		return false;
	}
	if(!_GradientShader->Init(device, wind)){

		Logger::Get()->Error(L"ShaderManager: Init: could not init GradientShader");
		return false;
	}

	// SkinnedShaderSmall shader //
	_SkinnedShaderSmall = new SkinnedShaderSmall();
	if(!_SkinnedShaderSmall){

		QUICK_MEMORY_ERROR_MESSAGE;
		return false;
	}
	if(!_SkinnedShaderSmall->Init(device)){

		Logger::Get()->Error(L"ShaderManager: Init: could not init SkinnedShaderSmall");
		return false;
	}

	return true;
}


void ShaderManager::Release()
{
	// release objects
	SAFE_RELEASEDEL(_BumpMapShader);
	SAFE_RELEASEDEL(_LightShader);
	SAFE_RELEASEDEL(_TextureShader);
	SAFE_RELEASEDEL(_MultTextureShader);
	SAFE_RELEASEDEL(_GradientShader);
	SAFE_RELEASEDEL(_SkinnedShaderSmall);
	
}
// ------------------------------------ //
bool ShaderManager::RenderMultiTextureShader(ID3D11DeviceContext* device, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView** texture){

	// Render the model using the texture shader.
	if(!_MultTextureShader->Render(device, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture)){

		return false;
	}

	return true;
}

bool ShaderManager::RenderTextureShader(ID3D11DeviceContext* device, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture){

	// Render the model using the texture shader.
	if(!_TextureShader->Render(device, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture)){

		return false;
	}

	return true;
}


bool ShaderManager::RenderLightShader(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, 
										   ID3D11ShaderResourceView* texture, Float3 lightDirection, Float4 ambient, Float4 diffuse, 
										   Float3 cameraPosition, Float4 specular, float specularPower)
{

	// Render the model using the light shader.
	bool result = _LightShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, ambient, diffuse, cameraPosition, 
								   specular, specularPower);
	if(!result){

		return false;
	}

	return true;
}


bool ShaderManager::RenderBumpMapShader(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, 
											 ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalTexture, Float3 lightDirection, 
											 Float4 diffuse)
{

	// Render the model using the bump map shader.
	if(!_BumpMapShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, colorTexture, normalTexture, lightDirection, diffuse)){

		return false;
	}

	return true;
}
DLLEXPORT bool Leviathan::ShaderManager::RenderSkinnedShaderSmall(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, 
	D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, GameObject::SkeletonRig* Bones, ID3D11ShaderResourceView* texture, Float3 lightDirection, 
	Float4 ambientColor, Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower)
{
	// Render with Skinned shader small version
	if(!_SkinnedShaderSmall->Render(devcont, indexcount, worldmatrix, viewmatrix, projectionmatrix, Bones, texture, lightDirection, ambientColor, diffuseColor, 
		cameraPosition, specularColor, specularPower))
	{
		// rendering failed //
		return false;
	}

	return true;
}

// ------------------------------------ //

DLLEXPORT void Leviathan::ShaderManager::PrintShaderError(const wstring &shader, ID3D10Blob* datadump){


	// get errors from dump //
	char* Errors = (char*)(datadump->GetBufferPointer());
	unsigned long Errossize = datadump->GetBufferSize();


	// copy message to wstring //
	string tempstr(Errors, Errossize);

	Logger::Get()->Error(L"[SHADER] "+Convert::StringToWstring(tempstr), false);
	// release error data //
	SAFE_RELEASE(datadump);
}

ID3D11Buffer* Leviathan::ShaderManager::NULLBufferBlob[20] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL};

bool ShaderManager::RenderGradientShader(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, Float4& colorstart, Float4& colorend){

	if(!_GradientShader->Render(devcont, indexcount, worldmatrix, viewmatrix, projectionmatrix, colorstart, colorend)){

		return false;
	}

	return true;
}


DLLEXPORT  UINT Leviathan::ShaderManager::GetShaderCompileFlags(){
	UINT CompileFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#ifdef SHADER_COMPILE_PREFERFLOW
	CompileFlags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
#endif // SHADER_COMPILE_PREFERFLOW
#ifdef SHADER_COMPILE_SKIP_OPTIMIZE
	CompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // SHADER_COMPILE_SKIP_OPTIMIZE
#ifdef SHADER_COMPILE_DEBUG
	CompileFlags |= D3DCOMPILE_DEBUG;
	//CompileFlags |= D3DCOMPILE_
#endif // SHADER_COMPILE_DEBUG
	return CompileFlags;
}

// ------------------------------------ //

