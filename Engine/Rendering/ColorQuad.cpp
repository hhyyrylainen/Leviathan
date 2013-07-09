#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_QUAD
#include "ColorQuad.h"
#endif
#include "..\GuiPositionable.h"
#include "RenderingResourceCreator.h"
using namespace Leviathan;
// ------------------------------------ //
ColorQuad::ColorQuad() : VertexBuffer(NULL), IndexBuffer(NULL) {
	// init values to NULL //
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
	SAFE_RELEASE(IndexBuffer);
	SAFE_RELEASE(VertexBuffer);

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

	// create vertex buffer //
	VertexBuffer = Rendering::ResourceCreator::GenerateDefaultDynamicDefaultTypeVertexBuffer(COLORQUAD_VERTEXCOUNT);
	if(!VertexBuffer){
		Logger::Get()->Error(L"Failed to init renderingQuad buffers, create vertex buffer failed",0);
		return false;
	}

	// create index buffer //
	IndexBuffer = Rendering::ResourceCreator::GenerateDefaultIndexBuffer(COLORQUAD_VERTEXCOUNT);
	if(!IndexBuffer){
		Logger::Get()->Error(L"Failed to init renderingQuad buffers, create index buffer failed",0);
		return false;
	}

	return true;
}

// ---------------------------------- //
bool Leviathan::ColorQuad::UpdateBuffers(ID3D11DeviceContext* devcont, float posx, float posy, int screenwidth, int screenheight, float quadwidth, 
	float quadheight, int Coordtype, int colorstyle /*= 1*/)
{
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

	Float4 leftrighttopbottomlocations(Float4(0));

	Float2 coordinates(posx, posy);
	Float2 sizes(quadwidth, quadheight);

	Rendering::ResourceCreator::Generate2DCoordinatesFromLocationAndSize(coordinates, sizes, Coordtype, leftrighttopbottomlocations);

	// create the quad //
	VertexType* vertices = Rendering::ResourceCreator::GenerateQuadIntoVertexBuffer(coordinates, sizes, COLORQUAD_VERTEXCOUNT, Coordtype, colorstyle);
	if(!vertices){
		return false;
	}

	// Lock the vertex buffer so it can be written to.
	D3D11_MAPPED_SUBRESOURCE mappedresource;

	HRESULT hr = devcont->Map(VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedresource);
	if(FAILED(hr)){

		SAFE_DELETE_ARRAY(vertices);
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	VertexType* verticePTR = (VertexType*)mappedresource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticePTR, (void*)vertices, sizeof(VertexType)*COLORQUAD_VERTEXCOUNT);

	// Unlock the vertex buffer.
	devcont->Unmap(VertexBuffer, 0);

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
	devcont->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	devcont->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


