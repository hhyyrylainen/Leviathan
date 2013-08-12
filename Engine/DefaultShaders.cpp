#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DEFAULTSHADERSFILE
#include "DefaultShaders.h"
#endif
#include "../SkeletonRig.h"
#include ".\Rendering\ShaderManager.h"
using namespace Leviathan;
using namespace Leviathan::Rendering;
// ------------------------------------ //

// ------------------ LightShader ------------------ //
DLLEXPORT Leviathan::Rendering::LightShader::LightShader() : BaseShader(L"LightShader.hlsl", "LightVertexShader", "LightPixelShader", 
	"BUF:BMAT:CAMB:BLIGHT:TEX:NORMAL"), MatrixBuffer(NULL), CameraBuffer(NULL), LightBuffer(NULL)
{

}

DLLEXPORT Leviathan::Rendering::LightShader::~LightShader(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::LightShader::DoesInputObjectWork(ShaderRenderTask* paramstocheck) const{
	// check for data presence of required objects //
	BaseMatrixBufferData* bmtocheck = paramstocheck->GetBaseMatrixBufferData();
	BaseTextureHolder* bttocheck = paramstocheck->GetBaseTextureHolder();
	BaseLightBufferData* bltocheck = paramstocheck->GetBaseLightBufferData();
	CameraBufferData* bctocheck = paramstocheck->GetCameraBufferData();

	if(bmtocheck == NULL || bttocheck == NULL || bltocheck == NULL || bctocheck== NULL || bttocheck->TextureCount != 1 || 
		!(bttocheck->TextureFlags & TEXTURETYPE_NORMAL))
	{
		// failed a check //
		return false;
	}

	// passed all tests //
	return true;
}
// ------------------------------------ //
bool Leviathan::Rendering::LightShader::SetupShaderDataBuffers(ID3D11Device* dev){
	// create this shader specific buffers //
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&MatrixBuffer, sizeof(MatrixBufferType))){

		return false;
	}

	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&CameraBuffer, sizeof(CameraBufferType))){

		return false;
	}

	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&LightBuffer, sizeof(LightBufferType))){

		return false;
	}


	return true;
}

void Leviathan::Rendering::LightShader::ReleaseShaderDataBuffers(){
	// this shader specific buffers //
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(CameraBuffer);
	SAFE_RELEASE(LightBuffer);
}
// ------------------------------------ //
bool Leviathan::Rendering::LightShader::SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
	// copy new data from parameters object to the shader buffers //
	// prepare buffers //
	if(!SetNewDataToShaderBuffers(devcont, parameters)){

		Logger::Get()->Error(L"TextureShader: SetShaderParams: failed to update buffers");
		return false;
	}
	// all buffers should now have new data and be unlocked //


	// set this shader specific buffers active //


	// set VertexShader buffers //
	devcont->VSSetConstantBuffers(0, 1, &MatrixBuffer);
	devcont->VSSetConstantBuffers(1, 1, &CameraBuffer);
	devcont->VSSetConstantBuffers(2, 1, &LightBuffer);

	// set PixelShader resources //
	// we need this temporary because the function actually wants an array of these values //
	ID3D11ShaderResourceView* tmpview = static_cast<SingleTextureHolder*>(parameters->GetBaseTextureHolder())->Texture1->GetView();

	devcont->PSSetShaderResources(0, 1, &tmpview);

	return true;
}

