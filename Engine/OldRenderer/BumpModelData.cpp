#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BUMPMODELDATA
#include "BumpModelData.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
DLLEXPORT Leviathan::GameObject::BumpModelData::BumpModelData(){
	Type = MODELOBJECT_MODEL_TYPE_BUMP;
	BumbModel = NULL;

	IndexBuffer = NULL;
	VertexBuffer = NULL;
}

DLLEXPORT Leviathan::GameObject::BumpModelData::~BumpModelData(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameObject::BumpModelData::ReleaseModel(){
	SAFE_RELEASE(IndexBuffer);
	SAFE_RELEASE(VertexBuffer);
	SAFE_DELETE_ARRAY(BumbModel);
}

DLLEXPORT bool Leviathan::GameObject::BumpModelData::InitBuffers(ID3D11Device* device){
	HRESULT hr = S_OK;

	// Create the vertex array.
	BumpVertexType* vertices = new BumpVertexType[VertexCount];
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

		vertices[i].position = D3DXVECTOR3(BumbModel[i].x, BumbModel[i].y, BumbModel[i].z);
		vertices[i].texture = D3DXVECTOR2(BumbModel[i].tu, BumbModel[i].tv);
		vertices[i].normal = D3DXVECTOR3(BumbModel[i].nx, BumbModel[i].ny, BumbModel[i].nz);
		vertices[i].tangent = D3DXVECTOR3(BumbModel[i].tx, BumbModel[i].ty, BumbModel[i].tz);
		vertices[i].binormal = D3DXVECTOR3(BumbModel[i].bx, BumbModel[i].by, BumbModel[i].bz);

		indices[i] = i;
	}

	// Set up the description of the static vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(BumpVertexType) * VertexCount;
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
	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &VertexBuffer);
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
	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &IndexBuffer);
	if(FAILED(hr)){
		Logger::Get()->Error(L"Can't InitBuffers, create indexbuffer failed");
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameObject::BumpModelData::RenderBuffers(ID3D11DeviceContext* devcont){
	// Set vertex buffer stride and offset.
	unsigned int stride = sizeof(BumpVertexType); 
	unsigned int offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devcont->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	devcont->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameObject::BumpModelData::LoadRenderModel(wstring* file){
	// this needs a rewrite //




	// calculate vectors after loading //
	BumpCalculateModelVectors();

	return true;
}

// ------------------------------------ //
void Leviathan::GameObject::BumpModelData::BumpCalculateTangentBinormal(BumpModelVertexType vertex1, BumpModelVertexType vertex2, BumpModelVertexType vertex3, ModelVertexFormat& tangent, ModelVertexFormat& binormal){
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

void Leviathan::GameObject::BumpModelData::BumpCalculateModelVectors(){
	BumpModelVertexType vertex1, vertex2, vertex3;
	ModelVertexFormat tangent, binormal;


	// Calculate the number of faces in the model.
	int faceCount = VertexCount / 3;

	// Initialize the index to the model data.
	int index = 0;

	// Go through all the faces and calculate the tangent, binormal, and normal vectors.
	for(int i = 0; i < faceCount; i++){
		// Get the three vertices for this face from the model.
		vertex1.x = BumbModel[index].x;
		vertex1.y = BumbModel[index].y;
		vertex1.z = BumbModel[index].z;
		vertex1.tu = BumbModel[index].tu;
		vertex1.tv = BumbModel[index].tv;
		vertex1.nx = BumbModel[index].nx;
		vertex1.ny = BumbModel[index].ny;
		vertex1.nz = BumbModel[index].nz;
		index++;

		vertex2.x = BumbModel[index].x;
		vertex2.y = BumbModel[index].y;
		vertex2.z = BumbModel[index].z;
		vertex2.tu = BumbModel[index].tu;
		vertex2.tv = BumbModel[index].tv;
		vertex2.nx = BumbModel[index].nx;
		vertex2.ny = BumbModel[index].ny;
		vertex2.nz = BumbModel[index].nz;
		index++;

		vertex3.x = BumbModel[index].x;
		vertex3.y = BumbModel[index].y;
		vertex3.z = BumbModel[index].z;
		vertex3.tu = BumbModel[index].tu;
		vertex3.tv = BumbModel[index].tv;
		vertex3.nx = BumbModel[index].nx;
		vertex3.ny = BumbModel[index].ny;
		vertex3.nz = BumbModel[index].nz;
		index++;

		// Calculate the tangent and binormal of that face.
		BumpCalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Store the normal, tangent, and binormal for this face back in the model structure.
		BumbModel[index-1].tx = tangent.x;
		BumbModel[index-1].ty = tangent.y;
		BumbModel[index-1].tz = tangent.z;
		BumbModel[index-1].bx = binormal.x;
		BumbModel[index-1].by = binormal.y;
		BumbModel[index-1].bz = binormal.z;

		BumbModel[index-2].tx = tangent.x;
		BumbModel[index-2].ty = tangent.y;
		BumbModel[index-2].tz = tangent.z;
		BumbModel[index-2].bx = binormal.x;
		BumbModel[index-2].by = binormal.y;
		BumbModel[index-2].bz = binormal.z;

		BumbModel[index-3].tx = tangent.x;
		BumbModel[index-3].ty = tangent.y;
		BumbModel[index-3].tz = tangent.z;
		BumbModel[index-3].bx = binormal.x;
		BumbModel[index-3].by = binormal.y;
		BumbModel[index-3].bz = binormal.z;
	}
}

DLLEXPORT  bool Leviathan::GameObject::BumpModelData::WriteToFile(const wstring& file, bool InBinary /*= false*/){
	throw std::exception("The method or operation is not implemented.");
}

DLLEXPORT  wstring Leviathan::GameObject::BumpModelData::GetModelTypeName(){
	throw std::exception("The method or operation is not implemented.");
}




