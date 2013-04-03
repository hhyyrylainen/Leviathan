#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SKINNEDSHADER_SMALL
#include "SkinnedShaderSmall.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ShaderManager.h"


// ------------------------------------ //
SkinnedShaderSmall::SkinnedShaderSmall(){
	// set values to null //
	Inited = false;

	VertexShader = NULL;
	PixelShader = NULL;
	Layout = NULL;
	MatrixBuffer = NULL;
	SamplerState = NULL;
	CameraBuffer = NULL;
	LightBuffer = NULL;
	BoneMatriceBuffer = NULL;
}

SkinnedShaderSmall::~SkinnedShaderSmall(){
	if(Inited)
		Release();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SkinnedShaderSmall::Init(ID3D11Device* device){

	if(!this->InitShader(device, FileSystem::GetShaderFolder()+L"SkinnedModelShader.hlsl")){

		Logger::Get()->Error(L"SkinnedShaderSmall: Init: internal init failed", true);
		return false;
	}

	Inited = true;
	return true;
}

void Leviathan::SkinnedShaderSmall::Release(){
	// Release all data //
	ReleaseShader();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SkinnedShaderSmall::Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, 
	D3DXMATRIX projectionmatrix, GameObject::SkeletonRig* Bones, ID3D11ShaderResourceView* texture, Float3 lightDirection, Float4 ambientColor, 
	Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower)
{

	// Set the shader parameters for rendering //
	if(!SetShaderParams(devcont, worldmatrix, viewmatrix, projectionmatrix, texture, Bones, lightDirection, ambientColor, diffuseColor, cameraPosition, specularColor, specularPower)){
		return false;
	}

	// Render everything with the shader //
	ShaderRender(devcont, indexcount);

	return true;
}

bool Leviathan::SkinnedShaderSmall::InitShader(ID3D11Device* dev, const wstring &shaderfile){

	HRESULT hr = S_OK;
	ID3D10Blob* Errordumb(NULL);
	ID3D10Blob* Vertexshaderbuffer(NULL);
	ID3D10Blob* Pixelshaderbuffer(NULL);

	// set up compile parameters //
	UINT CompileFlags = ShaderManager::GetShaderCompileFlags();

	// needs to define the bone count for the compiler //
	D3D10_SHADER_MACRO Shader_Macros[2] = {  {"MAX_TRANSFORMS", "10"}, {NULL, NULL}  };

	// Compile Vertex shader //
	hr = D3DX11CompileFromFile(shaderfile.c_str(), Shader_Macros, NULL, "SkinnedVertexShader", "vs_5_0", CompileFlags, 0, NULL, 
						&Vertexshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr)){
		// check for compile error //

		if(Errordumb){

			ShaderManager::PrintShaderError(L"SkinnedShaderSmall", Errordumb);
			Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: vertex shader compile failed");
		} else {
			// file was not found //
			Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: can't find file: "+shaderfile);
		}
		return false;
	}

	// pixel shader compile //
	hr = D3DX11CompileFromFile(shaderfile.c_str(), Shader_Macros, NULL, "LightPixelShader", "ps_5_0", CompileFlags, 0, NULL, 
						&Pixelshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr)){
		// check for compile error //
		if(Errordumb){

			ShaderManager::PrintShaderError(L"SkinnedShaderSmall", Errordumb);
			Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: pixel shader compile failed");
		} else {
			// file was not found //
			Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: can't find file: "+shaderfile);
		}
		return false;
	}

	// Create the actual shaders from the buffers //
	hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: failed to create VertexShader from buffer",(int)hr);
		return false;
	}

	hr = dev->CreatePixelShader(Pixelshaderbuffer->GetBufferPointer(), Pixelshaderbuffer->GetBufferSize(), NULL, &PixelShader);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: failed to create PixelShader from buffer",(int)hr);
		return false;
	}


	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
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

	polygonLayout[3].SemanticName = "BONEIDS";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_UINT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "BONEWEIGHTS";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

	// get element count //
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// create input layout //
	hr = dev->CreateInputLayout(polygonLayout, numElements, Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), &Layout);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: failed to create layout object",hr);
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
	if(FAILED(hr)){
		Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: failed to create sampler state",hr);
		return false;
	}
	// -- Set up buffers -- //

	// setup matrix buffer //
	D3D11_BUFFER_DESC MatrixBufferDesc;
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	MatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	MatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBufferDesc.MiscFlags = 0;
	MatrixBufferDesc.StructureByteStride = 0;

	hr = dev->CreateBuffer(&MatrixBufferDesc, NULL, &MatrixBuffer);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: failed to create MatrixBuffer",hr);
		return false;
	}
	
	// Dynamic camera buffer //
	D3D11_BUFFER_DESC CameraBufferDesc;
	CameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	CameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	CameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	CameraBufferDesc.MiscFlags = 0;
	CameraBufferDesc.StructureByteStride = 0;

	// Create it //
	hr = dev->CreateBuffer(&CameraBufferDesc, NULL, &CameraBuffer);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: failed to create CameraBuffer",hr);
		return false;
	}

	// Light buffer //

	D3D11_BUFFER_DESC LightBufferDesc;
	LightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	LightBufferDesc.ByteWidth = sizeof(LightBufferType);
	LightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	LightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	LightBufferDesc.MiscFlags = 0;
	LightBufferDesc.StructureByteStride = 0;

	// Create it //
	hr = dev->CreateBuffer(&LightBufferDesc, NULL, &LightBuffer);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: failed to create LightBuffer",hr);
		return false;
	}

	// NEW //
	// Description for Bone transform matrices //
	D3D11_BUFFER_DESC BoneMatriceBufferDesc;
	BoneMatriceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BoneMatriceBufferDesc.ByteWidth = sizeof(BoneTransformsBufferType);
	BoneMatriceBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BoneMatriceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BoneMatriceBufferDesc.MiscFlags = 0;
	BoneMatriceBufferDesc.StructureByteStride = 0;

	// Create //
	hr = dev->CreateBuffer(&BoneMatriceBufferDesc, NULL, &BoneMatriceBuffer);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: InitShader: failed to create BoneMatriceBuffer",hr);
		return false;
	}

	return true;
}


