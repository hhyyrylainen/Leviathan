#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_BITMAP
#include "Bitmap.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
RenderingBitmap::RenderingBitmap(){
	// init values to NULL //
	Vertexbuffer = NULL;
	Indexbuffer = NULL;
	pTexture = NULL;

	VertexCount = 0;
	IndexCount = 0;
	Inited = false;
}
RenderingBitmap::~RenderingBitmap(){
	if(Inited){
		this->Release();
	}
}

bool RenderingBitmap::Init(ID3D11Device* device,  int screenwidth, int screenheight, wstring texture, wstring texture2, int bitmapwidth, int bitmapheight){

	// Store the screen size.
	ScreenWidth = screenwidth;
	ScreenHeight = screenheight;

	// Store the size in pixels that this bitmap should be rendered at.
	BitmapWidth = bitmapwidth;
	BitmapHeight = bitmapheight;

	// Initialize the previous rendering position to negative one.
	PreviousPosX = -1;
	PreviousPosY = -1;

	// init buffers //
	if(!InitBuffers(device)){
		Logger::Get()->Error(L"Failed to init Bitmap, init buffers failed");
		return false;
	}

	// Load the texture for this model.
	if(!LoadTextures(device, texture, texture2)){
		Logger::Get()->Error(L"Failed to init Bitmap, init Texture failed",0);
		return false;
	}
	Inited = true;

	return true;
}
void RenderingBitmap::Release(){
	// release all objects //

	SAFE_RELEASEDEL(pTexture);
	SAFE_RELEASE(Indexbuffer);
	SAFE_RELEASE(Vertexbuffer);

	Inited = false;
}
void RenderingBitmap::Render(ID3D11DeviceContext* devcont, int posx, int posy){
	// Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
	if(!UpdateBuffers(devcont, posx, posy)){
		return;
	}

	// set buffers to renderer for rendering //
	RenderBuffers(devcont);
}
int RenderingBitmap::GetIndexCount(){
	return IndexCount;
}

ID3D11ShaderResourceView** RenderingBitmap::GetTextureArray(){
	return pTexture->GetTextureArray();
}
ID3D11ShaderResourceView* RenderingBitmap::GetTexture(){
	return pTexture->GetTexture();
}
// ---------------------------------- //

bool RenderingBitmap::InitBuffers(ID3D11Device* device){
		
		
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT hr = S_OK;

	VertexCount = 6;
	IndexCount = VertexCount;


	// create vertex array //
	VertexType* verticet;
	verticet = new VertexType[VertexCount];

	// create index array //
	unsigned long* indices;
	indices = new unsigned long[IndexCount];




	// Initialize vertex array to zeros
	memset(verticet, 0, (sizeof(VertexType) * VertexCount));

	// Load index array with data
	for(int i = 0; i < IndexCount; i++){

		indices[i] = i;
	}

	// set buffer descs //
	D3D11_BUFFER_DESC Vertexbufferdesc;
	Vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	Vertexbufferdesc.ByteWidth = sizeof(VertexType) * VertexCount;
	Vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Vertexbufferdesc.MiscFlags = 0;
	Vertexbufferdesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = verticet;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// create vertex buffer //
	hr = device->CreateBuffer(&Vertexbufferdesc, &vertexData, &Vertexbuffer);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"Failed to init RenderingBitmap buffers, create vertex buffer failed",0);
		return false;
	}

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC Indexbufferdesc;
	Indexbufferdesc.Usage = D3D11_USAGE_DEFAULT;
	Indexbufferdesc.ByteWidth = sizeof(unsigned long) * IndexCount;
	Indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Indexbufferdesc.CPUAccessFlags = 0;
	Indexbufferdesc.MiscFlags = 0;
	Indexbufferdesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// create index buff //
	hr = device->CreateBuffer(&Indexbufferdesc, &indexData, &Indexbuffer);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"Failed to init RenderingBitmap buffers, create index buffer failed",0);
		return false;
	}

	SAFE_DELETE_ARRAY(verticet);
	SAFE_DELETE_ARRAY(indices);

	return true;
}

bool RenderingBitmap::LoadTextures(ID3D11Device* dev, wstring filename, wstring filename2){

	// Create the texture object.
	pTexture = new TextureArray;
	if(!pTexture){
		return false;
	}

	// Initialize the texture object.
	if(!pTexture->Init(dev, filename, filename2)){
		return false;
	}

	return true;
}




// ---------------------------------- //
bool RenderingBitmap::UpdateBuffers(ID3D11DeviceContext* devcont, int x, int y){
	// check has position been updated //
	if((x == PreviousPosX) && (y == PreviousPosY)){
		// no need to update //
		return true;
	}
	// save new pos //
	PreviousPosX = x;
	PreviousPosY = y;

	VertexType* vertices;
	D3D11_MAPPED_SUBRESOURCE mappedresource;

	VertexType* verticePTR;
	HRESULT hr = S_OK;

	// Calculate the screen coordinates of the left side of the bitmap.
	float left = (float)((ScreenWidth / 2) * -1) + (float)x;

	// Calculate the screen coordinates of the right side of the bitmap.
	float right = left + (float)BitmapWidth;

	// Calculate the screen coordinates of the top of the bitmap.
	float top = (float)(ScreenHeight / 2) - (float)y;

	// Calculate the screen coordinates of the bottom of the bitmap.
	float bottom = top - (float)BitmapHeight;


	// Create temporary vertex array
	vertices = new VertexType[VertexCount];
	if(!vertices){
		return false;
	}

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = D3DXVECTOR3(left, top, 0.0f);  // Top left.
	vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);

	vertices[1].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texture = D3DXVECTOR2(1.0f, 1.0f);

	vertices[2].position = D3DXVECTOR3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texture = D3DXVECTOR2(0.0f, 1.0f);

	// Second triangle.
	vertices[3].position = D3DXVECTOR3(left, top, 0.0f);  // Top left.
	vertices[3].texture = D3DXVECTOR2(0.0f, 0.0f);

	vertices[4].position = D3DXVECTOR3(right, top, 0.0f);  // Top right.
	vertices[4].texture = D3DXVECTOR2(1.0f, 0.0f);

	vertices[5].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = D3DXVECTOR2(1.0f, 1.0f);	

	// Lock the vertex buffer so it can be written to.
	hr = devcont->Map(Vertexbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedresource);
	if(FAILED(hr)){

		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticePTR = (VertexType*)mappedresource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticePTR, (void*)vertices, (sizeof(VertexType) * VertexCount));

	// Unlock the vertex buffer.
	devcont->Unmap(Vertexbuffer, 0);

	// Release vertex array
	SAFE_DELETE_ARRAY(vertices);

	return true;

}


void RenderingBitmap::RenderBuffers(ID3D11DeviceContext* devcont){
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
	
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devcont->IASetVertexBuffers(0, 1, &Vertexbuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(Indexbuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	devcont->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool RenderingBitmap::UsingMultitextures(){
	return pTexture->IsMultiTexture();
}

