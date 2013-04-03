#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_COLORGRADIENTSHADER
#include "ColorGradientShader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ShaderManager.h"

// --------- Shaders --------- //
GradientShader::GradientShader(){
	// set values to null //
	Inited = false;

	VertexShader = NULL;
	PixelShader = NULL;
	Layout = NULL;
	MatrixBuffer = NULL;
	SamplerState = NULL;
	ColorsBuffer = NULL;
}

GradientShader::~GradientShader(){
	if(Inited)
		Release();
}

bool GradientShader::Init(ID3D11Device* device, Window* wind){

	if(!this->InitShader(device, wind, FileSystem::GetShaderFolder()+L"gradient.vs", FileSystem::GetShaderFolder()+L"gradient.ps")){

		Logger::Get()->Error(L"Failed to init Shaders, InitShader failed",0);
		return false;
	}

	Inited = true;
	return true;
}

void GradientShader::Release(){
	// shutdown shaders //
	ReleaseShader();

}
// ------------------------------------ //
bool GradientShader::Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, Float4 colorstart, Float4 colorend){

	// Set the shader parameters that it will use for rendering.
	if(!SetShaderParams(devcont, worldmatrix, viewmatrix, projectionmatrix, colorstart, colorend)){
		return false;
	}

	// render buffers with shader //
	ShaderRender(devcont, indexcount);

	return true;
}

bool GradientShader::InitShader(ID3D11Device* device, Window* wind, wstring vsfilename, wstring psfilename){

	HRESULT hr = S_OK;
	ID3D10Blob* Errordumb;
	ID3D10Blob* Vertexshaderbuffer;
	ID3D10Blob* Pixelshaderbuffer;


	// init objects to null //
	Errordumb = NULL;
	Vertexshaderbuffer = NULL;
	Pixelshaderbuffer = NULL;

	// set up compile parameters //
	UINT CompileFlags = ShaderManager::GetShaderCompileFlags();

	// compile shaders //
	hr = D3DX11CompileFromFile(vsfilename.c_str(), NULL, NULL, "GradientVertexShader", "vs_5_0", CompileFlags, 0, NULL, 
						&Vertexshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr)){
		// check for compile error //
		if(Errordumb){
			PrintShaderError(Errordumb);
			Logger::Get()->Error(L"Failed to Init vetexshader, see info for error",0);
		} else {
			// file was not found //
			Logger::Get()->Error(L"InitShader failed, can't find Vertexshader :"+vsfilename,GetLastError());
		}
			
		return false;
	}

	// pixel shader compile //
	hr = D3DX11CompileFromFile(psfilename.c_str(), NULL, NULL, "GradientPixelShader", "ps_5_0", CompileFlags, 0, NULL, 
						&Pixelshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr)){
		// check for compile error //
		if(Errordumb){
			PrintShaderError(Errordumb);
			Logger::Get()->Error(L"Failed to Init pixelshader, see info for error",0);
		} else {
			// file was not found //
			Logger::Get()->Error(L"InitShader failed, can't find Vertexshader :"+vsfilename,GetLastError());
		}
		return false;
	}
	// create shaders from buffers
	hr = device->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader);
	if(FAILED(hr)){
		Logger::Get()->Error(L"InitShader failed, failed to create VertexShader from buffer",hr);
		return false;
	}

	hr = device->CreatePixelShader(Pixelshaderbuffer->GetBufferPointer(), Pixelshaderbuffer->GetBufferSize(), NULL, &PixelShader);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"InitShader failed, failed to create PixelShader from buffer",hr);
		return false;
	}
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	// setup shader layout data //
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;



	// get element count //
	unsigned int numElements;
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// create input layout //
	hr = device->CreateInputLayout(polygonLayout, numElements, Vertexshaderbuffer->GetBufferPointer(), 
						Vertexshaderbuffer->GetBufferSize(), &Layout);
	if(FAILED(hr)){
		Logger::Get()->Error(L"InitShader failed, failed to create layout object",hr);
		return false;
	}

	// release shader buffers
	SAFE_RELEASE(Vertexshaderbuffer);
	SAFE_RELEASE(Pixelshaderbuffer);

	   
	// Create a texture sampler state description.
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
	hr = device->CreateSamplerState(&samplerDesc, &SamplerState);
	if(FAILED(hr)){
		Logger::Get()->Error(L"InitShader failed, failed to create samplerstate",hr);
		return false;
	}
	// setup matrix buffer //
	D3D11_BUFFER_DESC matrixBufferDesc;

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	hr = device->CreateBuffer(&matrixBufferDesc, NULL, &MatrixBuffer);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"InitShader failed, failed to create MatrixBuffer",hr);
		return false;
	}

	
	// setup color buffer
	D3D11_BUFFER_DESC colorbufferdesc;
	colorbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	colorbufferdesc.ByteWidth = sizeof(ColorBuffer);
	colorbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	colorbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	colorbufferdesc.MiscFlags = 0;
	colorbufferdesc.StructureByteStride = 0;

	// Create the camera constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	hr = device->CreateBuffer(&colorbufferdesc, NULL, &ColorsBuffer);
	if(FAILED(hr)){
		return false;
	}


	return true;
}


