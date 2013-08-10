#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASESHADER
#include "BaseShader.h"
#endif
#include "Rendering\ShaderManager.h"
using namespace Leviathan;
// ------------------------------------ //


Leviathan::BaseShader::BaseShader(const wstring &shaderfile, const string &vsentry, const string &psentry, const string &inputpattern) : Inited(false), 
	VertexShader(NULL), PixelShader(NULL), SamplerState(NULL), Layout(NULL),
	ShaderFileName(shaderfile), VSShaderEntryPoint(vsentry), PSShaderEntryPoint(psentry), ShaderInputPattern(inputpattern){

}

DLLEXPORT Leviathan::BaseShader::~BaseShader(){
	if(Inited){
		// just so that no shader is left initialized //
		Release();
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseShader::Init(ID3D11Device* device){
	// BaseShader has nothing else to init so this function is just this //
	return CreateShader(device);
}

DLLEXPORT void Leviathan::BaseShader::Release(){
	ReleaseShader();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseShader::Render(ID3D11DeviceContext* devcont,int indexcount, ShaderRenderTask* Parameters){
	// call set parameters which must be defined by each child class //
	if(!SetShaderParams(devcont, Parameters)){

		Logger::Get()->Error(L"BaseShader: Render: failed to set shader parameters");
		return false;
	}
	ShaderRender(devcont, VertexShader, PixelShader, indexcount);
	return true;
}

void Leviathan::BaseShader::ShaderRender(ID3D11DeviceContext* devcont, ID3D11VertexShader* vertexshader, ID3D11PixelShader* pixelshader, 
	const int &indexcount)
{
	// set input layout //
	devcont->IASetInputLayout(Layout);

	// set shaders //
	devcont->VSSetShader(vertexshader, NULL, 0);
	devcont->PSSetShader(pixelshader, NULL, 0);

	// set the sampler state for the pixel shader //
	devcont->PSSetSamplers(0, 1, &SamplerState);

	// render image //
	devcont->DrawIndexed(indexcount, 0, 0);
}
// ------------------------------------ //
bool Leviathan::BaseShader::CreateShader(ID3D11Device* dev){
	if(!LoadShaderFromDisk(dev)){

		Logger::Get()->Error(L"CreateShader: failed to load shader code");
		return false;
	}
	if(!SetupShaderDataBuffers(dev)){

		Logger::Get()->Error(L"CreateShader: failed to setup shader specific buffers");
		return false;
	}

	return Inited = true;
}

bool Leviathan::BaseShader::LoadShaderFromDisk(ID3D11Device* dev){
	// set up compile parameters //
	UINT CompileFlags = ShaderManager::GetShaderCompileFlags();

	//D3D10_SHADER_MACRO Shader_Macros[2] = {  {"MAX_TRANSFORMS", maxcount.c_str()}, {NULL, NULL}  };
	D3D10_SHADER_MACRO Shader_Macros[1] = {{NULL, NULL}};

	// init objects to null //
	ID3D10Blob* Errordumb = NULL;
	ID3D10Blob* Vertexshaderbuffer = NULL;
	ID3D10Blob* Pixelshaderbuffer = NULL;
	HRESULT hr = S_OK;

	// compile shaders //
	for(int i = 0; i < 2; i++){

		if(i == 0){
			// compile vertex shader //
			hr = D3DX11CompileFromFile(ShaderFileName.c_str(), &Shader_Macros[0], NULL, VSShaderEntryPoint.c_str(), "vs_5_0", CompileFlags, 0, 
				NULL, &Vertexshaderbuffer, &Errordumb, NULL);
		} else {
			// pixel shader compile //
			hr = D3DX11CompileFromFile(ShaderFileName.c_str(), &Shader_Macros[0], NULL, PSShaderEntryPoint.c_str(), "ps_5_0", CompileFlags, 0, NULL, 
				&Pixelshaderbuffer, &Errordumb, NULL);
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
	}
	// buffers are needed for input layouts //
	if(!SetupShaderInputLayouts(dev, Vertexshaderbuffer)){
		// release shader buffers
		SAFE_RELEASE(Vertexshaderbuffer);
		SAFE_RELEASE(Pixelshaderbuffer);
		Logger::Get()->Error(L"InitShader: cannot create input layout");
		return false;
	}

	// create shaders from buffers
	hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader);
	if(FAILED(hr)){
		Logger::Get()->Error(L"InitShader: failed to create VertexShader from buffer", hr);
		return false;
	}
	hr = dev->CreatePixelShader(Pixelshaderbuffer->GetBufferPointer(), Pixelshaderbuffer->GetBufferSize(), NULL, &PixelShader);
	if(FAILED(hr)){
		Logger::Get()->Error(L"InitShader: failed to create PixelShader from buffer", hr);
		return false;
	}
	// release shader buffers
	SAFE_RELEASE(Vertexshaderbuffer);
	SAFE_RELEASE(Pixelshaderbuffer);

	// succeeded //
	return true;
}

void Leviathan::BaseShader::ReleaseShader(){
	// call unloading functions //
	UnloadShader();
	UnloadSamplerAndLayout();
	ReleaseShaderDataBuffers();

	Inited = false;
}

void Leviathan::BaseShader::UnloadShader(){
	SAFE_RELEASE(VertexShader);
	SAFE_RELEASE(PixelShader);
}

void Leviathan::BaseShader::UnloadSamplerAndLayout(){
	SAFE_RELEASE(Layout);
	SAFE_RELEASE(SamplerState);
}
// ------------------------------------ //
bool Leviathan::BaseShader::CreateDefaultSamplerState(ID3D11Device* dev){
	// create the texture sampler description //
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT hr = dev->CreateSamplerState(&samplerDesc, &SamplerState);
	if(FAILED(hr)){

		Logger::Get()->Error(L"InitShader: failed to create a ID3D11SamplerState", hr);
		return false;
	}

	return true;
}

bool Leviathan::BaseShader::CreateInputLayout(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer, D3D11_INPUT_ELEMENT_DESC* layout, 
	const UINT &elementcount)
{
	// create input layout //
	HRESULT hr = dev->CreateInputLayout(layout, elementcount, VertexShaderBuffer->GetBufferPointer(), VertexShaderBuffer->GetBufferSize(), &Layout);
	if(FAILED(hr)){

		Logger::Get()->Error(L"InitShader failed, failed to create layout object",hr);
		return false;
	}
	return true;
}

// ------------------ TextureShader ------------------ //
DLLEXPORT Leviathan::TextureShader::TextureShader() : BaseShader(L"TextureShader.hlsl", "TextureVertexShader", "TexturePixelShader", 
	"BUF:BMAT:TEX:NORMAL"), MatrixBuffer(NULL)
{

}

DLLEXPORT Leviathan::TextureShader::~TextureShader(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::TextureShader::DoesInputObjectWork(ShaderRenderTask* paramstocheck) const{
	// check for data presence of required objects //
	BaseMatrixBufferData* bmtocheck = paramstocheck->GetBaseMatrixBufferData();
	BaseTextureHolder* bttocheck = paramstocheck->GetBaseTextureHolder();

	if(bmtocheck == NULL || bttocheck == NULL || bttocheck->TextureCount != 1 || bttocheck->TextureFlags & TEXTURETYPE_NORMAL){
		// failed a check //
		return false;
	}

	// passed all tests //
	return true;
}
// ------------------------------------ //
bool Leviathan::TextureShader::SetupShaderDataBuffers(ID3D11Device* dev){
	// create this shader specific buffers //
	if(!Rendering::ResourceCreator::CreateDynamicConstantBufferForVSShader(&MatrixBuffer, sizeof(MatrixBufferType))){

		return false;
	}

	return true;
}

void Leviathan::TextureShader::ReleaseShaderDataBuffers(){
	// this shader specific buffers //
	SAFE_RELEASE(MatrixBuffer);
}
// ------------------------------------ //
bool Leviathan::TextureShader::SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
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

	// set PixelShader resources //
	// we need this temporary because the function actually wants an array of these values //
	ID3D11ShaderResourceView* tmpview = static_cast<SingleTextureHolder*>(parameters->GetBaseTextureHolder())->Texture1->GetView();

	devcont->PSSetShaderResources(0, 1, &tmpview);

	return true;
}

bool Leviathan::TextureShader::SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters){
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

	return true;
}

bool Leviathan::TextureShader::SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer){
	// create layout //
	D3D11_INPUT_ELEMENT_DESC shaderdatalayoutdesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	// calculate element count //
	UINT elementcount = sizeof(shaderdatalayoutdesc)/sizeof(shaderdatalayoutdesc[0]);

	if(!CreateInputLayout(dev, VertexShaderBuffer, &shaderdatalayoutdesc[0], elementcount)){

		return false;
	}

	return true;
}










