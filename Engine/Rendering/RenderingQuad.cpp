#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_QUAD
#include "RenderingQuad.h"
#endif
#include "..\GuiPositionable.h"
#include "RenderingResourceCreator.h"
#include "ShaderDataTypes.h"
using namespace Leviathan;
using namespace Rendering;
// ------------------------------------ //


DLLEXPORT Leviathan::Rendering::RenderingQuad::RenderingQuad() : BaseRenderableBufferContainer(InputDefinitionType, sizeof(VertexType)){
	// set so bizarre enough values that they are always updated //


}

DLLEXPORT Leviathan::Rendering::RenderingQuad::~RenderingQuad(){

}
// ---------------------------------- //
const string Leviathan::Rendering::RenderingQuad::InputDefinitionType = "INPUT:C0:T0";
// ---------------------------------- //
DLLEXPORT bool Leviathan::Rendering::RenderingQuad::Update(ID3D11DeviceContext* devcont, const Float2 &pos, const Float2 &sizes, int screenwidth, 
	int screenheight, int coordtype, int flowstyle /*= 1*/)
{
	// check has something been updated //
	if((Position == pos) && (screenwidth == ScreenWidth) && (screenheight == ScreenHeight) && (QuadSize == sizes) && (UVFlowStyle == flowstyle) &&
		(CoordType == coordtype))
	{
		// no need to update //
		return true;
	}
	// save new variables //
	UVFlowStyle = flowstyle;
	ScreenWidth =  screenwidth;
	ScreenHeight = screenheight;
	QuadSize = sizes;
	Position = pos;

	Float4 leftrighttopbottomlocations(Float4(0));
	Rendering::ResourceCreator::Generate2DCoordinatesFromLocationAndSize(Position, QuadSize, CoordType, leftrighttopbottomlocations);

	// create the quad //
	// don't forget to tell smart ptr that it is an array //
	unique_ptr<VertexType[]> vertices(Rendering::ResourceCreator::GenerateQuadIntoVertexBuffer(Position, QuadSize, COLORQUAD_VERTEXCOUNT, 
		CoordType, UVFlowStyle));
	if(!vertices.get()){
		return false;
	}

	// lock the buffer for writing //
	auto LockedVBuffer = Rendering::ResourceCreator::MapConstantBufferForWriting<VertexType>(devcont, VertexBuffer);
	if(LockedVBuffer.get() == NULL){

		return false;
	}

	// copy data to the vertex buffer //
	memcpy(LockedVBuffer->LockedResourcePtr, (void*)(vertices.get()), sizeof(VertexType)*COLORQUAD_VERTEXCOUNT);

	return true;
}
// ---------------------------------- //
bool RenderingQuad::CreateBuffers(ID3D11Device* device){

	// create vertex buffer //
	VertexBuffer = Rendering::ResourceCreator::GenerateDefaultDynamicDefaultTypeVertexBuffer(COLORQUAD_VERTEXCOUNT);
	if(!VertexBuffer){
		Logger::Get()->Error(L"Failed to init renderingQuad buffers, create vertex buffer failed");
		return false;
	}
	// base class can create basic index buffer //
	if(!Create1To1IndexBuffer(device, COLORQUAD_VERTEXCOUNT)){

		return false;
	}

	return true;
}
// ---------------------------------- //


