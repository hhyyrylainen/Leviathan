#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#include "GuiRendBlob.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "RenderingQuad.h"
#include "Engine.h"
#include ".\Rendering\TextRenderer.h"

Leviathan::RenderingGBlob::RenderingGBlob(const int &relativez, const int &slotid, const bool &hidden) : SHRender(NULL), 
	RelativeZ(relativez), SlotID(slotid), Hidden(hidden)
{
}
Leviathan::RenderingGBlob::~RenderingGBlob(){
	SAFE_DELETE(SHRender);
}

void Leviathan::RenderingGBlob::SetShaderMatrixBuffers(RenderingPassInfo* pass){
	// copy matrices //
	SHRender->GetBaseMatrixBufferData()->ProjectionMatrix = pass->GetProjectionMatrix();
	SHRender->GetBaseMatrixBufferData()->ViewMatrix = pass->GetViewMatrix();
	SHRender->GetBaseMatrixBufferData()->WorldMatrix = pass->GetWorldMatrix();
}

// ---------------- ColorQuardRendBlob -------------------- //
Leviathan::ColorQuadRendBlob::ColorQuadRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden) : RenderingGBlob(
	relativez, slotid, hidden)
{
	// create new quad //
	CQuad = new Rendering::RenderingQuad();
	if(!CQuad->Init(graph->GetRenderer()->GetDevice())){

		throw ExceptionNULLPtr(L" can't init RenderingQuad", false, __WFUNCTION__, NULL);
	}
}
Leviathan::ColorQuadRendBlob::~ColorQuadRendBlob(){
	SAFE_RELEASEDEL(CQuad);
}
void Leviathan::ColorQuadRendBlob::Update(Graphics* graph, const int &relativez, const Float2 &xypos, const Float4 &color, const Float4 &color2, 
	const Float2 &size, int colortranstype, int coordinatetype)
{
	// update z-order //
	RelativeZ = relativez;

	// update colours buffer //
	EnsureShaderRenderTask();

	SHRender->GetColourBufferTwo()->Colour1 = (D3DXVECTOR4)color;
	SHRender->GetColourBufferTwo()->Colour2 = (D3DXVECTOR4)color2;

	CQuad->Update(graph->GetRenderer()->GetDeviceContext(), xypos, size, graph->GetWindow()->GetWidth(), graph->GetWindow()->GetHeight(), 
		coordinatetype, colortranstype);
}


DLLEXPORT Rendering::BaseRenderableBufferContainer* Leviathan::ColorQuadRendBlob::GetRenderingBuffers(Graphics* graph){
	// RenderingQuad is inherited from the base so we can just return it //
	return CQuad;
}

DLLEXPORT ShaderRenderTask* Leviathan::ColorQuadRendBlob::GetShaderParameters(Graphics* graph, RenderingPassInfo* pass){
	// update matrix buffers //
	EnsureShaderRenderTask();
	SetShaderMatrixBuffers(pass);
	return SHRender;
}

void Leviathan::ColorQuadRendBlob::EnsureShaderRenderTask(){
	// create new if NULL //
	if(SHRender == NULL){

		SHRender = new ShaderRenderTask();
		// set required parts //
		SHRender->SetBaseMatrixBuffer(new BaseMatrixBufferData())->SetColourBufferTwo(new TwoColorBufferData());
	}
}

// ---------------- BasicTextRendBlob -------------------- //
Leviathan::BasicTextRendBlob::BasicTextRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden) : 
	RenderingGBlob(relativez, slotid, hidden)
{	
	// get unique id for text //
	TextID = IDFactory::GetID();
	// copy text renderer ptr //
	TRenderer = graph->GetTextRenderer();
}

Leviathan::BasicTextRendBlob::~BasicTextRendBlob(){
	// needs to destroy the text //
	TRenderer->ReleaseText(TextID);
}
// ------------------------------------ //
void Leviathan::BasicTextRendBlob::Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
	const wstring &font, int coordtype)
{
	// update z-order //
	RelativeZ = relativez;
	
	// set colour //
	EnsureShaderRenderTask();
	SHRender->GetColourBufferTwo()->Colour1 = (D3DXVECTOR4)color;

	// update text //
	CheapText* tmpt = TRenderer->GetCheapText(TextID);
	// get screen sizes for update method //
	Window* tmpw = TRenderer->GetOwningGraphics()->GetWindow();

	tmpt->Update(TRenderer, text, font, xypos, coordtype, sizemod, tmpw->GetWidth(), tmpw->GetHeight());
}

void Leviathan::BasicTextRendBlob::EnsureShaderRenderTask(){
	// create new if NULL //
	if(SHRender == NULL){

		SHRender = new ShaderRenderTask();
		// set required parts //
		SHRender->SetBaseMatrixBuffer(new BaseMatrixBufferData())->SetColourBufferTwo(new TwoColorBufferData());
	}

}

DLLEXPORT Rendering::BaseRenderableBufferContainer* Leviathan::BasicTextRendBlob::GetRenderingBuffers(Graphics* graph){
	// let text renderer to return it's internal buffers //
	EnsureShaderRenderTask();
	return TRenderer->GetRenderingBuffers(TextID, SHRender);
}

DLLEXPORT ShaderRenderTask* Leviathan::BasicTextRendBlob::GetShaderParameters(Graphics* graph, RenderingPassInfo* pass){
	EnsureShaderRenderTask();
	SetShaderMatrixBuffers(pass);

	// fetch textures from text renderer //
	TRenderer->UpdateShaderRenderTask(TextID, SHRender, pass);
	return SHRender;
}

// ------------------ ExpensiveTextRendBlob ------------------ //
DLLEXPORT Leviathan::ExpensiveTextRendBlob::ExpensiveTextRendBlob(Graphics* graph, const int &relativez, const int &slotid, const bool &hidden) : 
	RenderingGBlob(relativez, slotid, hidden)
{
	// get unique id for text //
	TextID = IDFactory::GetID();


}

DLLEXPORT Leviathan::ExpensiveTextRendBlob::~ExpensiveTextRendBlob(){
	// release the text //
	TRenderer->ReleaseText(TextID);
}

DLLEXPORT void Leviathan::ExpensiveTextRendBlob::Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
	const wstring &font, int coordtype /*= GUI_POSITIONABLE_COORDTYPE_RELATIVE*/, bool fittobox /*= false*/, const Float2 box /*= (Float2)0*/, const 
	float &adjustcutpercentage /*= 0.4f*/)
{
	// update z-order //
	RelativeZ = relativez;

	// set colour //
	EnsureShaderRenderTask();
	SHRender->GetColourBufferTwo()->Colour1 = (D3DXVECTOR4)color;

	// update text //
	ExpensiveText* tmpt = TRenderer->GetExpensiveText(TextID);

	// get screen sizes for update method //
	Window* tmpw = TRenderer->GetOwningGraphics()->GetWindow();

	tmpt->UpdateIfNeeded(TRenderer, text, font, xypos, coordtype, sizemod, adjustcutpercentage, box, fittobox, tmpw->GetWidth(), tmpw->GetHeight());
}

void Leviathan::ExpensiveTextRendBlob::EnsureShaderRenderTask(){
	// create new if NULL //
	if(SHRender == NULL){

		SHRender = new ShaderRenderTask();
		// set required parts //
		SHRender->SetBaseMatrixBuffer(new BaseMatrixBufferData())->SetColourBufferTwo(new TwoColorBufferData());
	}

}

DLLEXPORT Rendering::BaseRenderableBufferContainer* Leviathan::ExpensiveTextRendBlob::GetRenderingBuffers(Graphics* graph){
	// let text renderer to return it's internal buffers //
	EnsureShaderRenderTask();
	return TRenderer->GetRenderingBuffers(TextID, SHRender);
}

DLLEXPORT ShaderRenderTask* Leviathan::ExpensiveTextRendBlob::GetShaderParameters(Graphics* graph, RenderingPassInfo* pass){
	EnsureShaderRenderTask();
	SetShaderMatrixBuffers(pass);

	// fetch textures from text renderer //
	TRenderer->UpdateShaderRenderTask(TextID, SHRender, pass);
	return SHRender;
}