bool Leviathan::Rendering::LightShader::SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
	// copy new matrix buffer data //
	auto AutoUnlocker = Rendering::ResourceCreator::MapConstantBufferForWriting<MatrixBufferType>(devcont, MatrixBuffer);
	if(AutoUnlocker == NULL){
		// lock failed //
		return false;
	}

	// create temporary matrices and transpose matrices into them //
	D3DXMATRIX worldmatrix;
	D3DXMATRIX viewmatrix;
	D3DXMATRIX projectionmatrix;

	D3DXMatrixTranspose(&worldmatrix, &parameters->GetBaseMatrixBufferData()->WorldMatrix);
	D3DXMatrixTranspose(&viewmatrix, &parameters->GetBaseMatrixBufferData()->ViewMatrix);
	D3DXMatrixTranspose(&projectionmatrix, &parameters->GetBaseMatrixBufferData()->ProjectionMatrix);

	// copy matrices //
	AutoUnlocker->LockedResourcePtr->world = worldmatrix;
	AutoUnlocker->LockedResourcePtr->world = viewmatrix;
	AutoUnlocker->LockedResourcePtr->world = projectionmatrix;
	
	// new camera buffer data //
	auto CamBufferData = Rendering::ResourceCreator::MapConstantBufferForWriting<CameraBufferType>(devcont, CameraBuffer);
	if(CamBufferData == NULL){
		// lock failed //
		return false;
	}

	CamBufferData->LockedResourcePtr->cameraPosition = parameters->GetCameraBufferData()->CameraPositionInWorld;

	// new light buffer data //
	auto LightBufferData = Rendering::ResourceCreator::MapConstantBufferForWriting<LightBufferType>(devcont, LightBuffer);
	if(LightBufferData == NULL){
		// lock failed //
		return false;
	}

	BaseLightBufferData* lbuf = parameters->GetBaseLightBufferData();

	// Copy the lighting variables into the light constant buffer.
	LightBufferData->LockedResourcePtr->ambientColor = lbuf->AmbientColor;
	LightBufferData->LockedResourcePtr->diffuseColor = lbuf->DiffuseColor;
	LightBufferData->LockedResourcePtr->lightDirection = lbuf->LightDirection;
	LightBufferData->LockedResourcePtr->specularColor = lbuf->SpecularColor;
	LightBufferData->LockedResourcePtr->specularPower = lbuf->SpecularPower;


	return true;
}
// ------------------------------------ //
bool Leviathan::Rendering::LightShader::SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer){
	// create layout //
	D3D11_INPUT_ELEMENT_DESC shaderdatalayoutdesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// calculate element count //
	UINT elementcount = sizeof(shaderdatalayoutdesc)/sizeof(shaderdatalayoutdesc[0]);

	if(!CreateInputLayout(dev, VertexShaderBuffer, &shaderdatalayoutdesc[0], elementcount)){

		return false;
	}

	return true;
}
// ------------------ GradientShader ------------------ //
DLLEXPORT Leviathan::Rendering::GradientShader::GradientShader() : BaseShader(L"GradientShader.hlsl", "GradientVertexShader", "GradientPixelShader", 
	"BUF:BMAT:COL2:INPUT:C0:T0"), MatrixBuffer(NULL), ColorsBuffer(NULL)
{

}

DLLEXPORT Leviathan::Rendering::GradientShader::~GradientShader(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::GradientShader::DoesInputObjectWork(ShaderRenderTask* paramstocheck) const{
	// check for data presence of required objects //
	BaseMatrixBufferData* bmtocheck = paramstocheck->GetBaseMatrixBufferData();
	TwoColorBufferData* bctocheck = paramstocheck->GetColourBufferTwo();

	if(bmtocheck == NULL || bctocheck== NULL){
		// failed a check //
		return false;
	}

	// passed all tests //
	return true;
}
// ------------------------------------ //
bool Leviathan::Rendering::GradientShader::SetupShaderDataBuffers(ID3D11Device* dev){
	// create this shader specific buffers //
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&MatrixBuffer, sizeof(MatrixBufferType))){

		return false;
	}

	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&ColorsBuffer, sizeof(ColorBufferTwoType))){

		return false;
	}


	return true;
}

void Leviathan::Rendering::GradientShader::ReleaseShaderDataBuffers(){
	// this shader specific buffers //
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(ColorsBuffer);
}
// ------------------------------------ //
bool Leviathan::Rendering::GradientShader::SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
	// copy new data from parameters object to the shader buffers //
	// prepare buffers //
	if(!SetNewDataToShaderBuffers(devcont, parameters)){

		Logger::Get()->Error(L"TextureShader: SetShaderParams: failed to update buffers");
		return false;
	}
	// all buffers should now have new data and be unlocked //


	// set this shader specific buffers active //


	// set VertexShader buffers //
	devcont->VSSetConstantBuffers(0, 1, &MatrixBuffer);
	// !PixelShader uses this buffer //
	devcont->PSSetConstantBuffers(0, 1, &ColorsBuffer);

	return true;
}

bool Leviathan::Rendering::GradientShader::SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
	// copy new matrix buffer data //
	auto AutoUnlocker = Rendering::ResourceCreator::MapConstantBufferForWriting<MatrixBufferType>(devcont, MatrixBuffer);
	if(AutoUnlocker == NULL){
		// lock failed //
		return false;
	}

	// create temporary matrices and transpose matrices into them //
	D3DXMATRIX worldmatrix;
	D3DXMATRIX viewmatrix;
	D3DXMATRIX projectionmatrix;

	D3DXMatrixTranspose(&worldmatrix, &parameters->GetBaseMatrixBufferData()->WorldMatrix);
	D3DXMatrixTranspose(&viewmatrix, &parameters->GetBaseMatrixBufferData()->ViewMatrix);
	D3DXMatrixTranspose(&projectionmatrix, &parameters->GetBaseMatrixBufferData()->ProjectionMatrix);

	// copy matrices //
	AutoUnlocker->LockedResourcePtr->world = worldmatrix;
	AutoUnlocker->LockedResourcePtr->world = viewmatrix;
	AutoUnlocker->LockedResourcePtr->world = projectionmatrix;

	// new colour buffer data //
	auto ColourBufferData = Rendering::ResourceCreator::MapConstantBufferForWriting<ColorBufferTwoType>(devcont, ColorsBuffer);
	if(ColourBufferData == NULL){
		// lock failed //
		return false;
	}

	ColourBufferData->LockedResourcePtr->ColorStart = parameters->GetColourBufferTwo()->Colour1;
	ColourBufferData->LockedResourcePtr->ColorEnd = parameters->GetColourBufferTwo()->Colour2;


	return true;
}
// ------------------------------------ //
bool Leviathan::Rendering::GradientShader::SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer){
	// create layout //
	D3D11_INPUT_ELEMENT_DESC shaderdatalayoutdesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// calculate element count //
	UINT elementcount = sizeof(shaderdatalayoutdesc)/sizeof(shaderdatalayoutdesc[0]);

	if(!CreateInputLayout(dev, VertexShaderBuffer, &shaderdatalayoutdesc[0], elementcount)){

		return false;
	}

	return true;
}
// ------------------ LightBumpShader ------------------ //
DLLEXPORT Leviathan::Rendering::LightBumpShader::LightBumpShader() : BaseShader(L"BumpMapShader.hlsl", "BumpMapVertexShader", "BumpMapPixelShader", 
	"BUF:BMAT:CAMB:BLIGHT:TEX:NORMAL:BUMP:INPUT:C0:T0:N0:TANG0:BINOR0"), MatrixBuffer(NULL), CameraBuffer(NULL), LightBuffer(NULL)
{

}

DLLEXPORT Leviathan::Rendering::LightBumpShader::~LightBumpShader(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::LightBumpShader::DoesInputObjectWork(ShaderRenderTask* paramstocheck) const{
	// check for data presence of required objects //
	BaseMatrixBufferData* bmtocheck = paramstocheck->GetBaseMatrixBufferData();
	BaseTextureHolder* bttocheck = paramstocheck->GetBaseTextureHolder();
	BaseLightBufferData* bltocheck = paramstocheck->GetBaseLightBufferData();
	CameraBufferData* bctocheck = paramstocheck->GetCameraBufferData();

	if(bmtocheck == NULL || bttocheck == NULL || bltocheck == NULL || bctocheck== NULL || bttocheck->TextureCount != 2 || 
		!(bttocheck->TextureFlags & TEXTURETYPE_NORMAL) || !(bttocheck->TextureFlags & TEXTURETYPE_BUMPMAP) )
	{
		// failed a check //
		return false;
	}

	// passed all tests //
	return true;
}
// ------------------------------------ //
bool Leviathan::Rendering::LightBumpShader::SetupShaderDataBuffers(ID3D11Device* dev){
	// create this shader specific buffers //
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&MatrixBuffer, sizeof(MatrixBufferType))){

		return false;
	}

	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&CameraBuffer, sizeof(CameraBufferType))){

		return false;
	}

	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&LightBuffer, sizeof(LightBufferType))){

		return false;
	}


	return true;
}

