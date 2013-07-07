#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_QUAD
#include "ColorQuad.h"
#endif
#include "..\GuiPositionable.h"
using namespace Leviathan;
// ------------------------------------ //
ColorQuad::ColorQuad(){
	// init values to NULL //
	Vertexbuffer = NULL;
	Indexbuffer = NULL;

	Inited = false;
}
ColorQuad::~ColorQuad(){
	if(Inited){
		Release();
	}
}
// ---------------------------------- //
DLLEXPORT bool Leviathan::ColorQuad::Init(ID3D11Device* device, int screenwidth, int screenheight, int colorstyle /*= 1*/){

	Colorstyle = colorstyle;
	// Store the screen size.
	ScreenWidth = screenwidth;
	ScreenHeight = screenheight;


	// Initialize the previous rendering position to negative one.
	PreviousPosX = -1;
	PreviousPosY = -1;
	QuadWidth = -1;
	QuadHeight = -1;

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
	SAFE_RELEASE(Indexbuffer);
	SAFE_RELEASE(Vertexbuffer);

	Inited = false;
}
// ---------------------------------- //
DLLEXPORT bool Leviathan::ColorQuad::Render(ID3D11DeviceContext* devcont, float posx, float posy, int screenwidth, int screenheight, float quadwidth, 
	float quadheight, int Coordtype, int colorstyle /*= 1*/)
{
	// Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
	if(!UpdateBuffers(devcont, posx, posy, screenwidth, screenheight, quadwidth, quadheight, Coordtype, colorstyle)){
		return false;
	}

	// set buffers to renderer for rendering //
	RenderBuffers(devcont);
	return true;
}
// ---------------------------------- //
bool Leviathan::ColorQuad::InitBuffers(ID3D11Device* device){

	HRESULT hr = S_OK;

	// create vertex array //
	VertexType* verticet;
	verticet = new VertexType[COLORQUAD_VERTEXCOUNT];

	// create index array //
	unsigned long* indices;
	indices = new unsigned long[COLORQUAD_VERTEXCOUNT];

	// Initialize vertex array to zeros
	memset(verticet, 0, (sizeof(VertexType) * COLORQUAD_VERTEXCOUNT));

	// Load index array with data
	for(int i = 0; i < COLORQUAD_VERTEXCOUNT; i++){
		indices[i] = i;
	}

	// set buffer descs //
	D3D11_BUFFER_DESC Vertexbufferdesc;
	Vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	Vertexbufferdesc.ByteWidth = sizeof(VertexType) * COLORQUAD_VERTEXCOUNT;
	Vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Vertexbufferdesc.MiscFlags = 0;
	Vertexbufferdesc.StructureByteStride = 0;

	// Give the sub resource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	vertexData.pSysMem = verticet;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// create vertex buffer //
	hr = device->CreateBuffer(&Vertexbufferdesc, &vertexData, &Vertexbuffer);
	if(FAILED(hr)){
		Logger::Get()->Error(L"Failed to init renderingQuad buffers, create vertex buffer failed",0);
		return false;
	}

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC Indexbufferdesc;
	Indexbufferdesc.Usage = D3D11_USAGE_DEFAULT;
	Indexbufferdesc.ByteWidth = sizeof(unsigned long) * COLORQUAD_VERTEXCOUNT;
	Indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Indexbufferdesc.CPUAccessFlags = 0;
	Indexbufferdesc.MiscFlags = 0;
	Indexbufferdesc.StructureByteStride = 0;

	// Give the sub resource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// create index buff //
	hr = device->CreateBuffer(&Indexbufferdesc, &indexData, &Indexbuffer);
	if(FAILED(hr)){
		Logger::Get()->Error(L"Failed to init renderingQuad buffers, create index buffer failed",0);
		return false;
	}

	SAFE_DELETE_ARRAY(verticet);
	SAFE_DELETE_ARRAY(indices);

	return true;
}

// ---------------------------------- //
bool Leviathan::ColorQuad::UpdateBuffers(ID3D11DeviceContext* devcont, float posx, float posy, int screenwidth, int screenheight, float quadwidth, float quadheight, int Coordtype, int colorstyle /*= 1*/){
	// check has something been updated //
	if((posx == PreviousPosX) && (posy == PreviousPosY) && (screenwidth == ScreenWidth) && (screenheight == ScreenHeight) && (quadwidth == QuadWidth) 
		&& (quadheight == QuadHeight) && (colorstyle == Colorstyle))
	{
		// no need to update //
		return true;
	}
	// save new stuff //
	Colorstyle = colorstyle;
	ScreenWidth =  screenwidth;
	ScreenHeight = screenheight;
	QuadWidth = quadwidth;
	QuadHeight = quadheight;
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

	if(Coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE){
		// generate absolute positions //
		float absx = posx*ScreenWidth;
		float absy = posy*ScreenHeight;
		float abswidth = QuadWidth*ScreenWidth;
		float absheight = QuadHeight*ScreenHeight;

		// Calculate the screen coordinates of the left side of the bitmap.
		left = ((ScreenWidth / 2.f)*-1)+absx;

		// Calculate the screen coordinates of the right side of the bitmap.
		right = left + abswidth;

		// Calculate the screen coordinates of the top of the bitmap.
		top = (ScreenHeight / 2.f)-absy;

		// Calculate the screen coordinates of the bottom of the bitmap.
		bottom = top-absheight;

	} else {

		// Calculate the screen coordinates of the left side of the bitmap.
		left = ((ScreenWidth / 2.f) *-1)+posx;

		// Calculate the screen coordinates of the right side of the bitmap.
		right = left+QuadWidth;

		// Calculate the screen coordinates of the top of the bitmap.
		top = (ScreenHeight / 2.f)-posy;

		// Calculate the screen coordinates of the bottom of the bitmap.
		bottom = top-QuadHeight;
	}


	// Create temporary vertex array
	vertices = new VertexType[COLORQUAD_VERTEXCOUNT];
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
	case COLORQUAD_COLOR_STYLE_RIGHTBOTTOMLEFTTOP:
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
	memcpy(verticePTR, (void*)vertices, sizeof(VertexType)*COLORQUAD_VERTEXCOUNT);

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


