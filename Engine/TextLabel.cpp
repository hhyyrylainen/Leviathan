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
#include ".\Rendering\ColorQuad.h"
#include "GuiScriptInterface.h"

Leviathan::Gui::TextLabel::TextLabel(int id) : LText(L"NONE"), Font(L"Arial") {
	// setting created flags to false //
	Updated = false;
	BridgeCreated = false;
	QuadCreated = false;
	TextCreated = false;
	TextHidden = false;
	OldHidden = false;

	X = 0;
	Y = 0;
	Width = 20;
	Height = 5;
	Hidden = false;

	ID = id;

	ObjectLevel = GUI_OBJECT_LEVEL_ANIMATEABLE;
	Objecttype = GOBJECT_TYPE_TEXTLABEL;

	Zorder = 1;

	RelaTivedTextMod = -1.f;
	// unique id for text (when it is created) //
	TextID = IDFactory::GetID();
	TextLength = -1;
	// default padding around text //
	TextPadding = 5;
	TextPaddingY = 3;
}

Leviathan::Gui::TextLabel::~TextLabel(){
	// release rendering stuff //
	if(RBridge.get() != NULL)
		RBridge->WantsToClose = true;
	RBridge.reset();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::TextLabel::Init(int xpos, int ypos, int width, int height, const wstring &text, const Float4 &color1, 
	const Float4 &color2, const Float4 &textcolor, float textsize /*= 1.0f*/, bool autoadjust /*= true*/, const wstring &font /*= L"Arial"*/, 
	vector<shared_ptr<VariableBlock>>* listenindexes /*= NULL*/)
{
	Updated = true;

	LText = text;
	X = xpos;
	Y = ypos;
	Width = width;
	Height = height;

	TextColour = textcolor;
	Colour1 = color1;
	Colour2 = color2;

	Font = font;
	AutoAdjust = autoadjust;

	TextMod = textsize;
	RelaTivedTextMod = ResolutionScaling::ScaleTextSize(TextMod);

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

	Updated = false;

	NoNUpdatedFrames = 0;

	// check bridge creation //
	if(!BridgeCreated){
		// create rendering bridge for communicating with renderer //
		RBridge = shared_ptr<RenderBridge>(new RenderBridge(this->ID, this->Hidden, this->Zorder));

		// submit //
		graph->SubmitRenderBridge(RBridge); // this hopefully copies the bridge and maintains the second copy
		// this won't have to be created again //
		RBridge->DrawActions.push_back(new ColorQuadRendBlob(1,0, Int2(X,Y), Colour1, Colour2, Width, Height, 
			COLOR_QUAD_COLOR_STYLE_LEFT_TOP_RIGHT_BOTTOM, false));

		// created //
		BridgeCreated = true;
	}

	// send text create if not created //
	if(!TextCreated){
		
		// create text rendering part into bridge //
		RBridge->DrawActions.push_back(new BasicTextRendBlob(5,1, Int2(X+TextPadding, Y+TextPaddingY), TextColour, TextMod, LText, false, Font));

		// created //
		TextCreated = true;
	}

	if(ValuesUpdated){
		// received listener value //
		
		// even if there are multiple values just use one and pop the rest //

		// get values //
		VariableBlock* updatedvalue = UpdatedValues[0]->GetValueDirect();

		// error if cannot be made into wstring or directly assign new value to wstring //
		if(!updatedvalue->ConvertAndAssingToVariable<wstring>(LText)){
			// well that's an error //
			wstring errormessage = L"TextLabel: Render: processing updated values, cannot cast value to wstring: "+
				Convert::ToWstring<int>(updatedvalue->GetBlock()->Type);
			LText = errormessage;
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
		}

		// call script (if right listeners exist) //


		// remove all other update messages //
		_PopUdated();

		// adjust sizes if wanted //
		if(AutoAdjust){
			SizeAdjust();
		}
	}

	// send messages //
	if((Hidden) && (!OldHidden)){
		OldHidden = true;

		_SetHiddenStates(true);

	} else if ((!Hidden) && (OldHidden)){
		OldHidden = false;

		_SetHiddenStates(false);
	}

	// base //
	if(!Hidden){
		// update this ZOrder //
		RBridge->ZVal = this->Zorder;

		// update it //
		int Index = RBridge->GetSlotIndex(0);
		if(Index < 0){
			BridgeCreated = false;
		} else {
			// cast object //
			ColorQuadRendBlob* tempuptr = reinterpret_cast<ColorQuadRendBlob*>(RBridge->DrawActions[Index]);

			if(tempuptr == NULL){
				// cast failed //
				DEBUG_BREAK;
				// this really should never happen //
				assert(0);
			}
			// update object //
			tempuptr->Update(1, Int2(X,Y), Colour1, Colour2, Width, Height, COLOR_QUAD_COLOR_STYLE_LEFT_TOP_RIGHT_BOTTOM, false);
		}
	}
	// text updating //
	if(!TextHidden){
		// update it //
		int Index = (*RBridge).GetSlotIndex(1);
		if(Index < 0){
			TextCreated = false;
		} else {
			// cast //
			BasicTextRendBlob* tempuptr = reinterpret_cast<BasicTextRendBlob*>(RBridge->DrawActions[Index]);

			if(tempuptr == NULL){
				// cast failed //
				DEBUG_BREAK;
				// this really should never happen //
				assert(0);
			}

			// updating //
			tempuptr->Update(5, Int2(X+TextPadding,Y+TextPaddingY), TextColour, TextMod, LText, false, Font);
		}
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::TextLabel::UpdateColours(const Float4 &color1, const Float4 &color2, const Float4 &textcolor){
	Updated = true;
	TextColour = textcolor;
	Colour1 = color1;
	Colour2 = color2;
}

DLLEXPORT void Leviathan::Gui::TextLabel::Update(int xpos, int ypos, int width /*= -1*/, int height /*= -1*/, bool autoadjust /*= true */, const wstring &text /*= L""*/){
	if(xpos != VAL_NOUPDATE){
		Updated = true;
		X = xpos;
	}
	if(ypos != VAL_NOUPDATE){
		Updated = true;
		Y = ypos;
	}
	if(width != VAL_NOUPDATE){
		Updated = true;
		Width = width;
	}
	if(height != VAL_NOUPDATE){
		Updated = true;
		Height = height;
	}
	if(text != L""){
		Updated = true;
		LText = text;
	}
	AutoAdjust = autoadjust;
	if(AutoAdjust)
		SizeAdjust();
}
void Leviathan::Gui::TextLabel::SetHiddenState(bool hidden){
	Updated = true;
	this->Hidden = hidden;
}
// ------------------------------------ //
void Leviathan::Gui::TextLabel::SizeAdjust(){

	// get text length //
	int textlength = -1;
	textlength = GuiManager::Get()->GetGraph()->CountTextRenderLength(LText, Font, TextMod, false);


	// count new size //


	Width = TextPadding*2+textlength;


	Height = TextPaddingY*2+GuiManager::Get()->GetGraph()->GetTextRenderHeight(Font, TextMod, false);

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
			break;
		}
		if(AnimationQueue[i]->Type == GUI_ANIMATION_SHOW){
			this->Hidden = false;
			Updated = true;

			AnimationQueue.erase(AnimationQueue.begin()+i);
			i--;
			break;
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
			this->X = (int)val;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_Y:
		{
			this->Y = (int)val;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_WIDTH:
		{
			this->Width = (int)val;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_HEIGHT:
		{
			this->Height = (int)val;
		}
	break;
	}
	// data has been updated //
	Updated = true;
}

DLLEXPORT float Leviathan::Gui::TextLabel::GetValue(const int &semanticid) const{
	// return semantic data (inverse of SetValue) //
	switch(semanticid){
	case GUI_ANIMATEABLE_SEMANTIC_X:		return (float)this->X;
	case GUI_ANIMATEABLE_SEMANTIC_Y:		return (float)this->Y;
	case GUI_ANIMATEABLE_SEMANTIC_WIDTH:	return (float)this->Width;
	case GUI_ANIMATEABLE_SEMANTIC_HEIGHT:	return (float)this->Height;
	}

	return -1.0f;
}
// ------------------------------------ //
void Leviathan::Gui::TextLabel::_SetHiddenStates(bool states){
	// set whole bridge as hidden //
	RBridge->Hidden = states;
	// these two aren't even required //
	RBridge->SetHidden(0, states);
	RBridge->SetHidden(1, states);
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
	// create object //
	TextLabel* curlabel = new TextLabel(RealID);
	// add to temporary objects //
	tempobjects.push_back(curlabel);

	int x = -36003;
	int y = -36003;
	int width = -36003;
	int height = -36003;

	wstring text(L"");

	Float4 StartColor;
	Float4 EndColor;
	Float4 TextColor;

	float TextSize = 1.0f;
	bool AutoAdjust = true;
	wstring Font = L"arial";

	vector<bool> AreIndexes;
	vector<shared_ptr<VariableBlock>> ListenIndexes;

	// get values for initiation //
	for(size_t a = 0; a < dataforthis.Contents.size(); a++){
		// check what list is being processed //
		if(dataforthis.Contents[a]->Name == L"params"){
			// get variables //

			ObjectFileProcessor::LoadValueFromNamedVars<int>(dataforthis.Contents[a]->Variables, L"X", x, INT_MAX, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<int>(dataforthis.Contents[a]->Variables, L"Y", y, INT_MAX, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<int>(dataforthis.Contents[a]->Variables, L"Width", width, INT_MAX, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<int>(dataforthis.Contents[a]->Variables, L"Height", height, INT_MAX, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<float>(dataforthis.Contents[a]->Variables, L"TextSizeMod", TextSize, 1.f, true,
				L"TextLabel: LoadFromFileStructure:");

			ObjectFileProcessor::LoadValueFromNamedVars<bool>(dataforthis.Contents[a]->Variables, L"AutoAdjust", AutoAdjust, true, true, 
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

	// init with correct values //
	curlabel->Init(x, y, width, height, text, StartColor, EndColor, TextColor, TextSize, AutoAdjust, Font, &ListenIndexes);
	curlabel->Scripting = shared_ptr<ScriptObject>(dataforthis.CreateScriptObjectAndReleaseThis(0, GOBJECT_TYPE_TEXTLABEL));

	// succeeded //
	return true;
}

//void TextLabel::CalculateRelativePositions(){
//	if(!AreAbsolutePos){
//
//		RelaTivedTextMod = ResolutionScaling::ScaleTextSize(TextMod);
//
//		RelativedPadding = ResolutionScaling::ScaleAbsoluteXToFactor(TextPadding);
//		RelativedYPadding = ResolutionScaling::ScaleAbsoluteYToFactor(TextPaddingY);
//
//		RelativedX = ResolutionScaling::ScalePromilleToFactorX(X);
//		RelativedY = ResolutionScaling::ScalePromilleToFactorY(Y);
//		RelativedWidth = ResolutionScaling::ScalePromilleToFactorX(Width);
//		RelativedHeight = ResolutionScaling::ScalePromilleToFactorY(Height);
//
//		
//
//	}
//}
