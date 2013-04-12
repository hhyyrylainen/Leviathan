#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SKINNEDSHADER_SMALL
#include "SkinnedShader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ShaderManager.h"


// ------------------------------------ //
SkinnedShader::SkinnedShader(){
	// set values to null //
	Inited = false;

	// definitions for all sizes //
	VertexShader_tiny = NULL;
	//Layout_tiny = NULL;
	BoneMatriceBuffer_tiny = NULL;

	VertexShader_small = NULL;
	//Layout_small = NULL;
	BoneMatriceBuffer_small = NULL;

	VertexShader_medium = NULL;
	//Layout_medium = NULL;
	BoneMatriceBuffer_medium = NULL;

	VertexShader_large = NULL;
	//Layout_large = NULL;
	BoneMatriceBuffer_large = NULL;

	VertexShader_huge = NULL;
	//Layout_huge = NULL;
	BoneMatriceBuffer_huge = NULL;

	VertexShader_max = NULL;
	//Layout_max = NULL;
	BoneMatriceBuffer_max = NULL;

	PixelShader = NULL;
	Layout = NULL;

	MatrixBuffer = NULL;
	SamplerState = NULL;
	CameraBuffer = NULL;
	LightBuffer = NULL;

}

SkinnedShader::~SkinnedShader(){
	if(Inited)
		Release();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SkinnedShader::Init(ID3D11Device* device){

	if(!this->InitShader(device, FileSystem::GetShaderFolder()+L"SkinnedModelShader.hlsl")){

		Logger::Get()->Error(L"SkinnedShader: Init: internal init failed", true);
		return false;
	}

	Inited = true;
	return true;
}

void Leviathan::SkinnedShader::Release(){
	// Release all data //
	ReleaseShader();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SkinnedShader::Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, 
	D3DXMATRIX projectionmatrix, GameObject::SkeletonRig* Bones, ID3D11ShaderResourceView* texture, Float3 lightDirection, Float4 ambientColor, 
	Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower)
{

	SHADER_BONECOUNT RequiredBones;
	if(Bones->GetBoneCount() <= MAX_BONES_TINY){
		RequiredBones = SHADER_BONES_TINY;
	} else if(Bones->GetBoneCount() <= MAX_BONES_SMALL){
		RequiredBones = SHADER_BONES_SMALL;
	} else if(Bones->GetBoneCount() <= MAX_BONES_MEDIUM){
		RequiredBones = SHADER_BONES_MEDIUM;
	} else if(Bones->GetBoneCount() <= MAX_BONES_LARGE){
		RequiredBones = SHADER_BONES_LARGE;
	} else if(Bones->GetBoneCount() <= MAX_BONES_HUGE){
		RequiredBones = SHADER_BONES_HUGE;
	} else if(Bones->GetBoneCount() <= MAX_BONES_MAX){
		RequiredBones = SHADER_BONES_MAX;
	} else {

		ComplainOnce::PrintErrorOnce(L"SkinnedShader: Render: too many bones!", L"SkinnedShader: Render: too many bones! max is "+Convert::IntToWstring(MAX_BONES_MAX)
			+L" bone count: "+Convert::IntToWstring(Bones->GetBoneCount()));
		return false;
	}

	// Set the shader parameters for rendering //
	if(!SetShaderParams(RequiredBones, devcont, worldmatrix, viewmatrix, projectionmatrix, texture, Bones, lightDirection, ambientColor, diffuseColor, cameraPosition, specularColor, specularPower)){
		return false;
	}

	// Render everything with the shader //
	ShaderRender(RequiredBones, devcont, indexcount);

	return true;
}

bool Leviathan::SkinnedShader::InitShader(ID3D11Device* dev, const wstring &shaderfile){

	HRESULT hr = S_OK;
	ID3D10Blob* Errordumb(NULL);

	ID3D10Blob* Pixelshaderbuffer(NULL);

	// set up compile parameters //
	UINT CompileFlags = ShaderManager::GetShaderCompileFlags();
	
	//D3D10_SHADER_MACRO Shader_Macros[2] = {  {"MAX_TRANSFORMS", maxcount.c_str()}, {NULL, NULL}  };

	// shader macros are hopefully not required here for this to work //
	// pixel shader compile //
	hr = D3DX11CompileFromFile(shaderfile.c_str(), NULL, NULL, "LightPixelShader", "ps_5_0", CompileFlags, 0, NULL, 
						&Pixelshaderbuffer, &Errordumb, NULL);
	if(FAILED(hr)){
		// check for compile error //
		if(Errordumb){

			ShaderManager::PrintShaderError(L"SkinnedShaderSmall", Errordumb);
			Logger::Get()->Error(L"SkinnedShader: InitShader: pixel shader compile failed");
		} else {
			// file was not found //
			Logger::Get()->Error(L"SkinnedShader: InitShader: can't find file: "+shaderfile);
		}
		return false;
	}


	// create all vertex shaders //
	for(int i = 0; i < 6; i++){
		int max_transforms = 0;
		switch(i){
			case 0: max_transforms = MAX_BONES_TINY; break;
			case 1: max_transforms = MAX_BONES_SMALL; break;
			case 2: max_transforms = MAX_BONES_MEDIUM; break;
			case 3: max_transforms = MAX_BONES_LARGE; break;
			case 4: max_transforms = MAX_BONES_HUGE; break;
			case 5: max_transforms = MAX_BONES_MAX; break;
			default:
				// can't reach this //
				__assume(0);
		}

		ID3D10Blob* Vertexshaderbuffer(NULL);


		// needs to define the bone count for the compiler //
		string maxcount = Convert::ToString<int>(max_transforms);
		D3D10_SHADER_MACRO Shader_Macros[2] = {  {"MAX_TRANSFORMS", maxcount.c_str()}, {NULL, NULL}  };

		// Compile Vertex shader //
		hr = D3DX11CompileFromFile(shaderfile.c_str(), Shader_Macros, NULL, "SkinnedVertexShader", "vs_5_0", CompileFlags, 0, NULL, 
			&Vertexshaderbuffer, &Errordumb, NULL);
		if(FAILED(hr)){
			// check for compile error //

			if(Errordumb){

				ShaderManager::PrintShaderError(L"SkinnedShaderSmall", Errordumb);
				Logger::Get()->Error(L"SkinnedShader: InitShader: vertex shader compile failed");
			} else {
				// file was not found //
				Logger::Get()->Error(L"SkinnedShader: InitShader: can't find file: "+shaderfile);
			}
			return false;
		}

		switch(i){
		case 0: hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader_tiny); break;
		case 1: hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader_small); break;
		case 2: hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader_medium); break;
		case 3: hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader_large); break;
		case 4: hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader_huge); break;
		case 5: hr = dev->CreateVertexShader(Vertexshaderbuffer->GetBufferPointer(), Vertexshaderbuffer->GetBufferSize(), NULL, &VertexShader_max); break;
		default:
			// can't reach this //
			__assume(0);
		}
		
		if(FAILED(hr)){

			Logger::Get()->Error(L"SkinnedShader: InitShader: failed to create VertexShader from buffer",(int)hr);
			return false;
		}

		// create input layout, if not already //
		if(Layout == NULL){
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
			if(FAILED(hr) || Layout == NULL){

				Logger::Get()->Error(L"SkinnedShader: InitShader: failed to create layout object",hr);
				return false;
			}
		}


		SAFE_RELEASE(Vertexshaderbuffer);


	}

	// Create the actual shaders from the buffers //
	hr = dev->CreatePixelShader(Pixelshaderbuffer->GetBufferPointer(), Pixelshaderbuffer->GetBufferSize(), NULL, &PixelShader);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShader: InitShader: failed to create PixelShader from buffer",(int)hr);
		return false;
	}


	// release shader buffers
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
		Logger::Get()->Error(L"SkinnedShader: InitShader: failed to create sampler state",hr);
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

		Logger::Get()->Error(L"SkinnedShader: InitShader: failed to create MatrixBuffer",hr);
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

		Logger::Get()->Error(L"SkinnedShader: InitShader: failed to create CameraBuffer",hr);
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

		Logger::Get()->Error(L"SkinnedShader: InitShader: failed to create LightBuffer",hr);
		return false;
	}

	// NEW //
	// Description for Bone transform matrices //
	D3D11_BUFFER_DESC BoneMatriceBufferDesc;
	BoneMatriceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BoneMatriceBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BoneMatriceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BoneMatriceBufferDesc.MiscFlags = 0;
	BoneMatriceBufferDesc.StructureByteStride = 0;

	for(int i = 0; i < 6; i++){
		switch(i){
		case 0:
			{
				// set size //
				BoneMatriceBufferDesc.ByteWidth = sizeof(BoneTransformsBufferTypeTiny);

				// Create //
				hr = dev->CreateBuffer(&BoneMatriceBufferDesc, NULL, &BoneMatriceBuffer_tiny);
			}
		break;
		case 1: 
			{
				// set size //
				BoneMatriceBufferDesc.ByteWidth = sizeof(BoneTransformsBufferTypeSmall);

				// Create //
				hr = dev->CreateBuffer(&BoneMatriceBufferDesc, NULL, &BoneMatriceBuffer_small);
			}
		break;
		case 2: 
			{
				// set size //
				BoneMatriceBufferDesc.ByteWidth = sizeof(BoneTransformsBufferTypeMedium);

				// Create //
				hr = dev->CreateBuffer(&BoneMatriceBufferDesc, NULL, &BoneMatriceBuffer_medium);
			}
		break;
		case 3: 
			{
				// set size //
				BoneMatriceBufferDesc.ByteWidth = sizeof(BoneTransformsBufferTypeLarge);

				// Create //
				hr = dev->CreateBuffer(&BoneMatriceBufferDesc, NULL, &BoneMatriceBuffer_large);
			}
		break;
		case 4: 
			{
				// set size //
				BoneMatriceBufferDesc.ByteWidth = sizeof(BoneTransformsBufferTypeHuge);

				// Create //
				hr = dev->CreateBuffer(&BoneMatriceBufferDesc, NULL, &BoneMatriceBuffer_huge);
			}
		break;
		case 5: 
			{
				// set size //
				BoneMatriceBufferDesc.ByteWidth = sizeof(BoneTransformsBufferTypeMax);

				// Create //
				hr = dev->CreateBuffer(&BoneMatriceBufferDesc, NULL, &BoneMatriceBuffer_max);
			}
		break;
		default:
			// can't reach this //
			__assume(0);
		}
		if(FAILED(hr)){

			Logger::Get()->Error(L"SkinnedShader: InitShader: failed to create BoneMatriceBuffer", (int)hr);
			return false;
		}
	}


	return true;
}