void Leviathan::Rendering::LightBumpShader::ReleaseShaderDataBuffers(){
	// this shader specific buffers //
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(CameraBuffer);
	SAFE_RELEASE(LightBuffer);
}
// ------------------------------------ //
bool Leviathan::Rendering::LightBumpShader::SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
	// copy new data from parameters object to the shader buffers //
	// prepare buffers //
	if(!SetNewDataToShaderBuffers(devcont, parameters)){

		Logger::Get()->Error(L"TextureShader: SetShaderParams: failed to update buffers");
		return false;
	}
	// all buffers should now have new data and be unlocked //


	// set this shader specific buffers active //


	// set VertexShader buffers //
	devcont->VSSetConstantBuffers(0, 1, &MatrixBuffer);
	devcont->VSSetConstantBuffers(1, 1, &CameraBuffer);
	devcont->VSSetConstantBuffers(2, 1, &LightBuffer);

	// set PixelShader resources //
	// we need this temporary because the texture holder doesn't combine the textures //
	ID3D11ShaderResourceView* TmpTexArray[2] = {NULL, NULL};
	// the textures need to be in right order //
	DoubleTextureHolder* tmphold = static_cast<DoubleTextureHolder*>(parameters->GetBaseTextureHolder());

	if(tmphold->Texture1->GetType() == TEXTURETYPE_BUMPMAP){
		// Bump map first and then normal //
		TmpTexArray[0] = tmphold->Texture2->GetView();
		TmpTexArray[1] = tmphold->Texture1->GetView();

	} else {
		// other way around //
		TmpTexArray[0] = tmphold->Texture1->GetView();
		TmpTexArray[1] = tmphold->Texture2->GetView();
	}


	devcont->PSSetShaderResources(0, 2, &TmpTexArray[0]);

	return true;
}

bool Leviathan::Rendering::LightBumpShader::SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
	// copy new matrix buffer data //
	auto AutoUnlocker = Rendering::ResourceCreator::MapConstantBufferForWriting<MatrixBufferType>(devcont, MatrixBuffer);
	if(AutoUnlocker == NULL){
		// lock failed //
		return false;
	}

	// create temporary matrices and transpose matrices into them //
	D3DXMATRIX worldmatrix;
	D3DXMATRIX viewmatrix;
	D3DXMATRIX projectionmatrix;

	D3DXMatrixTranspose(&worldmatrix, &parameters->GetBaseMatrixBufferData()->WorldMatrix);
	D3DXMatrixTranspose(&viewmatrix, &parameters->GetBaseMatrixBufferData()->ViewMatrix);
	D3DXMatrixTranspose(&projectionmatrix, &parameters->GetBaseMatrixBufferData()->ProjectionMatrix);

	// copy matrices //
	AutoUnlocker->LockedResourcePtr->world = worldmatrix;
	AutoUnlocker->LockedResourcePtr->world = viewmatrix;
	AutoUnlocker->LockedResourcePtr->world = projectionmatrix;

	// new camera buffer data //
	auto CamBufferData = Rendering::ResourceCreator::MapConstantBufferForWriting<CameraBufferType>(devcont, CameraBuffer);
	if(CamBufferData == NULL){
		// lock failed //
		return false;
	}

	CamBufferData->LockedResourcePtr->cameraPosition = parameters->GetCameraBufferData()->CameraPositionInWorld;

	// new light buffer data //
	auto LightBufferData = Rendering::ResourceCreator::MapConstantBufferForWriting<LightBufferType>(devcont, LightBuffer);
	if(LightBufferData == NULL){
		// lock failed //
		return false;
	}

	BaseLightBufferData* lbuf = parameters->GetBaseLightBufferData();

	// Copy the lighting variables into the light constant buffer.
	LightBufferData->LockedResourcePtr->ambientColor = lbuf->AmbientColor;
	LightBufferData->LockedResourcePtr->diffuseColor = lbuf->DiffuseColor;
	LightBufferData->LockedResourcePtr->lightDirection = lbuf->LightDirection;
	LightBufferData->LockedResourcePtr->specularColor = lbuf->SpecularColor;
	LightBufferData->LockedResourcePtr->specularPower = lbuf->SpecularPower;


	return true;
}
// ------------------------------------ //
bool Leviathan::Rendering::LightBumpShader::SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer){
	// create layout //
	D3D11_INPUT_ELEMENT_DESC shaderdatalayoutdesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// calculate element count //
	UINT elementcount = sizeof(shaderdatalayoutdesc)/sizeof(shaderdatalayoutdesc[0]);

	if(!CreateInputLayout(dev, VertexShaderBuffer, &shaderdatalayoutdesc[0], elementcount)){

		return false;
	}

	return true;
}
// ------------------ SkinnedShader ------------------ //
DLLEXPORT Leviathan::Rendering::SkinnedShader::SkinnedShader() : BaseShader(L"SkinnedModelShader.hlsl", "SkinnedVertexShader", "SkinnedPixelShader", 
	"BUF:BMAT:CAMB:BLIGHT:BSKIN:TEX:NORMAL"), MatrixBuffer(NULL), CameraBuffer(NULL), LightBuffer(NULL), BoneMatriceBuffer_tiny(NULL),
	VertexShader_small(NULL), BoneMatriceBuffer_small(NULL), VertexShader_medium(NULL), BoneMatriceBuffer_medium(NULL), VertexShader_large(NULL),
	BoneMatriceBuffer_large(NULL), VertexShader_huge(NULL), BoneMatriceBuffer_huge(NULL), VertexShader_max(NULL), BoneMatriceBuffer_max(NULL)
{


}

