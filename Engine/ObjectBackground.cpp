#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUIOBJECTBACKGROUND
#include "ObjectBackground.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include ".\Rendering\ColorQuad.h"

// initialize all different background pointers to NULL //
DLLEXPORT Leviathan::Gui::ObjectBackground::ObjectBackground(int slot, int zorder) : BaseGraphicalComponent(slot, zorder), GData(NULL) {
	// set type //
	CType = GUI_GRAPHICALCOMPONENT_TYPE_BACKGROUND;
	// not initialized //
	WhichType = BACKGROUNDTYPE_NONE;
}

DLLEXPORT Leviathan::Gui::ObjectBackground::~ObjectBackground(){
	// release data if any (should have been released) //
	_UnAllocateAllBackgroundData();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::ObjectBackground::Release(RenderBridge* bridge){
	// remove from render bridge //

	size_t i = bridge->GetSlotIndex(Slot);

	SAFE_DELETE(bridge->DrawActions[i]);

	bridge->DrawActions.erase(bridge->DrawActions.begin());

	// unallocate //
	_UnAllocateAllBackgroundData();
}

DLLEXPORT bool Leviathan::Gui::ObjectBackground::Init(const Float4 &colour1, const Float4 &colour2, int gradient){
	// check has this already been initialized (update methods should be used) //
	if(WhichType != BACKGROUNDTYPE_NONE){
		// needs to unallocate old one //
		_UnAllocateAllBackgroundData();
	}
	// initialize with gradient style //
	WhichType = BACKGROUNDTYPE_GRADIENT;

	// allocate new object //
	GData = new GradientBackgroundData(colour1, colour2, gradient);
	// ensure that updated is set //
	RUpdated = true;

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::ObjectBackground::Render(RenderBridge* bridge, Graphics* graph){
	if(!RUpdated){
		// nothing to update //
		return;
	}

	// ensure that right thing and parameters are in the render bridge //
	size_t i = bridge->GetSlotIndex(Slot);
	// check for NULL //
	if(bridge->DrawActions[i] == NULL){

		// allocate new //

		switch(WhichType){
		case BACKGROUNDTYPE_GRADIENT:
			{
				bridge->DrawActions[i] = new ColorQuadRendBlob(ZOrder, Slot, Position, GData->Colour1, GData->Colour2, Size, GData->GradientType,
					CoordType);
			}
			break;
		default:
			// error //
			DEBUG_BREAK;
		}
	} else {
		// update rendering blob //
		switch(WhichType){
		case BACKGROUNDTYPE_GRADIENT:
			{
				((ColorQuadRendBlob*)bridge->DrawActions[i])->Update(ZOrder, Position, GData->Colour1, GData->Colour2, Size, GData->GradientType,
					CoordType);
			}
			break;
		default:
			// error //
			DEBUG_BREAK;
		}

	}
	// set as non updated //
	RUpdated = false;
	PositionsUpdated = false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::ObjectBackground::UpdateGradient(const Float4 &colour1, const Float4 &colour2, int gradient){
	// return if wrong type //
	if(WhichType != BACKGROUNDTYPE_GRADIENT)
		return false;

	// update object //
	GData->Colour1 = colour1;
	GData->Colour2 = colour2;
	GData->GradientType = gradient;

	// set as updated //
	RUpdated = true;

	return true;
}
// ------------------------------------ //
void Leviathan::Gui::ObjectBackground::_OnLocationOrSizeChange(){
	// set as updated //
	RUpdated = true;
}

void Leviathan::Gui::ObjectBackground::_UnAllocateAllBackgroundData(){
	SAFE_DELETE(GData);
}
// ------------------------------------ //
Leviathan::Gui::GradientBackgroundData::GradientBackgroundData(const Float4 &colour1, const Float4 &colour2, int type) : Colour1(colour1), 
	Colour2(colour2)
{
	GradientType = type;
	// not created //
	RBridgeObjectCreated = false;
}
