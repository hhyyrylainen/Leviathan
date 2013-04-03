#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_BUMPMODEL
#include "BumpModel.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
BumpModel::BumpModel(){
	VertexBuffer = NULL;
	IndexBuffer = NULL;
	Model = NULL;
	ColorandMap = NULL;
	//ColorTexture = NULL;
	//NormalMapTexture = NULL;

	VertexCount = 0;
	IndexCount = 0;
	Inited = false;

	mtype = MBUMPTYPE;
}

BumpModel::~BumpModel(){
	if(Inited){
		this->Release();
	}
}
// ------------------------------------ //
bool BumpModel::Init(ID3D11Device* dev, wstring modelfile, wstring texturefile1, wstring texturefile2){

	// Load model data

	if(!LoadModel(modelfile)){
		Logger::Get()->Error(L"Failed to init BumbModel: loading model from file: "+modelfile);
		return false;
	}

	// Calculate the tangent and binormal vectors for the model.
	CalculateModelVectors();

	// Initialize vertex and index buffers
	if(!InitBuffers(dev)){
		Logger::Get()->Error(L"Failed to init BumbModel: error initing buffers");
		return false;
	}

	// Load textures for model
	if(!LoadTextures(dev, texturefile1, texturefile2)){
		Logger::Get()->Error(L"Failed to init BumbModel: error loading textures");
		return false;
	}
	Inited = true;

	return true;
}
void BumpModel::Release(){
	// Call release functions //
	// release and delete textures //
	//SAFE_RELEASEDEL(ColorTexture);
	//SAFE_RELEASEDEL(NormalMapTexture);
	SAFE_RELEASE(ColorandMap);

	// release buffers //
	SAFE_RELEASE(IndexBuffer);
	SAFE_RELEASE(VertexBuffer);

	// delete model data //
	SAFE_DELETE(Model);

	Inited = false;

}
// ------------------------------------ //
void BumpModel::Render(ID3D11DeviceContext* devcont){
	// Put buffers to pipeline
	RenderBuffers(devcont);
}
int BumpModel::GetIndexCount(){
	return IndexCount;
}

ID3D11ShaderResourceView* BumpModel::GetColorTexture(){
	return ColorandMap->GetTextureArray()[0];
	//return ColorTexture->GetTexture();
}

ID3D11ShaderResourceView* BumpModel::GetNormalMapTexture(){
	return ColorandMap->GetTextureArray()[1];
	//return NormalMapTexture->GetTexture();
}
// ------------------------------------ //
bool BumpModel::LoadModel(wstring modelfile){

	// Open the model file.  If it could not open the file then exit.
	wifstream reader;
	reader.open(modelfile);
	if(!reader.good()){
		Logger::Get()->Error(L"Can't LoadModel, file could not be opened or is empty"+modelfile);
		return false;
	}

	// Read up to the value of vertex count.
	wchar_t readchar;
	reader.get(readchar);
	while(readchar != L':'){
		reader.get(readchar);
	}

	// Read in the vertex count.
	reader >> VertexCount;

	// Set the number of indices to be the same as the vertex count.
	IndexCount = VertexCount;

	// Create the model using the vertex count that was read in.
	Model = new ModelType[VertexCount];

	if(!Model){
		return false;
	}

	// Read up to the beginning of the data.
	reader.get(readchar);
	while(readchar != ':'){
		reader.get(readchar);
	}
	reader.get(readchar);
	reader.get(readchar);

	// Read in the vertex data.
	for(int i = 0; i < VertexCount; i++){

		reader >> Model[i].x >> Model[i].y >> Model[i].z;
		reader >> Model[i].tu >> Model[i].tv;
		reader >> Model[i].nx >> Model[i].ny >> Model[i].nz;
	}

	// Close the model file.
	reader.close();

	return true;
}