DLLEXPORT Leviathan::Rendering::SkinnedShader::~SkinnedShader(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::SkinnedShader::DoesInputObjectWork(ShaderRenderTask* paramstocheck) const{
	// check for data presence of required objects //
	BaseMatrixBufferData* bmtocheck = paramstocheck->GetBaseMatrixBufferData();
	BaseTextureHolder* bttocheck = paramstocheck->GetBaseTextureHolder();
	BaseLightBufferData* bltocheck = paramstocheck->GetBaseLightBufferData();
	CameraBufferData* bctocheck = paramstocheck->GetCameraBufferData();
	BaseSkinningData* bstocheck = paramstocheck->GetBaseSkinningData();

	if(bmtocheck == NULL || bttocheck == NULL || bltocheck == NULL || bctocheck== NULL || bstocheck == NULL || bttocheck->TextureCount != 1 || 
		!(bttocheck->TextureFlags & TEXTURETYPE_NORMAL))
	{
		// failed a check //
		return false;
	}

	// passed all tests //
	return true;
}
// ------------------------------------ //
bool Leviathan::Rendering::SkinnedShader::SetupShaderDataBuffers(ID3D11Device* dev){
	// create this shader specific buffers //
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&MatrixBuffer, sizeof(MatrixBufferType))){

		return false;
	}

	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&CameraBuffer, sizeof(CameraBufferType))){

		return false;
	}

	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&LightBuffer, sizeof(LightBufferType))){

		return false;
	}

	
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&BoneMatriceBuffer_tiny, sizeof(BoneTransformsBufferTypeTiny))){

		return false;
	}
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&BoneMatriceBuffer_small, sizeof(BoneTransformsBufferTypeSmall))){

		return false;
	}
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&BoneMatriceBuffer_medium, sizeof(BoneTransformsBufferTypeMedium))){

		return false;
	}
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&BoneMatriceBuffer_large, sizeof(BoneTransformsBufferTypeLarge))){

		return false;
	}
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&BoneMatriceBuffer_huge, sizeof(BoneTransformsBufferTypeHuge))){

		return false;
	}
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&BoneMatriceBuffer_max, sizeof(BoneTransformsBufferTypeMax))){

		return false;
	}



	return true;
}

