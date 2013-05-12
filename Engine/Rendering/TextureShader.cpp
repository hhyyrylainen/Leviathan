#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TEXTURESHADER
#include "TextureShader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ShaderManager.h"

// --------- Shaders --------- //
TextureShader::TextureShader(){
	// set values to null //
	Inited = false;

	VertexShader = NULL;
	PixelShader = NULL;
	Layout = NULL;
	MatrixBuffer = NULL;
	SamplerState = NULL;
}

TextureShader::~TextureShader(){
	if(Inited)
		Release();
}

bool TextureShader::Init(ID3D11Device* device){
	if(!this->InitShader(device, FileSystem::GetShaderFolder()+L"texture.vs", FileSystem::GetShaderFolder()+L"texture.ps")){

		Logger::Get()->Error(L"Failed to init Shaders, InitShader failed",0);
		return false;
	}

	Inited = true;
	return true;
}

void TextureShader::Release(){
	// shutdown shaders //
	ReleaseShader();

}
// ------------------------------------ //
bool TextureShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture){

	// Set the shader parameters that it will use for rendering.
	if(!SetShaderParams(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture)){
		return false;
	}

	// render buffers with shader //
	ShaderRender(deviceContext, indexCount);

	return true;
}

bool Leviathan::TextureShader::InitShader(ID3D11Device* dev, const wstring &vsfilename, const wstring &psfilename){
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
	hr = D3DX11CompileFromFile(vsfilename.c_str(), NULL, NULL, "TextureVertexShader", "vs_5_0", CompileFlags, 0, NULL, 
						&Vertexshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr))
	{
		// check for compile error //
		if(Errordumb)
		{
			PrintShaderError(Errordumb);
			Logger::Get()->Error(L"Failed to Init vetexshader, see info for error",0);
		}
		else
		{
			// file was not found //
			Logger::Get()->Error(L"InitShader failed, can't find Vertexshader :"+vsfilename,GetLastError());
		}
			
		return false;
	}

	// pixel shader compile //
	hr = D3DX11CompileFromFile(psfilename.c_str(), NULL, NULL, "TexturePixelShader", "ps_5_0", CompileFlags, 0, NULL, 
						&Pixelshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr))
	{
		// check for compile error //
		if(Errordumb)
		{
			PrintShaderError(Errordumb);
			Logger::Get()->Error(L"Failed to Init pixelshader, see info for error",0);
		}
		else
		{
			// file was not found //
			Logger::Get()->Error(L"InitShader failed, can't find Vertexshader :"+vsfilename,GetLastError());
		}
		return false;
	}
	// create shaders from buffers
	hr = device->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader);
	if(FAILED(hr))
	{
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
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"InitShader failed, failed to create layout object",hr);
		return false;
	}

	// release shader buffers
	SAFE_RELEASE(Vertexshaderbuffer);
	SAFE_RELEASE(Pixelshaderbuffer);

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
	if(FAILED(hr))
	{
		return false;
	}

	return true;
}


void TextureShader::ReleaseShader(){
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(Layout);
	SAFE_RELEASE(PixelShader);
	SAFE_RELEASE(VertexShader);
	SAFE_RELEASE(SamplerState);

}
void TextureShader::PrintShaderError(ID3D10Blob* datadumb){
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
bool TextureShader::SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture){
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// prepare shaders //
	D3DXMatrixTranspose(&worldmatrix, &worldmatrix);
	D3DXMatrixTranspose(&viewmatrix, &viewmatrix);
	D3DXMatrixTranspose(&projectionmatrix, &projectionmatrix);

	// lock buffer for writing //
	hr = devcont->Map(MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr))
	{
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

	// Set shader texture resource in the pixel shader.
	devcont->PSSetShaderResources(0, 1, &texture);

	return true;
}
void TextureShader::ShaderRender(ID3D11DeviceContext* devcont, int indexcount){
	// set input layout
	devcont->IASetInputLayout(Layout);

	// set shaders //
	devcont->VSSetShader(VertexShader, NULL, 0);
	devcont->PSSetShader(PixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	devcont->PSSetSamplers(0, 1, &SamplerState);

	// render image
	devcont->DrawIndexed(indexcount, 0, 0);

	// clear shader resources //
	devcont->PSSetShaderResources(0, 0, NULL);
	//devcont->PSSetShaderResources(1, 1, NULL); // not used by this shader //
}

