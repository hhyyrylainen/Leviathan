#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FONTSHADER
#include "FontShader.h"
#endif
using namespace Leviathan;
using namespace Rendering;
// ------------------------------------ //
#include "ShaderManager.h"

DLLEXPORT Leviathan::Rendering::GradientShader::GradientShader() : BaseShader(L"FontShader.hlsl", "FontVertexShader", "FontPixelShader", 
	"BUF:BMAT:COL2:TEX:TEXT:INPUT:C0:T0"), MatrixBuffer(NULL), ColorsBuffer(NULL)
{

}

DLLEXPORT Leviathan::Rendering::GradientShader::~GradientShader(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::GradientShader::DoesInputObjectWork(ShaderRenderTask* paramstocheck) const{
	// check for data presence of required objects //
	BaseMatrixBufferData* bmtocheck = paramstocheck->GetBaseMatrixBufferData();
	TwoColorBufferData* bctocheck = paramstocheck->GetColourBufferTwo();
	BaseTextureHolder* bttocheck = paramstocheck->GetBaseTextureHolder();

	if(bmtocheck == NULL || bctocheck== NULL || bttocheck == NULL || bttocheck->TextureCount != 1 || !(bttocheck->TextureFlags & TEXTURETYPE_TEXT)){
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

	// set PixelShader resources //
	// we need this temporary because the function actually wants an array of these values //
	ID3D11ShaderResourceView* tmpview = static_cast<SingleTextureHolder*>(parameters->GetBaseTextureHolder())->Texture1->GetView();

	devcont->PSSetShaderResources(0, 1, &tmpview);

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

