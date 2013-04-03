#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#include "GuiRendBlob.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ColorQuad.h"
#include "Engine.h"

RenderingGBlob::RenderingGBlob(){
	TypeName = -1;
}
RenderingGBlob::RenderingGBlob(int relativez, int slotid){
	RelativeZ = relativez;
	SlotID = slotid;
	TypeName = 0;
	Hidden = false;
	Updated = true;
}
RenderingGBlob::~RenderingGBlob(){

}
bool RenderingGBlob::IsThisType(int tochecktype){
	if(tochecktype == TypeName)
		return true;
	return false;
}
// ---------------- ColorQuardRendBlob -------------------- //
ColorQuadRendBlob::ColorQuadRendBlob() : RenderingGBlob(){
	//SmoothX = VAL_NOUPDATE;
	//SmoothY = VAL_NOUPDATE;
	//SmoothWidth = VAL_NOUPDATE;
	//SmoothHeight = VAL_NOUPDATE;
}
ColorQuadRendBlob::ColorQuadRendBlob(int relativez, int slotid, const Int2 &xypos, const Float4 &color, const Float4 &color2, int width, int height, 
	int colortranstype, bool absolute)
{
	RelativeZ = relativez;
	SlotID = slotid;
	TypeName = GUIRENDERING_BLOB_TYPE_CQUAD;
	Hidden = false;
	Updated = true;

	AbsoluteCoord = absolute;
	Coord = xypos;
	Color1 = color;
	Color2 = color2;
	Updated = true;
	Size = Int2(width,height);
	ColorTransType = colortranstype;

	CQuad = NULL;

	//SmoothX = Coord[0];
	//SmoothY = Coord[1];
	//SmoothWidth = Size[0];
	//SmoothHeight = Size[1];
}
ColorQuadRendBlob::~ColorQuadRendBlob(){
	SAFE_RELEASEDEL(CQuad);
}
void ColorQuadRendBlob::Update(int relativez, const Int2 &xypos, const Float4 &color, const Float4 &color2, int width, int height, int colortranstype,
	bool absolute)
{
	Updated = true;

	AbsoluteCoord = absolute;
	Coord = xypos;
	Color1 = color;
	Color2 = color2;
	Size = Int2(width,height);
	ColorTransType = colortranstype;
	RelativeZ = relativez;
}
void ColorQuadRendBlob::Get(Int2 &xypos, Float4 &color, Float4 &color2, Int2 &size, int &colortranstype, bool &absolute){
	xypos = Coord;
	color = Color1;
	color2 = Color2;
	size = Size;
	colortranstype = ColorTransType;
	absolute = AbsoluteCoord;
}
bool ColorQuadRendBlob::HasUpdated(){
	return Updated;
}
bool ColorQuadRendBlob::ConsumeUpdate(){
	if(Updated){
		Updated = false;
		return true;
	}
	return false;
}
// ---------------- BasicTextRendBlob -------------------- //
BasicTextRendBlob::BasicTextRendBlob() : RenderingGBlob(){
	HasText = false;
	//SmoothX = VAL_NOUPDATE;
	//SmoothY = VAL_NOUPDATE;
}
BasicTextRendBlob::BasicTextRendBlob(int relativez, int slotid, const Int2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
	bool absolute, const wstring &font)
{
	RelativeZ = relativez;
	SlotID = slotid;
	TypeName = GUIRENDERING_BLOB_TYPE_TEXT;
	Hidden = false;
	Updated = true;

	AbsoluteCoord = absolute;
	Size = sizemod;
	Coord = xypos;
	Color = color;
	Font = font;
	Text = text;
	TextID = IDFactory::GetID();

	HasText = false;
	//SmoothX = Coord[0];
	//SmoothY = Coord[1];
}
BasicTextRendBlob::~BasicTextRendBlob(){
	if(HasText){
		// needs to destroy the text //
		Engine::GetEngine()->GetGraphics()->GetTextRenderer()->ReleaseSentenceID(TextID);
		HasText = false;
	}
}
void BasicTextRendBlob::Update(int relativez, const Int2 &xypos, const Float4 &color, float sizemod, const wstring &text, bool absolute, 
	const wstring &font)
{
	Updated = true;

	AbsoluteCoord = absolute;
	Size = sizemod;
	Coord = xypos;
	Color = color;
	Font = font;
	Text = text;
	RelativeZ = relativez;
}
//Int2 &xypos, Float4 &color, float &size, wstring &text, wstring &font, bool &absolute, int& textid
void BasicTextRendBlob::Get(Int2 &xypos, Float4 &color, float &size, wstring &text, wstring &font, bool &absolute, int& textid){
	xypos = Coord;
	color = Color;
	size = Size;
	font = Font;
	text = Text;
	absolute = AbsoluteCoord;
	textid = TextID;
}
bool BasicTextRendBlob::HasUpdated(){
	return Updated;
}
bool BasicTextRendBlob::ConsumeUpdate(){
	if(Updated){
		Updated = false;
		return true;
	}
	return false;
}
void BasicTextRendBlob::SetUpdated(){
	Updated = true;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //