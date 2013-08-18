#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASERENDERABLEBUFFERCONTAINER
#include "BaseRenderableBufferContainer.h"
#endif
#include "RenderingResourceCreator.h"
using namespace Leviathan;
using namespace Leviathan::Rendering;
// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::BaseRenderableBufferContainer::Init(ID3D11Device* device){

	if(!CreateBuffers(device)){
		DEBUG_BREAK;
		return false;
	}

	return Inited = true;
}

DLLEXPORT void Leviathan::Rendering::BaseRenderableBufferContainer::Release(){

	ReleaseBuffers();
	Inited = false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::BaseRenderableBufferContainer::SetBuffersForRendering(ID3D11DeviceContext* devcont, int &indexbuffersize){

	indexbuffersize = GetIndexCount();
	return RenderBuffers(devcont);
}

bool Leviathan::Rendering::BaseRenderableBufferContainer::RenderBuffers(ID3D11DeviceContext* devcont){

	// Set vertex buffer stride and offset.
	unsigned int stride = InputObjectSize; 
	unsigned int offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devcont->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	devcont->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

// ------------------------------------ //
void Leviathan::Rendering::BaseRenderableBufferContainer::ReleaseBuffers(){
	SAFE_RELEASE(VertexBuffer);
	SAFE_RELEASE(IndexBuffer);
}
// ------------------------------------ //
bool Leviathan::Rendering::BaseRenderableBufferContainer::Create1To1IndexBuffer(ID3D11Device* device, const int &icount){

	IndexBuffer = ResourceCreator::GenerateDefaultIndexBuffer(icount);
	return IndexBuffer ? true: false;
}

DLLEXPORT wstring Leviathan::Rendering::BaseRenderableBufferContainer::GetPreferredShaderName(){
	return L"";
}