void Leviathan::SkinnedShader::ReleaseShader(){
	// release all allocations //
	SAFE_RELEASE(LightBuffer);
	SAFE_RELEASE(CameraBuffer);
	SAFE_RELEASE(MatrixBuffer);
	SAFE_RELEASE(PixelShader);
	SAFE_RELEASE(SamplerState);
	SAFE_RELEASE(Layout);

	SAFE_RELEASE(VertexShader_tiny);
	//SAFE_RELEASE(Layout_tiny);
	SAFE_RELEASE(BoneMatriceBuffer_tiny);

	SAFE_RELEASE(VertexShader_small);
	//SAFE_RELEASE(Layout_small);
	SAFE_RELEASE(BoneMatriceBuffer_small);

	SAFE_RELEASE(VertexShader_medium);
	//SAFE_RELEASE(Layout_medium);
	SAFE_RELEASE(BoneMatriceBuffer_medium);

	SAFE_RELEASE(VertexShader_large);
	//SAFE_RELEASE(Layout_large);
	SAFE_RELEASE(BoneMatriceBuffer_large);

	SAFE_RELEASE(VertexShader_huge);
	//SAFE_RELEASE(Layout_huge);
	SAFE_RELEASE(BoneMatriceBuffer_huge);

	SAFE_RELEASE(VertexShader_max);
	//SAFE_RELEASE(Layout_max);
	SAFE_RELEASE(BoneMatriceBuffer_max);
}
bool Leviathan::SkinnedShader::SetShaderParams(SHADER_BONECOUNT AmountBones, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture, 
	GameObject::SkeletonRig* Bones, Float3 lightDirection, Float4 ambientColor, Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower)
{

	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// prepare shaders //

	// prepare matrices //
	D3DXMatrixTranspose(&worldmatrix, &worldmatrix);
	D3DXMatrixTranspose(&viewmatrix, &viewmatrix);
	D3DXMatrixTranspose(&projectionmatrix, &projectionmatrix);

	// Unbind old buffers //
	//devcont->VSSetConstantBuffers(0, 20, ShaderManager::NULLBufferBlob);
	//devcont->PSSetConstantBuffers(0, 20, ShaderManager::NULLBufferBlob);

	// lock buffer for writing //
	hr = devcont->Map(MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShader: SetShaderParams: buffer lock failed",hr);
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

		Logger::Get()->Error(L"SkinnedShader: SetShaderParams: buffer lock failed",hr);
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

	switch(AmountBones){
	case SHADER_BONES_TINY:
		{
			if(!(WriteMatricesToBuffer<BoneTransformsPointerTiny, BoneTransformsBufferTypeTiny>(&BoneMatriceBuffer_tiny, devcont, Bones))){
				DEBUG_BREAK;
				return false;
			}
		}
	break;
	case SHADER_BONES_SMALL:
		{
			if(!(WriteMatricesToBuffer<BoneTransformsPointerSmall, BoneTransformsBufferTypeSmall>(&BoneMatriceBuffer_small, devcont, Bones))){
				DEBUG_BREAK;
				return false;
			}
		}
	break;
	case SHADER_BONES_MEDIUM:
		{
			if(!(WriteMatricesToBuffer<BoneTransformsPointerMedium, BoneTransformsBufferTypeMedium>(&BoneMatriceBuffer_medium, devcont, Bones))){
				DEBUG_BREAK;
				return false;
			}
		}
	break;
	case SHADER_BONES_LARGE:
		{
			if(!(WriteMatricesToBuffer<BoneTransformsPointerLarge, BoneTransformsBufferTypeLarge>(&BoneMatriceBuffer_large, devcont, Bones))){
				DEBUG_BREAK;
				return false;
			}
		}
	break;
	case SHADER_BONES_HUGE:
		{
			if(!(WriteMatricesToBuffer<BoneTransformsPointerHuge, BoneTransformsBufferTypeHuge>(&BoneMatriceBuffer_huge, devcont, Bones))){
				DEBUG_BREAK;
				return false;
			}
		}
	break;
	case SHADER_BONES_MAX:
		{
			if(!(WriteMatricesToBuffer<BoneTransformsPointerMax, BoneTransformsBufferTypeMax>(&BoneMatriceBuffer_max, devcont, Bones))){
				DEBUG_BREAK;
				return false;
			}
		}
	break;
	}

	// Lock the light constant buffer so it can be written to.
	hr = devcont->Map(LightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)){

		Logger::Get()->Error(L"SkinnedShader: SetShaderParams: buffer lock failed",hr);
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
void Leviathan::SkinnedShader::ShaderRender(SHADER_BONECOUNT AmountBones, ID3D11DeviceContext* devcont, int indexcount){


	ID3D11VertexShader* VertexShader = NULL;

	// switch to see which layout and which shader needs to be used //
	switch(AmountBones){
	case SHADER_BONES_TINY:
		{
			VertexShader = VertexShader_tiny;
		}
	break;
	case SHADER_BONES_SMALL:
		{
			VertexShader = VertexShader_small;
		}
	break;
	case SHADER_BONES_MEDIUM:
		{
			VertexShader = VertexShader_medium;
		}
	break;
	case SHADER_BONES_LARGE:
		{
			VertexShader = VertexShader_large;
		}
	break;
	case SHADER_BONES_HUGE:
		{
			VertexShader = VertexShader_huge;
		}
	break;
	case SHADER_BONES_MAX:
		{
			VertexShader = VertexShader_max;
		}
	break;
	}
	if(!Layout || !VertexShader){
		ComplainOnce::PrintErrorOnce(L"SkinnedShader_bonecount_not_supported", L"SkinnedShader: Render: invalid bone count enum value :"+
			Convert::IntToWstring((int)AmountBones));
		return;
	}
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