void Leviathan::Rendering::SkinnedShader::ReleaseShaderDataBuffers(){
	// this shader specific buffers //
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(CameraBuffer);
	SAFE_RELEASE(LightBuffer);
	SAFE_RELEASE(VertexShader_small);
	SAFE_RELEASE(VertexShader_medium);
	SAFE_RELEASE(VertexShader_large);
	SAFE_RELEASE(VertexShader_huge);
	SAFE_RELEASE(VertexShader_max);
	SAFE_RELEASE(BoneMatriceBuffer_tiny);
	SAFE_RELEASE(BoneMatriceBuffer_small);
	SAFE_RELEASE(BoneMatriceBuffer_medium);
	SAFE_RELEASE(BoneMatriceBuffer_large);
	SAFE_RELEASE(BoneMatriceBuffer_huge);
	SAFE_RELEASE(BoneMatriceBuffer_max);
}
// ------------------------------------ //
bool Leviathan::Rendering::SkinnedShader::SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
	// we need to figure out wanted bone size //
	const int &bonecount = parameters->GetBaseSkinningData()->Bones->GetBoneCount();
	if(bonecount <= MAX_BONES_TINY){
		parameters->GetBaseSkinningData()->ShaderInternalDataPass = SHADER_BONES_TINY;
	} else if(bonecount <= MAX_BONES_SMALL){
		parameters->GetBaseSkinningData()->ShaderInternalDataPass = SHADER_BONES_SMALL;
	} else if(bonecount <= MAX_BONES_MEDIUM){
		parameters->GetBaseSkinningData()->ShaderInternalDataPass = SHADER_BONES_MEDIUM;
	} else if(bonecount <= MAX_BONES_LARGE){
		parameters->GetBaseSkinningData()->ShaderInternalDataPass = SHADER_BONES_LARGE;
	} else if(bonecount <= MAX_BONES_HUGE){
		parameters->GetBaseSkinningData()->ShaderInternalDataPass = SHADER_BONES_HUGE;
	} else if(bonecount <= MAX_BONES_MAX){
		parameters->GetBaseSkinningData()->ShaderInternalDataPass = SHADER_BONES_MAX;
	} else {
		// error //
		DEBUG_BREAK;
	}
	// copy new data from parameters object to the shader buffers //
	// prepare buffers //
	if(!SetNewDataToShaderBuffers(devcont, parameters)){

		Logger::Get()->Error(L"TextureShader: SetShaderParams: failed to update buffers");
		return false;
	}
	// all buffers should now have new data and be unlocked //


	// set this shader specific buffers active //


	// set VertexShader buffers //
	devcont->VSSetConstantBuffers(0, 1, &MatrixBuffer);
	devcont->VSSetConstantBuffers(1, 1, &CameraBuffer);
	devcont->VSSetConstantBuffers(2, 1, &LightBuffer);

	// set PixelShader resources //
	// we need this temporary because the function actually wants an array of these values //
	ID3D11ShaderResourceView* tmpview = static_cast<SingleTextureHolder*>(parameters->GetBaseTextureHolder())->Texture1->GetView();

	devcont->PSSetShaderResources(0, 1, &tmpview);

	return true;
}

