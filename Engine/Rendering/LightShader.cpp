#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LIGHTSHADER
#include "LightShader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ShaderManager.h"

// --------- Shaders --------- //
LightShader::LightShader(){
	// set values to null //
	Inited = false;

	VertexShader = NULL;
	PixelShader = NULL;
	Layout = NULL;
	MatrixBuffer = NULL;
	SamplerState = NULL;
	CameraBuffer = NULL;
	LightBuffer = NULL;
}

LightShader::~LightShader(){
	if(Inited)
		Release();
}

bool Leviathan::LightShader::Init(ID3D11Device* device){

	if(!this->InitShader(device, FileSystem::GetShaderFolder()+L"light.vs", FileSystem::GetShaderFolder()+L"light.ps")){

		Logger::Get()->Error(L"Failed to init Shaders, InitShader failed",0);
		return false;
	}

	Inited = true;
	return true;
}

void LightShader::Release(){
	// shutdown shaders //
	ReleaseShader();

}
// ------------------------------------ //
bool LightShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, 
	Float3 lightDirection, Float4 ambientColor, Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower){

	// Set the shader parameters that it will use for rendering.
	if(!SetShaderParams(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, ambientColor, diffuseColor, cameraPosition, specularColor, specularPower)){
		return false;
	}

	// render buffers with shader //
	ShaderRender(deviceContext, indexCount);

	return true;
}

bool Leviathan::LightShader::InitShader(ID3D11Device* dev, const wstring &vsfilename, const wstring &psfilename){

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
	hr = D3DX11CompileFromFile(vsfilename.c_str(), NULL, NULL, "LightVertexShader", "vs_5_0", CompileFlags, 0, NULL, 
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
	hr = D3DX11CompileFromFile(psfilename.c_str(), NULL, NULL, "LightPixelShader", "ps_5_0", CompileFlags, 0, NULL, 
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
	hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"InitShader failed, failed to create VertexShader from buffer",hr);
		return false;
	}

	hr = dev->CreatePixelShader(Pixelshaderbuffer->GetBufferPointer(), Pixelshaderbuffer->GetBufferSize(), NULL, &PixelShader);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"InitShader failed, failed to create PixelShader from buffer",hr);
		return false;
	}
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
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

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	// get element count //
	unsigned int numElements;
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// create input layout //
	hr = dev->CreateInputLayout(polygonLayout, numElements, Vertexshaderbuffer->GetBufferPointer(), 
						Vertexshaderbuffer->GetBufferSize(), &Layout);
	if(FAILED(hr))
	{
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
	hr = dev->CreateSamplerState(&samplerDesc, &SamplerState);
	if(FAILED(hr))
	{
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

	hr = dev->CreateBuffer(&matrixBufferDesc, NULL, &MatrixBuffer);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"InitShader failed, failed to create MatrixBuffer",hr);
		return false;
	}

	
	// Setup the description of the camera dynamic constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the camera constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	hr = dev->CreateBuffer(&cameraBufferDesc, NULL, &CameraBuffer);
	if(FAILED(hr))
	{
		return false;
	}

	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	hr = dev->CreateBuffer(&lightBufferDesc, NULL, &LightBuffer);
	if(FAILED(hr))
	{
		return false;
	}
	return true;
}


void LightShader::ReleaseShader(){
	SAFE_RELEASE(LightBuffer);
	SAFE_RELEASE(CameraBuffer);
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(Layout);
	SAFE_RELEASE(PixelShader);
	SAFE_RELEASE(VertexShader);
	SAFE_RELEASE(SamplerState);


}
void LightShader::PrintShaderError(ID3D10Blob* datadumb){
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
bool LightShader::SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture, 
	Float3 lightDirection, Float4 ambientColor, Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower)
{

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

	// Lock the camera constant buffer so it can be written to.
	hr = devcont->Map(CameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	CameraBufferType* dataPtr3;
	dataPtr3 = (CameraBufferType*)mappedResource.pData;

	// Copy the camera position into the constant buffer.
	dataPtr3->cameraPosition = D3DXVECTOR3(cameraPosition.X, cameraPosition.Y, cameraPosition.Z);
	dataPtr3->padding = 0.0f;

	// Unlock the camera constant buffer.
	devcont->Unmap(CameraBuffer, 0);

	// Set the position of the camera constant buffer in the vertex shader.
	buffernumber = 1;

	// Now set the camera constant buffer in the vertex shader with the updated values.
	devcont->VSSetConstantBuffers(buffernumber, 1, &CameraBuffer);

	// Lock the light constant buffer so it can be written to.
	hr = devcont->Map(LightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr))
	{
		return false;
	}

	// Get a pointer to the data in the light constant buffer.
	LightBufferType* dataPtr2;
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the light constant buffer.
	dataPtr2->ambientColor = D3DXVECTOR4(ambientColor[0], ambientColor[1], ambientColor[2], ambientColor[2]);
	dataPtr2->diffuseColor = D3DXVECTOR4(diffuseColor[0], diffuseColor[1], diffuseColor[2], diffuseColor[2]);
	dataPtr2->lightDirection = D3DXVECTOR3(lightDirection[0], lightDirection[1], lightDirection[2]);
	dataPtr2->specularColor = D3DXVECTOR4(specularColor[0], specularColor[1], specularColor[2], specularColor[3]);
	dataPtr2->specularPower = specularPower;
	
	// Unlock the light constant buffer.
	devcont->Unmap(LightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	buffernumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	devcont->PSSetConstantBuffers(buffernumber, 1, &LightBuffer);

	// set the texture! //
	devcont->PSSetShaderResources(0, 1, &texture);

	return true;
}
void LightShader::ShaderRender(ID3D11DeviceContext* devcont, int indexcount){
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
	//devcont->PSSetShaderResources(1, 1, NULL); // not used //
}

