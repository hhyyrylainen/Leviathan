#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_QUAD
#include "ColorQuad.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ColorQuad::ColorQuad(){
	// init values to NULL //
	Vertexbuffer = NULL;
	Indexbuffer = NULL;
	//pTexture = NULL;

	VertexCount = 0;
	IndexCount = 0;
	Inited = false;
}
ColorQuad::~ColorQuad(){
	if(Inited){
		this->Release();
	}
}

bool ColorQuad::Init(ID3D11Device* device,  int screenwidth, int screenheight, int quadwidth, int quadheight, int colorstyle){

	Colorstyle = colorstyle;
	// Store the screen size.
	ScreenWidth = screenwidth;
	ScreenHeight = screenheight;

	// Store the size in pixels that this bitmap should be rendered at.
	BitmapWidth = quadwidth;
	BitmapHeight = quadheight;

	// Initialize the previous rendering position to negative one.
	PreviousPosX = -1;
	PreviousPosY = -1;

	// init buffers //
	if(!InitBuffers(device)){
		Logger::Get()->Error(L"Failed to init Quad, init buffers failed");
		return false;
	}

	Inited = true;

	return true;
}
void ColorQuad::Release(){
	// release all objects //

	//SAFE_RELEASEDEL(pTexture);
	SAFE_RELEASE(Indexbuffer);
	SAFE_RELEASE(Vertexbuffer);

	Inited = false;
}
void ColorQuad::Render(ID3D11DeviceContext* devcont, int posx, int posy, int screenwidth, int screenheight, int quadwidth, int quadheight, bool absolute, int colorstyle){
	// Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
	if(!UpdateBuffers(devcont, posx, posy, screenwidth, screenheight, quadwidth, quadheight, absolute, colorstyle)){
		return;
	}

	// set buffers to renderer for rendering //
	RenderBuffers(devcont);
}
int ColorQuad::GetIndexCount(){
	return IndexCount;
}

// ---------------------------------- //

bool ColorQuad::InitBuffers(ID3D11Device* device){
		
		
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
		Logger::Get()->Error(L"Failed to init renderingQuad buffers, create vertex buffer failed",0);
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
		Logger::Get()->Error(L"Failed to init renderingQuad buffers, create index buffer failed",0);
		return false;
	}

	SAFE_DELETE_ARRAY(verticet);
	SAFE_DELETE_ARRAY(indices);

	return true;
}