void Leviathan::SkinnedShaderSmall::ReleaseShader(){
	// release all allocations //
	SAFE_RELEASE(LightBuffer);
	SAFE_RELEASE(CameraBuffer);
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(Layout);
	SAFE_RELEASE(PixelShader);
	SAFE_RELEASE(VertexShader);
	SAFE_RELEASE(SamplerState);
	SAFE_RELEASE(BoneMatriceBuffer);
}
bool Leviathan::SkinnedShaderSmall::SetShaderParams(ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture, 
	GameObject::SkeletonRig* Bones, Float3 lightDirection, Float4 ambientColor, Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower)
{

	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// prepare shaders //
	D3DXMatrixTranspose(&worldmatrix, &worldmatrix);
	D3DXMatrixTranspose(&viewmatrix, &viewmatrix);
	D3DXMatrixTranspose(&projectionmatrix, &projectionmatrix);

	// Unbind old buffers //
	//devcont->VSSetConstantBuffers(0, 20, ShaderManager::NULLBufferBlob);
	//devcont->PSSetConstantBuffers(0, 20, ShaderManager::NULLBufferBlob);

	// lock buffer for writing //
	hr = devcont->Map(MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: SetShaderParams: buffer lock failed",hr);
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
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: SetShaderParams: buffer lock failed",hr);
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	CameraBufferType* dataPtr3;
	dataPtr3 = (CameraBufferType*)mappedResource.pData;

	// Copy the camera position into the constant buffer.
	dataPtr3->cameraPosition = D3DXVECTOR3(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
	dataPtr3->padding = 0.0f;

	// Unlock the camera constant buffer.
	devcont->Unmap(CameraBuffer, 0);

	// Set the position of the camera constant buffer in the vertex shader.
	buffernumber = 1;

	// Now set the camera constant buffer in the vertex shader with the updated values.
	devcont->VSSetConstantBuffers(buffernumber, 1, &CameraBuffer);


	// Data to Bone transform matrice buffer //
	// map buffer for writing //
	hr = devcont->Map(BoneMatriceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: SetShaderParams: buffer lock failed",hr);
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	BoneTransformsBufferType* BoneBuffer = (BoneTransformsBufferType*)mappedResource.pData;

	// copy the current transforms into the buffer //
	if(Bones){
		// get values from the rig //
		if(!Bones->CopyValuesToBuffer(BoneBuffer)){
			// invalid bone stuff //
			return false;
		}

	} else {
		// clear the buffer //
		(*BoneBuffer) = BoneTransformsBufferType();
	}

	// Unlock the buffer
	devcont->Unmap(BoneMatriceBuffer, 0);

	// Set position (avoid overwriting old buffers) //
	buffernumber = 2;

	// Now set the camera constant buffer in the vertex shader with the updated values.
	devcont->VSSetConstantBuffers(buffernumber, 1, &BoneMatriceBuffer);


	// Lock the light constant buffer so it can be written to.
	hr = devcont->Map(LightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShaderSmall: SetShaderParams: buffer lock failed",hr);
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
void Leviathan::SkinnedShaderSmall::ShaderRender(ID3D11DeviceContext* devcont, int indexcount){
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