bool BumpModel::InitBuffers(ID3D11Device* dev){
	
	HRESULT hr = S_OK;
	
	
	

	VertexType* vertices;
	// Create the vertex array.
	vertices = new VertexType[VertexCount];
	if(!vertices){
		return false;
	}

	// Create the index array.
	unsigned long* indices;
	indices = new unsigned long[IndexCount];
	if(!indices){
		return false;
	}

	// Load the vertex array and index array with data.
	for(int i = 0; i < VertexCount; i++){

		vertices[i].position = D3DXVECTOR3(Model[i].x, Model[i].y, Model[i].z);
		vertices[i].texture = D3DXVECTOR2(Model[i].tu, Model[i].tv);
		vertices[i].normal = D3DXVECTOR3(Model[i].nx, Model[i].ny, Model[i].nz);
		vertices[i].tangent = D3DXVECTOR3(Model[i].tx, Model[i].ty, Model[i].tz);
		vertices[i].binormal = D3DXVECTOR3(Model[i].bx, Model[i].by, Model[i].bz);

		indices[i] = i;
	}
	
	// Set up the description of the static vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * VertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    hr = dev->CreateBuffer(&vertexBufferDesc, &vertexData, &VertexBuffer);
	if(FAILED(hr)){
		Logger::Get()->Error(L"Can't InitBuffers, create vertexbuffer failed");
		return false;
	}

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * IndexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	hr = dev->CreateBuffer(&indexBufferDesc, &indexData, &IndexBuffer);
	if(FAILED(hr)){
		Logger::Get()->Error(L"Can't InitBuffers, create indexbuffer failed");
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);

	return true;
}

bool BumpModel::LoadTextures(ID3D11Device* dev, wstring texturefile1, wstring texturefile2){

	// Create the color texture object.
	ColorandMap = new TextureArray;
	if(!ColorandMap){
		return false;
	}

	ColorandMap->Init(dev, texturefile1, texturefile2);

	//ColorTexture = new Texture;
	//if(!ColorTexture){
	//	return false;
	//}

	//// Initialize the color texture object.
	//if(!ColorTexture->Init(dev, texturefile1)){
	//	return false;
	//}

	//// Create the normal map texture object.
	//NormalMapTexture = new Texture;
	//if(!NormalMapTexture){
	//	return false;
	//}

	//// Initialize the normal map texture object.
	//if(!NormalMapTexture->Init(dev, texturefile2)){
	//	return false;
	//}

	return true;
}
// ------------------------------------ //
void BumpModel::RenderBuffers(ID3D11DeviceContext* deviceContext){

	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}
// ------------------------------------ //
void BumpModel::CalculateModelVectors()
{
	TempVertexType vertex1, vertex2, vertex3;
	VectorType tangent, binormal;


	// Calculate the number of faces in the model.
	int faceCount = VertexCount / 3;

	// Initialize the index to the model data.
	int index = 0;

	// Go through all the faces and calculate the tangent, binormal, and normal vectors.
	for(int i = 0; i < faceCount; i++){
		// Get the three vertices for this face from the model.
		vertex1.x = Model[index].x;
		vertex1.y = Model[index].y;
		vertex1.z = Model[index].z;
		vertex1.tu = Model[index].tu;
		vertex1.tv = Model[index].tv;
		vertex1.nx = Model[index].nx;
		vertex1.ny = Model[index].ny;
		vertex1.nz = Model[index].nz;
		index++;

		vertex2.x = Model[index].x;
		vertex2.y = Model[index].y;
		vertex2.z = Model[index].z;
		vertex2.tu = Model[index].tu;
		vertex2.tv = Model[index].tv;
		vertex2.nx = Model[index].nx;
		vertex2.ny = Model[index].ny;
		vertex2.nz = Model[index].nz;
		index++;

		vertex3.x = Model[index].x;
		vertex3.y = Model[index].y;
		vertex3.z = Model[index].z;
		vertex3.tu = Model[index].tu;
		vertex3.tv = Model[index].tv;
		vertex3.nx = Model[index].nx;
		vertex3.ny = Model[index].ny;
		vertex3.nz = Model[index].nz;
		index++;

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Store the normal, tangent, and binormal for this face back in the model structure.
		Model[index-1].tx = tangent.x;
		Model[index-1].ty = tangent.y;
		Model[index-1].tz = tangent.z;
		Model[index-1].bx = binormal.x;
		Model[index-1].by = binormal.y;
		Model[index-1].bz = binormal.z;

		Model[index-2].tx = tangent.x;
		Model[index-2].ty = tangent.y;
		Model[index-2].tz = tangent.z;
		Model[index-2].bx = binormal.x;
		Model[index-2].by = binormal.y;
		Model[index-2].bz = binormal.z;

		Model[index-3].tx = tangent.x;
		Model[index-3].ty = tangent.y;
		Model[index-3].tz = tangent.z;
		Model[index-3].bx = binormal.x;
		Model[index-3].by = binormal.y;
		Model[index-3].bz = binormal.z;
	}
}

void BumpModel::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3,
										  VectorType& tangent, VectorType& binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];


	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	float den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	float length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));
			
	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));
			
	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;
}