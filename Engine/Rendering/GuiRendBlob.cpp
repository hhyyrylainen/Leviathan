#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#include "GuiRendBlob.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ColorQuad.h"
#include "Engine.h"

Leviathan::RenderingGBlob::RenderingGBlob(){
	TypeName = -1;
}
Leviathan::RenderingGBlob::RenderingGBlob(int relativez, int slotid){
	RelativeZ = relativez;
	SlotID = slotid;
	TypeName = 0;
	Hidden = false;
	Updated = true;
}
Leviathan::RenderingGBlob::~RenderingGBlob(){

}
bool RenderingGBlob::IsThisType(int tochecktype){
	if(tochecktype == TypeName)
		return true;
	return false;
}
// ---------------- ColorQuardRendBlob -------------------- //
Leviathan::ColorQuadRendBlob::ColorQuadRendBlob() : RenderingGBlob(){

}
Leviathan::ColorQuadRendBlob::ColorQuadRendBlob(int relativez, int slotid, const Float2 &xypos, const Float4 &color, 
	const Float4 &color2, const Float2 &size, int colortranstype, int coordinatetype) : RenderingGBlob(relativez, slotid), Color1(color), 
	Color2(color2), Coord(xypos), Size(size)
{
	TypeName = GUIRENDERING_BLOB_TYPE_CQUAD;

	Hidden = false;
	Updated = true;

	CoordType = coordinatetype;

	Updated = true;
	ColorTransType = colortranstype;

	CQuad = NULL;
}
Leviathan::ColorQuadRendBlob::~ColorQuadRendBlob(){
	SAFE_RELEASEDEL(CQuad);
}
void Leviathan::ColorQuadRendBlob::Update(int relativez, const Float2 &xypos, const Float4 &color, const Float4 &color2, const Float2 &size, 
	int colortranstype, int coordinatetype)
{
	Updated = true;

	CoordType = coordinatetype;
	Coord = xypos;
	Color1 = color;
	Color2 = color2;
	Size = size;
	ColorTransType = colortranstype;

	RelativeZ = relativez;
}
void Leviathan::ColorQuadRendBlob::Get(Float2 &xypos, Float4 &color, Float4 &color2, Float2 &size, int &colortranstype, int &coordinatetype){
	xypos = Coord;
	color = Color1;
	color2 = Color2;
	size = Size;
	colortranstype = ColorTransType;
	coordinatetype = CoordType;
}
bool Leviathan::ColorQuadRendBlob::HasUpdated(){
	return Updated;
}
bool Leviathan::ColorQuadRendBlob::ConsumeUpdate(){
	if(Updated){
		Updated = false;
		return true;
	}
	return false;
}
// ---------------- BasicTextRendBlob -------------------- //
Leviathan::BasicTextRendBlob::BasicTextRendBlob() : RenderingGBlob(){
	HasText = false;
}
Leviathan::BasicTextRendBlob::BasicTextRendBlob(int relativez, int slotid, const Float2 &xypos, const Float4 &color, float sizemod, 
	const wstring &text, const wstring &font, int coordtype) : RenderingGBlob(relativez, slotid), Coord(xypos), Color(color), Font(font), Text(text)
{	
	TypeName = GUIRENDERING_BLOB_TYPE_TEXT;

	Hidden = false;
	Updated = true;

	CoordType = coordtype;
	Size = sizemod;

	// get unique id for text //
	TextID = IDFactory::GetID();

	HasText = false;
}
Leviathan::BasicTextRendBlob::~BasicTextRendBlob(){
	if(HasText){
		// needs to destroy the text //
		Engine::GetEngine()->GetGraphics()->GetTextRenderer()->ReleaseSentenceID(TextID);
		HasText = false;
	}
}
// ------------------------------------ //
void Leviathan::BasicTextRendBlob::Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
	const wstring &font, int coordtype)
{
	Updated = true;

	CoordType = coordtype;
	Size = sizemod;
	Coord = xypos;
	Color = color;
	Font = font;
	Text = text;

	RelativeZ = relativez;
}
void Leviathan::BasicTextRendBlob::Get(Float2 &xypos, Float4 &color, float &size, wstring &text, wstring &font, int &coordtype, int& textid){
	xypos = Coord;
	color = Color;
	size = Size;
	font = Font;
	text = Text;
	coordtype = CoordType;
	textid = TextID;
}
// ------------------------------------ //
bool Leviathan::BasicTextRendBlob::HasUpdated(){
	return Updated;
}
bool Leviathan::BasicTextRendBlob::ConsumeUpdate(){
	if(Updated){
		Updated = false;
		return true;
	}
	return false;
}
void Leviathan::BasicTextRendBlob::SetUpdated(){
	Updated = true;
}
// ------------------------------------ //


DLLEXPORT Leviathan::ExpensiveTextRendBlob::ExpensiveTextRendBlob(int relativez, int slotid, const Float2 &xypos, const Float4 &color, float sizemod, 
	const wstring &text, const wstring &font, int coordtype /*= GUI_POSITIONABLE_COORDTYPE_RELATIVE*/, bool fittobox /*= false*/, const Float2 box 
	/*= (Float2)0*/, const float &adjustcutpercentage /*= 0.4f*/) : RenderingGBlob(relativez, slotid), Coord(xypos), BoxToFit(box), 
	CoordType(coordtype), Color(color), Font(font), Text(text), AdjustCutModifier(adjustcutpercentage)
{
	TypeName = GUIRENDERING_BLOB_TYPE_EXPENSIVETEXT;

	Size = sizemod;
	FitToBox = fittobox;

	TextID = IDFactory::GetID();
}

DLLEXPORT Leviathan::ExpensiveTextRendBlob::~ExpensiveTextRendBlob(){
	// release the text //
	Graphics::Get()->GetTextRenderer()->ReleaseExpensiveText(TextID);
}

DLLEXPORT void Leviathan::ExpensiveTextRendBlob::Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
	const wstring &font, int coordtype /*= GUI_POSITIONABLE_COORDTYPE_RELATIVE*/, bool fittobox /*= false*/, const Float2 box /*= (Float2)0*/, const 
	float &adjustcutpercentage /*= 0.4f*/)
{
	Size = sizemod;

	Coord = xypos;
	BoxToFit = box;
	FitToBox = fittobox;
	CoordType = coordtype;
	AdjustCutModifier = adjustcutpercentage;

	Color = color;

	Font = font;
	Text = text;
}