void GradientShader::ReleaseShader(){

	SAFE_RELEASE(ColorsBuffer);
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(Layout);
	SAFE_RELEASE(PixelShader);
	SAFE_RELEASE(VertexShader);
	SAFE_RELEASE(SamplerState);

}
void GradientShader::PrintShaderError(ID3D10Blob* datadumb){
	char* Errors;
	unsigned long Errossize;
	wstring FinalError;


	// get erros from dumb
	Errors = (char*)(datadumb->GetBufferPointer());
	Errossize = datadumb->GetBufferSize();


	// copy message to wstring //
	FinalError = L"";
	for(unsigned int i=0; i < Errossize; i++)
	{
		FinalError += Errors[i];
	}

	Logger::Get()->Info(L"Shader error datadumb :\n"+FinalError,true);
	// release error data
	SAFE_RELEASE(datadumb);
}
bool GradientShader::SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, Float4 colorstart, Float4 colorend){

	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// prepare shaders //
	D3DXMatrixTranspose(&worldmatrix, &worldmatrix);
	D3DXMatrixTranspose(&viewmatrix, &viewmatrix);
	D3DXMatrixTranspose(&projectionmatrix, &projectionmatrix);

	// lock buffer for writing //
	hr = devcont->Map(MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){
		Logger::Get()->Error(L"SetShaderParams failed, buffer lock failed",hr);
		return false;
	}

	// get pointer to data //
	MatrixBufferType* dataPtr;
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// copy matrices //
	dataPtr->world = worldmatrix;
	dataPtr->view = viewmatrix;
	dataPtr->projection = projectionmatrix;

	// unlock buffer //
	devcont->Unmap(MatrixBuffer, 0);

	// set position of buffer
	unsigned int buffernumber;
	buffernumber = 0;

	// set constant buffer with updated values //
	devcont->VSSetConstantBuffers(buffernumber, 1, &MatrixBuffer);

	// Lock color buffer
	hr = devcont->Map(ColorsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){


		return false;
	}

	// Get a pointer to the data in the constant buffer.
	ColorBuffer* dataPtr3;
	dataPtr3 = (ColorBuffer*)mappedResource.pData;

	// Copy the camera position into the constant buffer.
	dataPtr3->ColorStart = (float*)colorstart;
	dataPtr3->ColorEnd = (float*)colorend;

	// Unlock the camera constant buffer.
	devcont->Unmap(ColorsBuffer, 0);

	// Set the position of the camera constant buffer in the vertex shader.
	buffernumber = 1;

	// set color buffer //
	//devcont->VSSetConstantBuffers(buffernumber, 1, &ColorsBuffer); // might not need this 
	
	// set for pixel shader too //
	devcont->PSSetConstantBuffers(0, 1, &ColorsBuffer);

	return true;
}
void GradientShader::ShaderRender(ID3D11DeviceContext* devcont, int indexcount){
	// set input layout
	devcont->IASetInputLayout(Layout);

	// set shaders //
	devcont->VSSetShader(VertexShader, NULL, 0);
	devcont->PSSetShader(PixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	devcont->PSSetSamplers(0, 1, &SamplerState);

	// render image
	devcont->DrawIndexed(indexcount, 0, 0);
}