bool Leviathan::Rendering::SkinnedShader::SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
	// copy new matrix buffer data //
	auto AutoUnlocker = Rendering::ResourceCreator::MapConstantBufferForWriting<MatrixBufferType>(devcont, MatrixBuffer);
	if(AutoUnlocker == NULL){
		// lock failed //
		return false;
	}

	// create temporary matrices and transpose matrices into them //
	D3DXMATRIX worldmatrix;
	D3DXMATRIX viewmatrix;
	D3DXMATRIX projectionmatrix;

	D3DXMatrixTranspose(&worldmatrix, &parameters->GetBaseMatrixBufferData()->WorldMatrix);
	D3DXMatrixTranspose(&viewmatrix, &parameters->GetBaseMatrixBufferData()->ViewMatrix);
	D3DXMatrixTranspose(&projectionmatrix, &parameters->GetBaseMatrixBufferData()->ProjectionMatrix);

	// copy matrices //
	AutoUnlocker->LockedResourcePtr->world = worldmatrix;
	AutoUnlocker->LockedResourcePtr->world = viewmatrix;
	AutoUnlocker->LockedResourcePtr->world = projectionmatrix;

	// new camera buffer data //
	auto CamBufferData = Rendering::ResourceCreator::MapConstantBufferForWriting<CameraBufferType>(devcont, CameraBuffer);
	if(CamBufferData == NULL){
		// lock failed //
		return false;
	}

	CamBufferData->LockedResourcePtr->cameraPosition = parameters->GetCameraBufferData()->CameraPositionInWorld;

	// new light buffer data //
	auto LightBufferData = Rendering::ResourceCreator::MapConstantBufferForWriting<LightBufferType>(devcont, LightBuffer);
	if(LightBufferData == NULL){
		// lock failed //
		return false;
	}

	BaseLightBufferData* lbuf = parameters->GetBaseLightBufferData();

	// Copy the lighting variables into the light constant buffer.
	LightBufferData->LockedResourcePtr->ambientColor = lbuf->AmbientColor;
	LightBufferData->LockedResourcePtr->diffuseColor = lbuf->DiffuseColor;
	LightBufferData->LockedResourcePtr->lightDirection = lbuf->LightDirection;
	LightBufferData->LockedResourcePtr->specularColor = lbuf->SpecularColor;
	LightBufferData->LockedResourcePtr->specularPower = lbuf->SpecularPower;



	// Data to Bone transform matrix buffer //
	GameObject::SkeletonRig* Bones = parameters->GetBaseSkinningData()->Bones;
	switch(parameters->GetBaseSkinningData()->ShaderInternalDataPass){
	case SHADER_BONES_TINY:
		if(!(WriteMatricesToBuffer<BoneTransformsPointerTiny, BoneTransformsBufferTypeTiny>(&BoneMatriceBuffer_tiny, devcont, Bones))){
			return false;
		}
		break;
	case SHADER_BONES_SMALL:
		if(!(WriteMatricesToBuffer<BoneTransformsPointerSmall, BoneTransformsBufferTypeSmall>(&BoneMatriceBuffer_small, devcont, Bones))){
			return false;
		}
		break;
	case SHADER_BONES_MEDIUM:
		if(!(WriteMatricesToBuffer<BoneTransformsPointerMedium, BoneTransformsBufferTypeMedium>(&BoneMatriceBuffer_medium, devcont, Bones))){
			return false;
		}
		break;
	case SHADER_BONES_LARGE:
		if(!(WriteMatricesToBuffer<BoneTransformsPointerLarge, BoneTransformsBufferTypeLarge>(&BoneMatriceBuffer_large, devcont, Bones))){
			return false;
		}
		break;
	case SHADER_BONES_HUGE:
		if(!(WriteMatricesToBuffer<BoneTransformsPointerHuge, BoneTransformsBufferTypeHuge>(&BoneMatriceBuffer_huge, devcont, Bones))){
			return false;
		}
		break;
	case SHADER_BONES_MAX:
		if(!(WriteMatricesToBuffer<BoneTransformsPointerMax, BoneTransformsBufferTypeMax>(&BoneMatriceBuffer_max, devcont, Bones))){
			return false;
		}
		break;
	}




	return true;
}
// ------------------------------------ //
bool Leviathan::Rendering::SkinnedShader::SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer){
	// create layout //
	D3D11_INPUT_ELEMENT_DESC shaderdatalayoutdesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// calculate element count //
	UINT elementcount = sizeof(shaderdatalayoutdesc)/sizeof(shaderdatalayoutdesc[0]);

	if(!CreateInputLayout(dev, VertexShaderBuffer, &shaderdatalayoutdesc[0], elementcount)){

		return false;
	}

	return true;
}