// ---------------------------------- //
bool ColorQuad::UpdateBuffers(ID3D11DeviceContext* devcont, int posx, int posy, int screenwidth, int screenheight, int quadwidth, int quadheight, bool absolute, int colorstyle){
	// check has position been updated //
	if((posx == PreviousPosX) && (posy == PreviousPosY) && (screenwidth == ScreenWidth) && (screenheight == ScreenHeight) && (quadwidth == BitmapWidth) && (quadheight == BitmapHeight) && (colorstyle == Colorstyle)){
		// no need to update //
		return true;
	}

	//DEBUG_OUTPUT(L"ColorQuad: quad updated, old "+Convert::IntToWstring(ScreenWidth)+L","+Convert::IntToWstring(ScreenHeight)+L" new: "+
	//	Convert::IntToWstring(screenwidth)+L","+Convert::IntToWstring(screenheight)+L"\n");

	// save new pos //
	Colorstyle = colorstyle;
	ScreenWidth =  screenwidth;
	ScreenHeight = screenheight;
	BitmapWidth = quadwidth;
	BitmapHeight = quadheight;
	PreviousPosX = posx;
	PreviousPosY = posy;

	VertexType* vertices;
	D3D11_MAPPED_SUBRESOURCE mappedresource;

	VertexType* verticePTR;
	HRESULT hr = S_OK;

	float left;
	float right;
	float top;
	float bottom;

	if(absolute){
		// Calculate the screen coordinates of the left side of the bitmap.
		left = (float)((ScreenWidth / 2) * -1) + (float)posx;

		// Calculate the screen coordinates of the right side of the bitmap.
		right = left + (float)BitmapWidth;

		// Calculate the screen coordinates of the top of the bitmap.
		top = (float)(ScreenHeight / 2) - (float)posy;

		// Calculate the screen coordinates of the bottom of the bitmap.
		bottom = top - (float)BitmapHeight;
	} else {
		// generate absolute posses //
		int absx;
		int absy;
		int abswidth;
		int absheight;

		// important scale translations //
		// input coordinates are now in promilles (
		//absx = ((float)posx/ResolutionScaling::GetXScaleFactor())*ScreenWidth;
		//absy = ((float)posy/ResolutionScaling::GetYScaleFactor())*ScreenHeight;
		//abswidth = BitmapWidth*((float)ScreenWidth/ResolutionScaling::GetXScaleFactor());
		//absheight = BitmapHeight*((float)ScreenHeight/ResolutionScaling::GetYScaleFactor());
		absx = (int)(((float)posx/ResolutionScaling::GetPromilleFactor())*ScreenWidth);
		absy = (int)(((float)posy/ResolutionScaling::GetPromilleFactor())*ScreenHeight);
		abswidth = (int)(ScreenWidth*((float)BitmapWidth/ResolutionScaling::GetPromilleFactor()));
		absheight = (int)(ScreenHeight*((float)BitmapHeight/ResolutionScaling::GetPromilleFactor()));

		//Logger::Get()->Info(L"ColorQuad: resized to "+Convert::IntToWstring(absx)+L","+Convert::IntToWstring(absy), false);


		// Calculate the screen coordinates of the left side of the bitmap.
		left = (float)((ScreenWidth / 2) * -1) + (float)absx;

		// Calculate the screen coordinates of the right side of the bitmap.
		// important scale function here //
		right = left + (float)abswidth;

		// Calculate the screen coordinates of the top of the bitmap.
		top = (float)(ScreenHeight / 2) - (float)absy;

		// Calculate the screen coordinates of the bottom of the bitmap.
		bottom = top - (float)absheight;
	}


	// Create temporary vertex array
	vertices = new VertexType[VertexCount];
	if(!vertices){
		return false;
	}
	// style //

	// base style; COLOR_QUAD_COLOR_STYLE_LEFT_TOP_RIGHT_BOTTOM //
	D3DXVECTOR2 topleft = D3DXVECTOR2(0.0f, 0.01f);
	D3DXVECTOR2 bottomright = D3DXVECTOR2(1.0f, 1.0f);
	D3DXVECTOR2 bottomleft = D3DXVECTOR2(0.0f, 1.0f);
	D3DXVECTOR2 topright = D3DXVECTOR2(1.0f, 0.0f);

	switch(Colorstyle){
	default:
		break;
	case COLOR_QUAD_COLOR_STYLE_RIGHT_BOTTOM_LEFT_TOP:
		{
			// opposite flow direction //
			topleft = D3DXVECTOR2(1.0f, 1.0f);
			bottomright = D3DXVECTOR2(0.0f, 0.0f);
			bottomleft = D3DXVECTOR2(1.0f, 0.0f);
			topright = D3DXVECTOR2(0.0f, 1.0f);
		}
	break;
	}


	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = D3DXVECTOR3(left, top, 0.0f);  // Top left.
	vertices[0].colorcoordinate = topleft;

	vertices[1].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].colorcoordinate = bottomright;

	vertices[2].position = D3DXVECTOR3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].colorcoordinate = bottomleft;

	// Second triangle.
	vertices[3].position = D3DXVECTOR3(left, top, 0.0f);  // Top left.
	vertices[3].colorcoordinate = topleft;

	vertices[4].position = D3DXVECTOR3(right, top, 0.0f);  // Top right.
	vertices[4].colorcoordinate = topright;

	vertices[5].position = D3DXVECTOR3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].colorcoordinate = bottomright;	

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

void ColorQuad::RenderBuffers(ID3D11DeviceContext* devcont){
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


