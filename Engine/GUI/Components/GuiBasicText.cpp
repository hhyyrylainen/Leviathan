#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUIBASICTEXT
#include "GuiBasicText.h"
#endif
#include "GUI\GuiManager.h"
#include "Utility\DebugVariableNotifier.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //

DLLEXPORT Leviathan::Gui::GuiBasicText::GuiBasicText(int slot, int zorder) : BaseGraphicalComponent(slot, zorder), Text(L""), Font(L"Arial"){
	// set every created flag to false //
	IsExpensiveText = false;
	NeedsAdjusting = true;

	// set type //
	CType = GUI_GRAPHICALCOMPONENT_TYPE_SIMPLETEXT;

	TextModifier = 1;

	OldExpensiveState = false;

	// flag default values //
	TextAdjustMode = GUI_BASICTEXT_MODE_JUSTRENDER;

	// ensure that length is negative so that it gets calculated when needed //
	TextLength = -1;
}

DLLEXPORT Leviathan::Gui::GuiBasicText::~GuiBasicText(){
	// nothing to delete //
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiBasicText::Init(const Float4 &colour, const wstring &text, const wstring &font, float textmodifier, 
	bool expensivetext /*= false*/, int adjustmode /*= GUI_BASICTEXT_MODE_JUSTRENDER*/, const float &cutfromscale)
{
	Text = text;
	IsExpensiveText = expensivetext;
	OldExpensiveState = IsExpensiveText;

	TextAdjustMode = adjustmode;
	TextCutScale = cutfromscale;
	PrimaryTextColour = colour;

	Font = font;

	TextModifier = textmodifier;

	// ensure that updated is set //
	RUpdated = true;
	NeedsAdjusting = true;

	return true;
}
DLLEXPORT void Leviathan::Gui::GuiBasicText::Release(RenderBridge* bridge){
	// remove from render bridge //
	bridge->DeleteBlobOnIndex(bridge->GetSlotIndex(Slot));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiBasicText::Render(RenderBridge* bridge, Graphics* graph){
	if(NeedsAdjusting){
		// needs to adjust size //
		_CheckTextAdjustment();
		NeedsAdjusting = false;
	}

	bool releaseold = false;

	if(IsExpensiveText != OldExpensiveState){
		// needs to change text type //
		RUpdated = true;
		releaseold = true;
	}

	if(!RUpdated){
		// nothing to update //
		return;
	}

	// get index from bridge //
	size_t i = bridge->GetSlotIndex(Slot);

	if(releaseold){
		// needs to delete old rendering blob //
		bridge->DeleteBlobOnIndex(i);
		i = bridge->GetSlotIndex(Slot);
	}


	// ensure that right thing and parameters are in the render bridge //


	if(bridge->DrawActions[i] == NULL){
		// create new object //
		bridge->DrawActions[i] = new TextRendBlob(graph, IDFactory::GetID(), ZOrder, Slot, false);
	}
	// update the existing object //
	TextRendBlob* tmpptr = (TextRendBlob*)bridge->DrawActions[i];

	tmpptr->Update(ZOrder, Position, PrimaryTextColour, TextModifier, Text, Font, CoordType, TextAdjustMode == GUI_BASICTEXT_MODE_TRYTOAUTOFIT,
		Size, TextCutScale);

	// set as non updated //
	RUpdated = false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiBasicText::UpdateText(const wstring &texttoset, bool expensivetext /*= false*/, int adjustmode /*= GUI_BASICTEXT_MODE_JUSTRENDER*/){
	// set new data //
	Text = texttoset;
	IsExpensiveText = expensivetext;
	TextAdjustMode = adjustmode;


	// set as updated //
	RUpdated = true;
	NeedsAdjusting = true;

	// set length to uncalculated //
	TextLength = -1;

	return true;
}
// ------------------------------------ //
void Leviathan::Gui::GuiBasicText::_OnLocationOrSizeChange(){
	// set as updated //
	RUpdated = true;
}

void Leviathan::Gui::GuiBasicText::_CheckTextAdjustment(){
	// return if adjustment isn't wanted //
	if(TextAdjustMode == GUI_BASICTEXT_MODE_JUSTRENDER)
		return;
	// the text renderer will automatically adjust it, just set parameters to the 
}

DLLEXPORT bool Leviathan::Gui::GuiBasicText::GetTextLength(float &lengthreceiver, float &heightreceiver){
	if(TextLength < 0){
		// calculate length and height //
		TextLength = -1;
		TextHeigth = -1;
	}

	lengthreceiver = TextLength;
	heightreceiver = TextHeigth;
	return true;
}

