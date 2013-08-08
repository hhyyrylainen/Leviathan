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

#include "DataBlock.h"
#include "ObjectBackground.h"
#include "GuiBasicText.h"
#include "DebugVariableNotifier.h"
#include <boost\assign\list_of.hpp>
#include "ObjectFileProcessor.h"


DLLEXPORT Leviathan::Gui::TextLabel::TextLabel(GuiManager* owner, const int &id, const bool &hidden, const Float2 &position, Float2 &size, 
	const int &autoadjust, shared_ptr<ScriptScript> script) : BaseGuiObject(owner, TEXTLABEL_OBJECTFLAGS, id, GUIOBJECTTYPE_TEXTLABEL, script), 
	AutoUpdateableObject(), GuiAnimateable(), RenderableGuiObject(0, TEXTLABEL_GRAPHICALCOMPONENT_COUNT, hidden), Positionable(position, size), 
	BaseGuiEventable(owner), TextPadding(0, 0)
{
	// set some default values //
	AutoAdjust = 2;
	TextAdjustMode = 1;

	TextWantedCutSize = 0.4f;
}

Leviathan::Gui::TextLabel::~TextLabel(){
	// don't listen to anything anymore //
	StopMonitoring(MonitoredValues);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::TextLabel::Init(const wstring &text, const wstring &font, const Float4 &textcolor, const float &textsize, 
	const Float2 &padding, const float &textscalecut, shared_ptr<NamedVariableList> backgroundgen, 
	vector<shared_ptr<VariableBlock>>* listenindexes /*= NULL*/)
{
	// freshly generated and we need to make sure that update happens before rendering //
	Updated = true;

	// copy values //
	TextPadding = padding;
	TextWantedCutSize = textscalecut;

	// initialize graphical components //
	assert(GComponents.size() == 2 && "g components should be right");


	// background //
	GComponents[0] = new ObjectBackground(0, 1);
	if(!GComponents[0]){

		QUICK_MEMORY_ERROR_MESSAGE;
		return false;
	}

	// initialize it //
	if(!((ObjectBackground*)GComponents[0])->Init(backgroundgen)){

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

	if(!((GuiBasicText*)GComponents[1])->Init(textcolor, text, font, textsize, TextAdjustMode != GUI_BASICTEXT_MODE_JUSTRENDER, TextAdjustMode, 
		TextWantedCutSize))
	{

		QUICK_ERROR_MESSAGE;
		return false;
	}
	// we want to adjust size automatically before first rendering if it is set //
	if(AutoAdjust)
		 SizeAdjust();

	if(listenindexes){
		// start monitoring //
		StartMonitoring(*listenindexes);
	}
	// register //
	RegisterForEvent(EVENT_TYPE_HIDE);
	RegisterForEvent(EVENT_TYPE_SHOW);

	// set size to text to ensure that it is set //
	//((GuiBasicText*)GComponents[1])->SetLocationData(TextWantedCoordinates, TextAreaSize);
	((ObjectBackground*)GComponents[0])->SetLocationData(Position, Size);

	return true;
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
	if(!VerifyRenderingBridge(graph, ID)){

		DEBUG_BREAK;
		return;
	}
	assert(GComponents.size() == 2 && "non initialized TextLabel");

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

	// ensure that size is properly calculated //
	SizeAdjust();

	// update bride's draw order //
	RBridge->ZVal = this->Zorder;
	
	// update components //
	// background will always use same location and size as this //
	((ObjectBackground*)GComponents[0])->SetLocationData(Position, Size);
	
	// render graphical components //
	GComponents[0]->Render(RBridge.get(), graph);
	GComponents[1]->Render(RBridge.get(), graph);
}
// ------------------------------------ //
void Leviathan::Gui::TextLabel::SizeAdjust(){

	if(AutoAdjust == 1){
		// adjust to text's length //
		// get text length //
		float length = 0;
		float heigth = 0;

		((GuiBasicText*)GComponents[1])->GetTextLength(length, heigth);

		// count new size //
		Size.X = TextPadding.X*2+length;
		Size.Y = TextPadding.Y*2+heigth;

		// calculate area for text //
		Float2 TextWantedCoordinates = Float2(Position.X+TextPadding.X, Position.Y+TextPadding.Y);
		Float2 TextAreaSize = Float2(Size.X-(TextPadding.X*2), Size.Y-(TextPadding.Y*2));

		// send new size to background //
		((ObjectBackground*)GComponents[0])->SetSize(Size);

		// set position to text //
		((GuiBasicText*)GComponents[1])->SetLocationData(TextWantedCoordinates, TextAreaSize);

		return;
	}
	// using text to fit box mode or no adjustment (in which case the text will still need these variables) //
	Float2 TextWantedCoordinates = Float2(Position.X+TextPadding.X, Position.Y+TextPadding.Y);
	Float2 TextAreaSize = Float2(Size.X-(TextPadding.X*2), Size.Y-(TextPadding.Y*2));

	// set positions to text //
	((GuiBasicText*)GComponents[1])->SetLocationData(TextWantedCoordinates, TextAreaSize);
}
 // ------------------------------------ //
int Leviathan::Gui::TextLabel::AnimationTime(int mspassed){
	// just call default and be done with it //
	return _RunAnimationTimeDefault(OwningInstance, mspassed);
}

void Leviathan::Gui::TextLabel::AnimationFinish(){
	// call script event if script wants to receive //
}
// ------------------------------------ //
int Leviathan::Gui::TextLabel::OnEvent(Event** pEvent){
	// script listeners that are static //

	const EVENT_TYPE &etype = (*pEvent)->Type;

	// check is this event type a event that we can forward to scripts //
	if(etype == EVENT_TYPE_HIDE || etype == EVENT_TYPE_SHOW){
		// let's try to pass this to scripts //
		wstring calllistenername(L"");

		switch(etype){
			case EVENT_TYPE_SHOW: calllistenername = LISTENERNAME_ONHIDE; break;
			case EVENT_TYPE_HIDE: calllistenername = LISTENERNAME_ONSHOW; break;
			default:
				return 0;
		}

		// check does right listener exist //
		if(!Scripting->GetModule()->DoesListenersContainSpecificListener(calllistenername)){
			// no valid listener //
			Logger::Get()->Info(L"TextLabel: missing listener : "+calllistenername);
			return 0;
		}

		vector<shared_ptr<NamedVariableBlock>> Params = boost::assign::list_of(new NamedVariableBlock(new IntBlock((*(Int2*)(*pEvent)->Data)[0]), L"Source"))
			(new NamedVariableBlock(new VoidPtrBlock(this), L"TextLabel"));
		// we need to add reference to ourselves to not get deleted by the script instance releasing //
		AddRef();


		// create parameters //
		ScriptRunningSetup ssetup;
		ssetup.SetEntrypoint(Convert::WstringToString(calllistenername)).SetUseFullDeclaration(false).SetArguements(Params);

		shared_ptr<VariableBlock> returned = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &ssetup);

		// script's exit value //
		return int(*returned.get());

	}

	// not used, request unregistration //
	return -1;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::TextLabel::SetValue(const int &semanticid, const float &val){
	// switch to right value //
	switch(semanticid){
	case GUI_ANIMATEABLE_SEMANTIC_X: Position.X = val; break;
	case GUI_ANIMATEABLE_SEMANTIC_Y: Position.Y = val; break;
	case GUI_ANIMATEABLE_SEMANTIC_WIDTH: Size.X = val; break;
	case GUI_ANIMATEABLE_SEMANTIC_HEIGHT: Size.Y = val; break;
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
DLLEXPORT bool Leviathan::Gui::TextLabel::LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects, vector<Int2> &idmappairs, 
	ObjectFileObject& dataforthis)
{
	// try to load a TextLabel from the structure //
	// get a new id for the new object //
	int RealID = IDFactory::GetID();

	// find the "fake" id which is used to assign this to parent //
	for(size_t a = 0; a < dataforthis.Prefixes.size(); a++){
		if(Misc::WstringStartsWith(*dataforthis.Prefixes[a], L"ID")){
			// use wstring iterator to get the id number //
			WstringIterator itr(dataforthis.Prefixes[a], false);

			unique_ptr<wstring> itrresult = itr.GetNextNumber(DECIMALSEPARATORTYPE_NONE);

			// check result word //
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

	Float2 Location(0, 0);
	Float2 Size(0, 0);
	Float2 Padding(0, 0);

	wstring text(L"");
	wstring Font(L"");

	float TextSize;
	Float4 TextColor;

	int AutoAdjust;
	bool Hidden;
	float TextCutScale = 0.4f;

	shared_ptr<NamedVariableList> BackgroundInfo(NULL);

	vector<shared_ptr<VariableBlock>> ListenIndexes;

	ObjectFileList* paramlist = NULL;

	// get values for initiation //
	for(size_t a = 0; a < dataforthis.Contents.size(); a++){
		// check what list is being processed //
		if(dataforthis.Contents[a]->Name == L"params"){
			// found the parameter list //
			paramlist = dataforthis.Contents[a];
			break;
		}
	}

	if(paramlist == NULL){
		// couldn't find it //

		Logger::Get()->Error(L"TextLabel: LoadFromFileStructure: variable list \"params\" wasn't found");
		return false;
	}


	// Get variables from the parameter object //

	// get variables //
	ObjectFileProcessor::LoadValueFromNamedVars<float>(paramlist->Variables, L"TextSizeMod", TextSize, 1.f, true,
		L"TextLabel: LoadFromFileStructure:");

	ObjectFileProcessor::LoadValueFromNamedVars<int>(paramlist->Variables, L"AutoAdjust", AutoAdjust, 1, true, 
		L"TextLabel: LoadFromFileStructure:");

	ObjectFileProcessor::LoadValueFromNamedVars<float>(paramlist->Variables, L"TextCutScale", TextCutScale, 0.4f, false);
	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(paramlist->Variables, L"StartText", text, L"", false);
	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(paramlist->Variables, L"Font", Font, L"Arial", false);
	ObjectFileProcessor::LoadValueFromNamedVars<bool>(paramlist->Variables, L"Hidden", Hidden, false, false);
	

	ObjectFileProcessor::LoadMultiPartValueFromNamedVars<Float4, float, 4>(paramlist->Variables, L"TextColor", TextColor,
		Float4::ColourWhite, true, L"TextLabel: LoadFromFileStructure:");

	ObjectFileProcessor::LoadMultiPartValueFromNamedVars<Float2, float, 2>(paramlist->Variables, L"Location", Location,
		Float2(0, 0), true, L"TextLabel: LoadFromFileStructure:");

	ObjectFileProcessor::LoadMultiPartValueFromNamedVars<Float2, float, 2>(paramlist->Variables, L"Size", Size,
		Float2(0, 0), true, L"TextLabel: LoadFromFileStructure:");

	ObjectFileProcessor::LoadMultiPartValueFromNamedVars<Float2, float, 2>(paramlist->Variables, L"Padding", Padding,
		Float2(0, 0), true, L"TextLabel: LoadFromFileStructure:");

	BackgroundInfo = paramlist->Variables.GetValueDirect(L"Background");

	// listen on should allow strings and multiple numbers //
	try{
		vector<VariableBlock*>* listenvalues = paramlist->Variables.GetValues(L"ListenOn");

		// check values //
		for(size_t i = 0; i < listenvalues->size(); i++){
			// check is it wstring or int //

			if(listenvalues->at(i)->GetBlock()->Type == DATABLOCK_TYPE_INT){

				// int index //
				ListenIndexes.push_back(shared_ptr<VariableBlock>(new VariableBlock(listenvalues->at(i)->GetBlock()->AllocateNewFromThis())));

			} else {
				// skip if cannot be made into wstring //
				if(!listenvalues->at(i)->IsConversionAllowedPtr<wstring>()){
					continue;
				}

				// is a string index //
				ListenIndexes.push_back(shared_ptr<VariableBlock>(new VariableBlock(listenvalues->at(i)->GetBlock()->AllocateNewFromThis())));
			}
		}
	}
	catch(...){
		// nothing to listen on //
		ListenIndexes.clear();
		Logger::Get()->Warning(L"TextLabel: LoadFromFileStructure: listen index parsing caused an exception, not listening for anything");
	}

	// create object //
	TextLabel* curlabel = new TextLabel(owner, RealID, Hidden, Location, Size, AutoAdjust, dataforthis.Script);
	// add to temporary objects //
	tempobjects.push_back(curlabel);

	// init with correct values //
	curlabel->Init(text, Font, TextColor, TextSize, Padding, TextCutScale, BackgroundInfo, &ListenIndexes);

	// succeeded //
	return true;
}
// ------------------------------------ //
void Leviathan::Gui::TextLabel::_SetHiddenStates(bool states){
	// set whole bridge as hidden //
	RBridge->Hidden = states;
}

void Leviathan::Gui::TextLabel::_OnLocationOrSizeChange(){
	Updated = true;
}
// ------------------------------------ //
int Leviathan::Gui::TextLabel::_RunAnimationTimeDefault(GuiManager* owner, const int &mspassed){
	if(AnimationQueue.size() == 0)
		return 0;
	bool contaction = false;

	for(size_t i = 0; i < AnimationQueue.size(); i++){
		// save simultaneous flag //
		contaction = AnimationQueue[i]->AllowSimultanous;

		if(owner->HandleAnimation<TextLabel>(AnimationQueue[i], this, mspassed) == 1){
			// event completed //

			RemoveActionFromQueue(AnimationQueue[i]);
			i--;
		}

		// break if simultaneous flag is not set //
		if(!contaction)
			break;
	}
	if(AnimationQueue.size() == 0){
		AnimationFinish();
	}

	return 0;
}
