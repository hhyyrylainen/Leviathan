#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#include "GuiRendBlob.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

Leviathan::RenderingGBlob::RenderingGBlob(const int &relativez, const int &slotid, const bool &hidden) : RelativeZ(relativez), SlotID(slotid), 
	Hidden(hidden)
{
}
Leviathan::RenderingGBlob::~RenderingGBlob(){

}



// ---------------- ColorQuardRendBlob -------------------- //
Leviathan::ColorQuadRendBlob::ColorQuadRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden) : RenderingGBlob(
	relativez, slotid, hidden)
{
	// create new quad //

}
Leviathan::ColorQuadRendBlob::~ColorQuadRendBlob(){

}
void Leviathan::ColorQuadRendBlob::Update(Graphics* graph, const int &relativez, const Float2 &xypos, const Float4 &color, const Float4 &color2, 
	const Float2 &size, int colortranstype, int coordinatetype)
{
	// update z-order //
	RelativeZ = relativez;


	//CQuad->Update(graph->GetRenderer()->GetDeviceContext(), xypos, size, graph->GetWindow()->GetWidth(), graph->GetWindow()->GetHeight(), 
	//	coordinatetype, colortranstype);
}

// ------------------ TextRendBlob ------------------ //
DLLEXPORT Leviathan::TextRendBlob::TextRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden) : 
	RenderingGBlob(relativez, slotid, hidden)
{

}

DLLEXPORT Leviathan::TextRendBlob::~TextRendBlob(){

}

DLLEXPORT void Leviathan::TextRendBlob::Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
	const wstring &font, int coordtype /*= GUI_POSITIONABLE_COORDTYPE_RELATIVE*/, bool fittobox /*= false*/, const Float2 box /*= (Float2)0*/, const 
	float &adjustcutpercentage /*= 0.4f*/)
{
	// update z-order //
	RelativeZ = relativez;

}

