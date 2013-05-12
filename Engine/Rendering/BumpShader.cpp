#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BUMBMAPSHADER
#include "BumpShader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ShaderManager.h"

BumpMapShader::BumpMapShader()
{
	// set values to null //
	Inited = false;

	VertexShader = NULL;
	PixelShader = NULL;
	Layout = NULL;
	MatrixBuffer = NULL;
	SampleState = NULL;
	LightBuffer = NULL;
}


BumpMapShader::~BumpMapShader(){

}


bool BumpMapShader::Init(ID3D11Device* device){

	if(!this->InitShader(device, FileSystem::GetShaderFolder()+L"bumpmap.vs", FileSystem::GetShaderFolder()+L"bumpmap.ps")){

		Logger::Get()->Error(L"Failed to init Shaders, InitShader failed",0);
		return false;
	}

	Inited = true;
	return true;
}


void BumpMapShader::Release(){
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ReleaseShader();

	return;
}


bool BumpMapShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
								D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalMapTexture, 
								Float3 lightDirection, Float4 diffuseColor)
{
	// Set the shader parameters that it will use for rendering.

	if(!SetShaderParams(deviceContext, worldMatrix, viewMatrix, projectionMatrix, colorTexture, normalMapTexture, lightDirection, diffuseColor)){

		return false;
	}

	// Now render the prepared buffers with the shader.
	ShaderRender(deviceContext, indexCount);

	return true;
}
	

bool Leviathan::BumpMapShader::InitShader(ID3D11Device* dev, const wstring &vsfilename, const wstring &psfilename){
	
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
	hr = D3DX11CompileFromFile(vsfilename.c_str(), NULL, NULL, "BumpMapVertexShader", "vs_5_0", CompileFlags, 0, NULL, 
						&Vertexshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr)){

		// check for compile error //
		if(Errordumb) {

			PrintShaderError(Errordumb);
			Logger::Get()->Error(L"Failed to Init vetexshader, see info for error",0);
		}
		else {
			// file was not found //
			Logger::Get()->Error(L"InitShader failed, can't find Vertexshader :"+vsfilename,GetLastError());
		}
			
		return false;
	}

	// pixel shader compile //
	hr = D3DX11CompileFromFile(psfilename.c_str(), NULL, NULL, "BumpMapPixelShader", "ps_5_0", CompileFlags, 0, NULL, 
						&Pixelshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr)){

		// check for compile error //
		if(Errordumb){

			PrintShaderError(Errordumb);
			Logger::Get()->Error(L"Failed to Init pixelshader, see info for error",0);
		}
		else {

			// file was not found //
			Logger::Get()->Error(L"InitShader failed, can't find Vertexshader :"+vsfilename,GetLastError());
		}
		return false;
	}
	// create shaders from buffers
	hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader);
	if(FAILED(hr)){

		Logger::Get()->Error(L"InitShader failed, failed to create VertexShader from buffer",hr);
		return false;
	}

	hr = dev->CreatePixelShader(Pixelshaderbuffer->GetBufferPointer(), Pixelshaderbuffer->GetBufferSize(), NULL, &PixelShader);
	if(FAILED(hr)){

		Logger::Get()->Error(L"InitShader failed, failed to create PixelShader from buffer",hr);
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
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

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "TANGENT";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "BINORMAL";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
		unsigned int numElements;
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	hr = dev->CreateInputLayout(polygonLayout, numElements, Vertexshaderbuffer->GetBufferPointer(), 
									   Vertexshaderbuffer->GetBufferSize(), &Layout);
	if(FAILED(hr)){
		Logger::Get()->Error(L"InitShader failed, failed to create InputLayout",hr);
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	SAFE_RELEASE(Vertexshaderbuffer);
	SAFE_RELEASE(Pixelshaderbuffer);


	// Setup the description of the matrix dynamic constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the matrix constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	hr = dev->CreateBuffer(&matrixBufferDesc, NULL, &MatrixBuffer);
	if(FAILED(hr)){
		Logger::Get()->Error(L"InitShader failed, failed to create Matrix constant buffer",hr);
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
	hr = dev->CreateSamplerState(&samplerDesc, &SampleState);
	if(FAILED(hr)){
		Logger::Get()->Error(L"InitShader failed, failed to create SamplerState",hr);
		return false;
	}

	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	hr = dev->CreateBuffer(&lightBufferDesc, NULL, &LightBuffer);
	if(FAILED(hr)){

		Logger::Get()->Error(L"InitShader failed, failed to create LightBuffer",hr);
		return false;
	}

	return true;
}


void BumpMapShader::ReleaseShader(){
	// release objects //
	SAFE_RELEASE(LightBuffer);

	SAFE_RELEASE(SampleState);

	SAFE_RELEASE(MatrixBuffer);

	SAFE_RELEASE(Layout);

	SAFE_RELEASE(PixelShader);

	SAFE_RELEASE(VertexShader);
}


void BumpMapShader::PrintShaderError(ID3D10Blob* datadumb){
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


bool BumpMapShader::SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldMatrix, 
											 D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, 
											 ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalMapTexture, 
											 Float3 lightDirection, Float4 diffuseColor)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;
	LightBufferType* dataPtr2;


	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the matrix constant buffer so it can be written to.
	hr = devcont->Map(MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the matrix constant buffer.
	devcont->Unmap(MatrixBuffer, 0);

	// Set the position of the matrix constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
	devcont->VSSetConstantBuffers(bufferNumber, 1, &MatrixBuffer);

	// Set shader texture resources in the pixel shader.
	devcont->PSSetShaderResources(0, 1, &colorTexture);
	devcont->PSSetShaderResources(1, 1, &normalMapTexture);

	// Lock the light constant buffer so it can be written to.
	hr = devcont->Map(LightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){

		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->diffuseColor = D3DXVECTOR4(diffuseColor[0], diffuseColor[1], diffuseColor[2], diffuseColor[3]);
	dataPtr2->lightDirection = D3DXVECTOR3(lightDirection[0], lightDirection[1], lightDirection[2]);

	// Unlock the constant buffer.
	devcont->Unmap(LightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	devcont->PSSetConstantBuffers(bufferNumber, 1, &LightBuffer);

	// set the texture! //
	devcont->PSSetShaderResources(0, 1, &colorTexture);
	devcont->PSSetShaderResources(1, 1, &normalMapTexture);

	return true;
}


void BumpMapShader::ShaderRender(ID3D11DeviceContext* devcont, int indexcount)
{
	// Set the vertex input layout.
	devcont->IASetInputLayout(Layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	devcont->VSSetShader(VertexShader, NULL, 0);
	devcont->PSSetShader(PixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	devcont->PSSetSamplers(0, 1, &SampleState);

	// Render the triangles.
	devcont->DrawIndexed(indexcount, 0, 0);

	// clear shader resources //
	devcont->PSSetShaderResources(0, 0, NULL);
	devcont->PSSetShaderResources(1, 0, NULL);

	return;
}