bool Leviathan::Rendering::SkinnedShader::LoadShaderFromDisk(ID3D11Device* dev){
	// set up compile parameters //
	UINT CompileFlags = ShaderManager::GetShaderCompileFlags();

	// init objects to null //
	HRESULT hr = S_OK;

	int max_transforms;

	// compile shaders //
	for(int i = 0; i < 7; i++){

		switch(i){
			case 0: max_transforms = MAX_BONES_TINY; break;
			case 1: break;
			case 2: max_transforms = MAX_BONES_SMALL; break;
			case 3: max_transforms = MAX_BONES_MEDIUM; break;
			case 4: max_transforms = MAX_BONES_LARGE; break;
			case 5: max_transforms = MAX_BONES_HUGE; break;
			case 6: max_transforms = MAX_BONES_MAX; break;
			default:
				// can't reach this //
				__assume(0);
		}

		D3D10_SHADER_MACRO Shader_Macros[2] = {{"MAX_TRANSFORMS", Convert::ToString(max_transforms).c_str()}, {NULL, NULL}};

		ID3D10Blob* Errordumb = NULL;
		ID3D10Blob* CurrentShaderBuffer = NULL;

		if(i != 1){
			// compile vertex shader //
			hr = D3DX11CompileFromFile(ShaderFileName.c_str(), &Shader_Macros[0], NULL, VSShaderEntryPoint.c_str(), "vs_5_0", CompileFlags, 0, 
				NULL, &CurrentShaderBuffer, &Errordumb, NULL);


		} else {
			// pixel shader compile //
			hr = D3DX11CompileFromFile(ShaderFileName.c_str(), &Shader_Macros[0], NULL, PSShaderEntryPoint.c_str(), "ps_5_0", CompileFlags, 0, NULL, 
				&CurrentShaderBuffer, &Errordumb, NULL);
		}
		// try to report something useful if the creation failed //
		if(FAILED(hr)){
			// check for compile error //
			if(Errordumb){
				// compile error //
				ShaderManager::PrintShaderError(ShaderFileName, Errordumb);
				return false;
			}
			// file was not found //
			Logger::Get()->Error(L"InitShader: can't find file: "+ShaderFileName);
			return false;
		}

		if(i == 0){
			// buffers are needed for input layouts //
			if(!SetupShaderInputLayouts(dev, CurrentShaderBuffer)){
				// release shader buffers
				SAFE_RELEASE(CurrentShaderBuffer);
				Logger::Get()->Error(L"InitShader: cannot create input layout");
				return false;
			}
		}
		// create shader //
		if(i == 1){

			hr = dev->CreatePixelShader(CurrentShaderBuffer->GetBufferPointer(), CurrentShaderBuffer->GetBufferSize(), NULL, &PixelShader);
			if(FAILED(hr)){
				SAFE_RELEASE(CurrentShaderBuffer);
				Logger::Get()->Error(L"InitShader: failed to create PixelShader from buffer", hr);
				return false;
			}

		} else {
			// create shaders from buffers
			ID3D11VertexShader** vsptr = NULL;

			switch(i){
			case 0: vsptr = &VertexShader; break;
			case 2: vsptr = &VertexShader_small; break;
			case 3: vsptr = &VertexShader_medium; break;
			case 4: vsptr = &VertexShader_large; break;
			case 5: vsptr = &VertexShader_huge; break;
			case 6: vsptr = &VertexShader_max; break;
			default:
				// can't reach this //
				__assume(0);
			}

			hr = dev->CreateVertexShader(CurrentShaderBuffer->GetBufferPointer(), CurrentShaderBuffer->GetBufferSize(), NULL, vsptr);
			if(FAILED(hr)){
				SAFE_RELEASE(CurrentShaderBuffer);
				Logger::Get()->Error(L"InitShader: failed to create VertexShader from buffer", hr);
				return false;
			}
		}


		// release shader buffer //
		SAFE_RELEASE(CurrentShaderBuffer);
	}

	// succeeded //
	return true;
}

DLLEXPORT bool Leviathan::Rendering::SkinnedShader::Render(ID3D11DeviceContext* devcont,int indexcount, ShaderRenderTask* Parameters){
	// call set parameters which must be defined by each child class //
	if(!SetShaderParams(devcont, Parameters)){

		Logger::Get()->Error(L"BaseShader: Render: failed to set shader parameters");
		return false;
	}
	// get right shader and render //
	ID3D11VertexShader* UseShader = NULL;

	// switch to see which layout and which shader needs to be used //
	switch(Parameters->GetBaseSkinningData()->ShaderInternalDataPass){
	case SHADER_BONES_TINY: UseShader = VertexShader; break;
	case SHADER_BONES_SMALL: UseShader = VertexShader_small; break;
	case SHADER_BONES_MEDIUM: UseShader = VertexShader_medium; break;
	case SHADER_BONES_LARGE: UseShader = VertexShader_large; break;
	case SHADER_BONES_HUGE: UseShader = VertexShader_huge; break;
	case SHADER_BONES_MAX: UseShader = VertexShader_max; break;
	}
	if(!UseShader){
		ComplainOnce::PrintErrorOnce(L"SkinnedShader_bonecount_not_supported", L"SkinnedShader: Render: invalid bone count enum value :"+
			Convert::IntToWstring((int)Parameters->GetBaseSkinningData()->ShaderInternalDataPass));
		return false;
	}


	ShaderRender(devcont, UseShader, PixelShader, indexcount);
	return true;
}
