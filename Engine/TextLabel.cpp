#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_TEXTLABEL
#include "TextLabel.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include "GuiManager.h"
#include "DataStore.h"
#include "GuiAnimation.h"

#include "GuiScriptInterface.h"
#include "DataBlock.h"
#include "ObjectBackground.h"
#include "GuiBasicText.h"
#include "Rendering\ColorQuad.h"


Leviathan::Gui::TextLabel::TextLabel(int id, const Float2 &position, Float2 &size, int autoadjust) : Positionable(position, size), GComponents(2){
	// set object levels //
	ObjectLevel = GUI_OBJECT_LEVEL_ANIMATEABLE;
	Objecttype = GOBJECT_TYPE_TEXTLABEL;

	// setting created flags to false //
	BridgeCreated = false;

	AutoAdjust = autoadjust;
	TextAdjustMode = GUI_BASICTEXT_MODE_JUSTRENDER;

	Updated = false;
	Hidden = false;
	OldHidden = false;
	// copy id //
	ID = id;
	
	Zorder = 1;


	TextWantedCoordinates = position;
	TextAreaSize = size;

	// default padding around text //
	TextPadding = 0.012f;
	TextPaddingY = 0.008f;
}

Leviathan::Gui::TextLabel::~TextLabel(){
	// release rendering stuff //
	if(RBridge.get() != NULL)
		RBridge->WantsToClose = true;
	RBridge.reset();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::TextLabel::Init(const wstring &text, const wstring &font, const Float4 &textcolor, float textsize, 
	const Float4 &color1, const Float4 &color2, vector<shared_ptr<VariableBlock>>* listenindexes)
{
	Updated = true;

	// initialize graphical components //
	if(GComponents.size() != 2)
		GComponents.resize(2);

	// background //
	GComponents[0] = new ObjectBackground(0, 1);
	if(!GComponents[0]){

		QUICK_MEMORY_ERROR_MESSAGE;
		return false;
	}

	// initialize it //
	if(!((ObjectBackground*)GComponents[0])->Init(color1, color2, COLORQUAD_COLOR_STYLE_LEFTTOPRIGHTBOTTOM)){

		QUICK_ERROR_MESSAGE;
		return false;
	}

	// text rendering //
	GComponents[1] = new GuiBasicText(1, 2);
	if(!GComponents[1]){

		QUICK_MEMORY_ERROR_MESSAGE;
		return false;
	}

	// initialize it //
	TextAdjustMode = GUI_BASICTEXT_MODE_JUSTRENDER;

	// check is autoadjust set to adjust text to size //
	if(AutoAdjust > 1)
		TextAdjustMode = GUI_BASICTEXT_MODE_TRYTOAUTOFIT;

	if(!((GuiBasicText*)GComponents[1])->Init(textcolor, text, font, textsize, TextAdjustMode != GUI_BASICTEXT_MODE_JUSTRENDER, TextAdjustMode)){

		QUICK_ERROR_MESSAGE;
		return false;
	}
	// set size to text to ensure that it is set //
	((GuiBasicText*)GComponents[1])->SetLocationData(TextWantedCoordinates, TextAreaSize);
	((ObjectBackground*)GComponents[0])->SetLocationData(Position, Size);


	
	if(AutoAdjust)
		 SizeAdjust();

	if(listenindexes){
		// start monitoring //
		StartMonitoring(*listenindexes);
	}
	// register //
	RegisterForEvent(EVENT_TYPE_HIDE);
	RegisterForEvent(EVENT_TYPE_SHOW);

	return true;
}
void Leviathan::Gui::TextLabel::Release(){
	// send close message //

	// close Graphical components //
	while(GComponents.size()){
		// release if exists //
		if(GComponents[0] && RBridge.get() != NULL)
			GComponents[0]->Release(RBridge.get());
		SAFE_DELETE(GComponents[0]);
		GComponents.erase(GComponents.begin());
	}


	// set bridge to die
	if(RBridge.get() != NULL)
		(*RBridge).WantsToClose = TRUE;
	RBridge.reset();

	// don't listen to anything anymore //
	StopMonitoring(MonitoredValues);
}
// ------------------------------------ //
void Leviathan::Gui::TextLabel::Render(Graphics* graph){
	NoNUpdatedFrames++;
	if((!ValuesUpdated) && (!Updated) && (OldHidden == Hidden) && (NoNUpdatedFrames < GUI_RENDERABLE_FORCEUPDATE_EVERY_N_FRAMES))
		return;
	// reset update values //
	Updated = false;
	NoNUpdatedFrames = 0;

	// check bridge creation //
	if(!BridgeCreated){
		// create rendering bridge for communicating with renderer //
		RBridge = shared_ptr<RenderBridge>(new RenderBridge(this->ID, this->Hidden, this->Zorder));

		// submit //
		graph->SubmitRenderBridge(RBridge); // this hopefully copies the bridge and maintains the second copy

		// created //
		BridgeCreated = true;
	}

	// check hidden state //
	if((Hidden) && (!OldHidden)){
		OldHidden = true;

		_SetHiddenStates(true);

	} else if ((!Hidden) && (OldHidden)){
		OldHidden = false;

		_SetHiddenStates(false);
	}

	if(ValuesUpdated){
		// received listener value //
		// even if there are multiple values just use one and pop the rest //

		// get values //
		VariableBlock* updatedvalue = UpdatedValues[0]->GetValueDirect();
		// temporary text holder //
		wstring text = L"";

		// error if cannot be made into wstring or directly assign new value to wstring //
		if(!updatedvalue->ConvertAndAssingToVariable<wstring>(text)){
			// well that's an error //
			wstring errormessage = L"TextLabel: Render: processing updated values, cannot cast value to wstring: "+
				Convert::ToWstring<int>(updatedvalue->GetBlock()->Type);
			text = errormessage;
			Logger::Get()->Error(errormessage, updatedvalue->GetBlock()->Type, true);

			// unregister name //
			int ival = -1;

			vector<shared_ptr<VariableBlock>> tounregister(1);

			wstringstream streamy(UpdatedValues[0]->GetName());
			streamy >> ival;
			if(ival != -1){
				// it was integer //

				tounregister[0] = shared_ptr<VariableBlock>(new VariableBlock(ival));

			} else {
				// name //
				tounregister[0] = shared_ptr<VariableBlock>(new VariableBlock(UpdatedValues[0]->GetName()));
			}
			// stop listening for this invalid index/name //
			StopMonitoring(tounregister);
		} else {
			// call script (if right listeners exist) //


			// remove all other update messages //
			_PopUdated();

			// set text //
			((GuiBasicText*)GComponents[1])->UpdateText(text, TextAdjustMode != GUI_BASICTEXT_MODE_JUSTRENDER, TextAdjustMode);
		}
	}


	if(Hidden){
		// updating anything can wait until visible //
		return;
	}

	// adjust size if visible and wanted //
	if(AutoAdjust)
		SizeAdjust();


	// update this ZOrder //
	RBridge->ZVal = this->Zorder;
	
	// update components if needed //
	assert(GComponents.size() == 2 && "non initialized TextLabel");

	// send location to background //
	((ObjectBackground*)GComponents[0])->SetLocationData(Position, Size);

	// location to text //
	((GuiBasicText*)GComponents[1])->SetPosition(TextWantedCoordinates);
	
	// render graphical components //

	GComponents[0]->Render(RBridge.get(), graph);
	GComponents[1]->Render(RBridge.get(), graph);
}
// ------------------------------------ //
void Leviathan::Gui::TextLabel::SetHiddenState(bool hidden){
	Updated = true;
	this->Hidden = hidden;
}
// ------------------------------------ //
void Leviathan::Gui::TextLabel::SizeAdjust(){

	if(AutoAdjust > 1){
		// using text to fit box mode, no need to update sizes here //

		// send old size to background //
		((ObjectBackground*)GComponents[0])->SetSize(Size);

		// set position to text //
		((GuiBasicText*)GComponents[1])->SetLocationData(TextWantedCoordinates, TextAreaSize);

		return;
	}


	// get text length //
	float length = 0;
	float heigth = 0;

	((GuiBasicText*)GComponents[1])->GetTextLength(length, heigth);

	// count new size //

	Size.X = TextPadding*2+length;

	Size.Y = TextPaddingY*2+heigth;

	// calculate area for text //
	TextWantedCoordinates = Float2(Position.X+TextPadding, Position.Y+TextPaddingY);
	TextAreaSize = Float2(Size.X-TextPadding, Size.Y-TextPaddingY);

	// send new size to background //
	((ObjectBackground*)GComponents[0])->SetSize(Size);

	// set position to text //
	((GuiBasicText*)GComponents[1])->SetLocationData(TextWantedCoordinates, TextAreaSize);

}
 // ------------------------------------ //
int Leviathan::Gui::TextLabel::AnimationTime(int mspassed){
	if(AnimationQueue.size() == 0)
		return 0;
	bool contaction = false;

	for(size_t i = 0; i < AnimationQueue.size(); i++){
		contaction = AnimationQueue[i]->AllowSimultanous;

		if(AnimationQueue[i]->Type == GUI_ANIMATION_HIDE){
			this->Hidden = true;
			Updated = true;

			AnimationQueue.erase(AnimationQueue.begin()+i);
			i--;
			continue;
		}
		if(AnimationQueue[i]->Type == GUI_ANIMATION_SHOW){
			this->Hidden = false;
			Updated = true;

			AnimationQueue.erase(AnimationQueue.begin()+i);
			i--;
			continue;
		}

		if(GuiManager::Get()->HandleAnimation(AnimationQueue[i].get(), this, mspassed) == 1){
			// event completed //
			
			AnimationQueue.erase(AnimationQueue.begin()+i);
			i--;
		}

		if(!contaction) // break if simultaneous flag is not set
			break;
	}

	if(AnimationQueue.size() == 0){
		AnimationFinish();
	}

	return 0;
}

void Leviathan::Gui::TextLabel::AnimationFinish(){
	// call script event if script wants to receive //
}
// ------------------------------------ //
int Leviathan::Gui::TextLabel::OnEvent(Event** pEvent){
	// figure what to do based on type
	switch((*pEvent)->Type){
	case EVENT_TYPE_HIDE:
		{
			// run specific script if found //
			if(Scripting->Script == NULL)
				return 0;

			if(Scripting->Script->Instructions.size() < 1)
				return 0;

			vector<shared_ptr<NamedVariableBlock>> Params;
			Params.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock((*(Int2*)(*pEvent)->Data)[0]), L"Source")));
			Params.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock(this->ID), L"InstanceID")));

			bool existed = false;
			shared_ptr<VariableBlock> returned = ScriptInterface::Get()->ExecuteIfExistsScript(Scripting.get(), L"OnHide", Params, existed, false);

			// check did it exist //
			if(!existed){
				// script didn't exist //
				return 0;
			}
			// script's exit value //
			int Value(*returned.get());
			return Value;
		}
	break;
	case EVENT_TYPE_SHOW:
		{
			// run specific script if found //
			if(Scripting->Script == NULL)
				return 0;

			if(Scripting->Script->Instructions.size() < 1)
				return 0;

			vector<shared_ptr<NamedVariableBlock>> Params;
			Params.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock((*(Int2*)(*pEvent)->Data)[0]), L"Source")));
			Params.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock(this->ID), L"InstanceID")));

			bool existed = false;
			shared_ptr<VariableBlock> returned = ScriptInterface::Get()->ExecuteIfExistsScript(Scripting.get(), L"OnShow", Params, existed, false);

			// check did it exist //
			if(!existed){
				// script didn't exist //
				return 0;
			}
			// script's exit value //
			int Value(*returned.get());
			return Value;
		}
	break;
	}

	// not used, request unregistration
	return -1;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::TextLabel::SetValue(const int &semanticid, const float &val){
	// switch to right value //
	switch(semanticid){
	case GUI_ANIMATEABLE_SEMANTIC_X:
		{
			Position.X = val;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_Y:
		{
			Position.Y = val;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_WIDTH:
		{
			Size.X = val;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_HEIGHT:
		{
			Size.Y = val;
		}
	break;
	}
	// data has been updated //
	Updated = true;
}

DLLEXPORT float Leviathan::Gui::TextLabel::GetValue(const int &semanticid) const{
	// return semantic data (inverse of SetValue) //
	switch(semanticid){
	case GUI_ANIMATEABLE_SEMANTIC_X:		return Position.X;
	case GUI_ANIMATEABLE_SEMANTIC_Y:		return Position.Y;
	case GUI_ANIMATEABLE_SEMANTIC_WIDTH:	return Size.X;
	case GUI_ANIMATEABLE_SEMANTIC_HEIGHT:	return Size.Y;
	}

	return -1.0f;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::TextLabel::UpdateBackgroundColours(const Float4 &colour1, const Float4 &colour2, int gradienttype){
	assert(GComponents.size() == 2 && "non initialized TextLabel");

	return ((ObjectBackground*)GComponents[0])->UpdateGradient(colour1, colour2, gradienttype);
}

DLLEXPORT bool Leviathan::Gui::TextLabel::UpdateText(const wstring &text, bool isexpensive /*= false*/){
	assert(GComponents.size() == 2 && "non initialized TextLabel");

	return ((GuiBasicText*)GComponents[1])->UpdateText(text, isexpensive, TextAdjustMode);
}
// ------------------------------------ //
void Leviathan::Gui::TextLabel::_SetHiddenStates(bool states){
	// set whole bridge as hidden //
	RBridge->Hidden = states;
	//// these two aren't even required //
	//RBridge->SetHidden(0, states);
	//RBridge->SetHidden(1, states);
}

DLLEXPORT void Leviathan::Gui::TextLabel::QueueAction(shared_ptr<AnimationAction> act){
	AnimationQueue.push_back(act);
}

DLLEXPORT bool Leviathan::Gui::TextLabel::LoadFromFileStructure(vector<BaseGuiObject*> &tempobjects, vector<Int2> &idmappairs, 
	ObjectFileObject& dataforthis)
{
	// try to load a TextLabel from the structure //
	int RealID = IDFactory::GetID();

	for(size_t a = 0; a < dataforthis.Prefixes.size(); a++){
		if(Misc::WstringStartsWith(*dataforthis.Prefixes[a], L"ID")){

			// use wstring iterator to get the id number //
			WstringIterator itr(dataforthis.Prefixes[a].get(), false);

			unique_ptr<wstring> itrresult = itr.GetNextNumber(DECIMALSEPARATORTYPE_NONE);

			// check first word //
			if(itrresult->size() == 0){

				// invalid number //
				Logger::Get()->Error(L"TextLabel: LoadFromFileStructure: invalid number as id, in prefix: "+*dataforthis.Prefixes[a]);

			} else {
				// should be valid id //
				idmappairs.push_back(Int2(Convert::WstringTo<int>(*itrresult), RealID));
			}

			break;
		}
	}


	float x = -36003;
	float y = -36003;
	float width = -36003;
	float height = -36003;

	wstring text(L"");

	Float4 StartColor;
	Float4 EndColor;
	Float4 TextColor;

	float TextSize = 1.0f;
	int AutoAdjust = 1;
	wstring Font = L"arial";

	vector<bool> AreIndexes;
	vector<shared_ptr<VariableBlock>> ListenIndexes;

	// get values for initiation //
	for(size_t a = 0; a < dataforthis.Contents.size(); a++){
		// check what list is being processed //
		if(dataforthis.Contents[a]->Name == L"params"){
			// get variables //

			ObjectFileProcessor::LoadValueFromNamedVars<float>(dataforthis.Contents[a]->Variables, L"X", x, FLT_MAX, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<float>(dataforthis.Contents[a]->Variables, L"Y", y, FLT_MAX, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<float>(dataforthis.Contents[a]->Variables, L"Width", width, FLT_MAX, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<float>(dataforthis.Contents[a]->Variables, L"Height", height, FLT_MAX, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<float>(dataforthis.Contents[a]->Variables, L"TextSizeMod", TextSize, 1.f, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<int>(dataforthis.Contents[a]->Variables, L"AutoAdjust", AutoAdjust, 1, true, 
				L"TextLabel: LoadFromFileStructure:");


			ObjectFileProcessor::LoadValueFromNamedVars<wstring>(dataforthis.Contents[a]->Variables, L"StartText", text, L"", true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<wstring>(dataforthis.Contents[a]->Variables, L"Font", Font, L"Arial", true,
				L"TextLabel: LoadFromFileStructure:");



			// listen on should allow strings and multiple numbers //
			
			try{
				vector<VariableBlock*>* listenvalues = dataforthis.Contents[a]->Variables->GetValues(L"ListenOn");

				// check values //
				for(size_t i = 0; i < listenvalues->size(); i++){
					// check is it wstring or int //

					if(listenvalues->at(i)->GetBlock()->Type == DATABLOCK_TYPE_INT){

						// int index //
						AreIndexes.push_back(true);
						ListenIndexes.push_back(shared_ptr<VariableBlock>(new VariableBlock(listenvalues->at(i)->GetBlock()->AllocateNewFromThis())));


					} else {
						// skip if cannot be made into wstring //
						if(!listenvalues->at(i)->IsConversionAllowedPtr<wstring>()){
							continue;
						}

						// is a string index //
						AreIndexes.push_back(false);
						ListenIndexes.push_back(shared_ptr<VariableBlock>(new VariableBlock(listenvalues->at(i)->GetBlock()->AllocateNewFromThis())));
					}
				}

			}
			catch(...){
				// nothing to listen on //
				ListenIndexes.clear();
			}

			ObjectFileProcessor::LoadMultiPartValueFromNamedVars<Float4, float, 4>(dataforthis.Contents[a]->Variables, L"TextColor", TextColor,
				Float4::ColourWhite, true, L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadMultiPartValueFromNamedVars<Float4, float, 4>(dataforthis.Contents[a]->Variables, L"StartColor", StartColor,
				Float4::ColourBlack, true, L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadMultiPartValueFromNamedVars<Float4, float, 4>(dataforthis.Contents[a]->Variables, L"EndColor", EndColor,
				Float4::ColourBlack, true, L"TextLabel: LoadFromFileStructure:");
		}
	}

	// create object //

	Float2 tmppos(x, y);
	Float2 tmpsize(width, height);

	TextLabel* curlabel = new TextLabel(RealID, tmppos, tmpsize, AutoAdjust);
	// add to temporary objects //
	tempobjects.push_back(curlabel);


	// init with correct values //
	curlabel->Init(text, Font, TextColor, TextSize, StartColor, EndColor, &ListenIndexes);
	curlabel->Scripting = shared_ptr<ScriptObject>(dataforthis.CreateScriptObjectAndReleaseThis(0, GOBJECT_TYPE_TEXTLABEL));

	// succeeded //
	return true;
}

void Leviathan::Gui::TextLabel::_OnLocationOrSizeChange(){
	Updated = true;
}
