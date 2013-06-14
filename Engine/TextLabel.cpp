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

//#include "TextLabelCalls.h"

TextLabel::TextLabel(int id){
	Updated = false;
	BridgeCreated = false;
	QuadCreated = false;
	TextCreated = false;
	TextHidden = false;
	OldHidden = false;

	LText = L"NOINIT";
	X = 0;
	Y = 0;
	Width = 20;
	Height = 5;
	Hidden = false;

	ID = id;

	ObjectLevel = GUI_OBJECT_LEVEL_ANIMATEABLE;

	Objecttype = GOBJECT_TYPE_TEXTLABEL;
	Zorder = 1;

	RelativedX = -1;
	RelativedY = -1;
	RelativedWidth = -1;
	RelativedHeight = -1;

	RelaTivedTextMod = -1.f;

	//TextCreated;
	TextID = IDFactory::GetID();
	TextLength = -1;
	Font = L"";

	RelativedPadding = 0;
	RelativedYPadding = 0;

	//SpesData = new int();
	Queue = vector<AnimationAction*>();

	TextPadding = 5;
	TextPaddingY = 3;

	AreAbsolutePos = false;
}
TextLabel::~TextLabel(){
	// release rendering stuff //
	if(RBridge.get() != NULL)
		RBridge->WantsToClose = true;
	RBridge.reset();
}
//ScriptCaller* TextLabel::StaticCall = NULL;
// ------------------------------------ //
bool TextLabel::Init(int xpos, int ypos, /*bool isposabsolute,*/ int width, int height, wstring text, Float4 color1, Float4 color2, Float4 textcolor, 
	float sizemod, bool autoadjust, wstring font, int autofetch)
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

	//AreAbsolutePos = isposabsolute;
	AreAbsolutePos = false;

	Font = font;
	TextHidden = false;
	OldHidden = false;
	Hidden = false;
	TextCreated = false;
	AutoAdjust = autoadjust;

	TextMod = sizemod;
	RelaTivedTextMod = ResolutionScaling::ScaleTextSize(TextMod);

	if(AutoAdjust)
		 SizeAdjust();

	if(autofetch > -1){
		AutoFetch = true;
		StartMonitoring(autofetch, false);
	}
	// register //
	this->RegisterForEvent(EVENT_TYPE_HIDE);
	this->RegisterForEvent(EVENT_TYPE_SHOW);

	return true;
}
void TextLabel::Release(Graphics* graph){
	// send close message //
	// set bridge to die
	if(RBridge.get() != NULL)
		(*RBridge).WantsToClose = TRUE;
	RBridge.reset();

	// this shouldn't be required anymore with StopMonitoring //
	//this->UnRegister(EVENT_TYPE_ALL, true);
	StopMonitoring(-1, L"", true);
}
// ------------------------------------ //
void TextLabel::Render(Graphics* graph){
	NoNUpdatedFrames++;
	if((!ValuesUpdated) && (!Updated) && (OldHidden == Hidden) && (NoNUpdatedFrames < GUI_RENDERABLE_FORCEUPDATE_EVERY_N_FRAMES))
		return;
	Updated = false;

	NoNUpdatedFrames = 0;

	//// calculate relative values for use when not in absolute pos mode //
	//CalculateRelativePositions();



	// check bridge creation //
	if(!BridgeCreated){
		BridgeCreated = true;

		RenderBridge* temppp = NULL;
		temppp = new RenderBridge(this->ID, this->Hidden, this->Zorder);

		RBridge = shared_ptr<RenderBridge>(temppp);

		// submit //
		graph->SubmitRenderBridge(RBridge); // this hopefully copies the bridge and maintains the second copy

		// create quad render action //
		//if(AreAbsolutePos){
			(*RBridge).DrawActions.push_back(new ColorQuadRendBlob(1,0, Int2(X,Y), Colour1, Colour2, Width, Height, 
				COLOR_QUAD_COLOR_STYLE_LEFT_TOP_RIGHT_BOTTOM, false));
		//} else {
		//	(*RBridge).DrawActions.push_back(new ColorQuadRendBlob(1,0, Int2(RelativedX,RelativedY), Colour1, Colour2, RelativedWidth, RelativedHeight,
		//		COLOR_QUAD_COLOR_STYLE_LEFT_TOP_RIGHT_BOTTOM, AreAbsolutePos));
		//}
	}
	// send text create if not created //
	if(!TextCreated){
		TextCreated = true;
		// create text rendering part into bridge //
		//if(AreAbsolutePos){
			(*RBridge).DrawActions.push_back(new BasicTextRendBlob(5,1, Int2(X+TextPadding, Y+TextPaddingY), TextColour, TextMod, LText, 
				false, Font));
		//} else {
		//	//(*RBridge).DrawActions.push_back(new BasicTextRendBlob(5,1, Int2(RelativedX+RelativedPadding,RelativedY+RelativedYPadding), TextColour,
		//	//	TextMod, LText, AreAbsolutePos, Font));
		//	(*RBridge).DrawActions.push_back(new BasicTextRendBlob(5,1, Int2(RelativedX+RelativedPadding,RelativedY+RelativedYPadding), TextColour,
		//		RelaTivedTextMod, LText, AreAbsolutePos, Font));


		//}
		//graph->SubmitAction(new RenderAction(RENDERACTION_TEXT, NULL, NULL, L"", RENDERACTION_CREATE, this->ID, this->TextID, /* estimate max length */new int(LText.size()*1.5f)));
	}

	if(ValuesUpdated){
		// received listener value //
		
		// even if there are multiple values just use one and pop the rest //

		// get values //
		int ivalue = -1;
		wstring wvalue = L"";

		UpdatedValues[0]->GetValue(ivalue, wvalue);

		// check is it int or string //
		if(UpdatedValues[0]->IsIntValue()){

			LText = Convert::IntToWstring(ivalue);

		} else {
			LText = wvalue;
		}

		_PopUdated();

		if(AutoAdjust){
			SizeAdjust();
			//CalculateRelativePositions();
		}
	}

	// send messages //
	if((Hidden) && (!OldHidden)){
		OldHidden = true;
		// send hide messages //
		//graph->SubmitAction(new RenderAction(RENDERACTION_HIDDEN, NULL, NULL, L"", 0, this->ID, 0, NULL));
		//if(!TextHidden){

		//	graph->SubmitAction(new RenderAction(RENDERACTION_TEXT, NULL, NULL, L"", RENDERACTION_HIDDEN, this->ID, this->TextID, new int(1)));
		//	TextHidden = true;
		//}
		(*RBridge).Hidden = true;
		// these two aren't even required //
		(*RBridge).SetHidden(0, true);
		(*RBridge).SetHidden(1, true);

		return;
	} else if ((!Hidden) && (OldHidden)){
		OldHidden = false;

		//// text //
		//if(TextHidden){

		//	graph->SubmitAction(new RenderAction(RENDERACTION_TEXT, NULL, NULL, L"", RENDERACTION_SHOW, this->ID, this->TextID, new int(0)));
		//	TextHidden = false;
		//}
		(*RBridge).Hidden = false;
		// these two aren't even required //
		(*RBridge).SetHidden(0, false);
		(*RBridge).SetHidden(1, false);
	}
	//vector<Float4>* colors = new vector<Float4>();
	//vector<float>* points = new vector<float>();

	// base //
	if(!Hidden){
		// update this Zorder //
		(*RBridge).ZVal = this->Zorder;

		// update it //
		ColorQuadRendBlob* tempuptr;
		int Index = (*RBridge).GetSlotIndex(0);
		if(Index < 0){
			BridgeCreated = false;
		}
		tempuptr = reinterpret_cast<ColorQuadRendBlob*>((*RBridge).DrawActions[Index]);

		//if(AreAbsolutePos){
			tempuptr->Update(1, Int2(X,Y), Colour1, Colour2, Width, Height, COLOR_QUAD_COLOR_STYLE_LEFT_TOP_RIGHT_BOTTOM, false);
		//} else {
		//	tempuptr->Update(1, Int2(RelativedX,RelativedY), Colour1, Colour2, RelativedWidth, RelativedHeight, 
		//		COLOR_QUAD_COLOR_STYLE_LEFT_TOP_RIGHT_BOTTOM, AreAbsolutePos);
		//}

//int relativez, Int2 &xypos, Float4 &color, Float4 &color2, int width, int height, int colortranstype, bool absolute = false
		//colors->push_back(Colour1);
		//colors->push_back(Colour2);
		//points->push_back(X);
		//points->push_back(Y);
		//points->push_back(Width);
		//points->push_back(Height);
		//graph->SubmitAction(new RenderAction(RENDERACTION_SQUARE, points, colors, L"", 0, this->ID, 0, NULL));
	}
	if(!TextHidden){
		// update it //
		BasicTextRendBlob* tempuptr;
		int Index = (*RBridge).GetSlotIndex(1);
		if(Index < 0){
			TextCreated = false;
		}
		tempuptr = reinterpret_cast<BasicTextRendBlob*>((*RBridge).DrawActions[Index]);

		//if(AreAbsolutePos){
			tempuptr->Update(5, Int2(X+TextPadding,Y+TextPaddingY), TextColour, TextMod, LText, false, Font);
		//} else {
		//	//tempuptr->Update(5, Int2(RelativedX+RelativedPadding,RelativedY+RelativedYPadding), TextColour, TextMod, LText, AreAbsolutePos, Font);
		//	tempuptr->Update(5, Int2(RelativedX+RelativedPadding,RelativedY+RelativedYPadding), TextColour, RelaTivedTextMod, LText, AreAbsolutePos, Font);
		//}
//int relativez, Int2 &xypos, Float4 &color, float sizemod, wstring text, bool absolute = false, wstring font = L"Arial"
		// update text //
		//colors = new vector<Float4>();
		//points = new vector<float>();

		//colors->push_back(TextColour);
		//points->push_back(X+TextPadding);
		//points->push_back(Y+TextPaddingY);
		//// size
		//points->push_back(TextMod);

		//graph->SubmitAction(new RenderAction(RENDERACTION_TEXT, points, colors, LText, RENDERACTION_UPDATE, this->ID, this->TextID, NULL, Font));
	}

	//graph->SubmitAction(new RenderAction(RENDERACTION_HIDDEN, vector<float>(), vector<Float4>(), L"", 0, this->ID, 1));
}
// ------------------------------------ //
void TextLabel::UpdateColours(Float4 color1, Float4 color2, Float4 textcolor){
	Updated = true;
	TextColour = textcolor;
	Colour1 = color1;
	Colour2 = color2;

}
void TextLabel::Update(int xpos, int ypos, int width, int height, bool autoadjust, wstring text){
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
void TextLabel::SetHiddenState(bool hidden){
	Updated = true;
	this->Hidden = hidden;
}
// ------------------------------------ //


// ------------------------------------ //
void TextLabel::SizeAdjust(){

	// get text length //
	int textlength = -1;
	textlength = GuiManager::Get()->GetGraph()->CountTextRenderLength(LText, Font, TextMod, false);


	// count new size //


	Width = TextPadding*2+textlength;


	Height = TextPaddingY*2+GuiManager::Get()->GetGraph()->GetTextRenderHeight(Font, TextMod, false);

}
//void TextLabel::SizeAdjust(){
//	// calculate paddings //
//	if(!AreAbsolutePos){
//		RelativedPadding = ResolutionScaling::ScaleAbsoluteXToFactor(TextPadding);
//		RelativedYPadding = ResolutionScaling::ScaleAbsoluteYToFactor(TextPaddingY);
//	}
//
//	// get text length //
//	int textlength = -1;
//	if(AreAbsolutePos){
//		textlength = GuiManager::Get()->GetGraph()->CountTextRenderLength(LText, Font, TextMod, AreAbsolutePos);
//	} else {
//		textlength = GuiManager::Get()->GetGraph()->CountTextRenderLength(LText, Font, RelaTivedTextMod, AreAbsolutePos);
//	}
//
//	// count new size //
//
//	if(AreAbsolutePos){
//		Width = TextPadding*2+textlength;
//	} else {
//		Width = RelativedPadding*2+textlength;
//	}
//	if(AreAbsolutePos){
//		Height = TextPaddingY*2+GuiManager::Get()->GetGraph()->GetTextRenderHeight(Font, TextMod, AreAbsolutePos);
//	} else {
//		Height = RelativedYPadding*2+GuiManager::Get()->GetGraph()->GetTextRenderHeight(Font, RelaTivedTextMod, AreAbsolutePos);
//	}
//	CalculateRelativePositions();
//}

 // ------------------------------------ //
int TextLabel::AnimationTime(int mspassed){
	if(Queue.size() == 0)
		return 0;
	bool contaction = false;

	for(unsigned int i = 0; i < Queue.size(); i++){
		contaction = Queue[i]->AllowSimultanous;

		if(Queue[i]->Type == GUI_ANIMATION_HIDE){
			this->Hidden = true;
			Updated = true;

			delete Queue[i];
			Queue.erase(Queue.begin()+i);
			i--;
			break;
		}
		if(Queue[i]->Type == GUI_ANIMATION_SHOW){
			this->Hidden = false;
			Updated = true;

			delete Queue[i];
			Queue.erase(Queue.begin()+i);
			i--;
			break;
		}

		if(GuiManager::Get()->HandleAnimation(Queue[i], this, mspassed) == 1){
			// event completed //
			
			delete Queue[i];
			Queue.erase(Queue.begin()+i);
			i--;
		}

		if(!contaction) // break if simultaneous flag is not set
			break;

	}

	if(Queue.size() == 0){
		AnimationFinish();
	}

	return 0;
	//return GuiManager::Get()->HandleAnimation(NULL, this, mspassed);
}

void TextLabel::AnimationFinish(){

}
void TextLabel::QueueAction(AnimationAction* act){
	Queue.push_back(act);
}
//void TextLabel::QueueActionForObject(TextLabel* object, AnimationAction* action){
//	object->Queue.push_back(action);
//}
// ------------------------------------ //
int TextLabel::OnEvent(Event** pEvent){
	// figure what to do based on type
	switch((*pEvent)->Type){
	case EVENT_TYPE_HIDE:
		{
			// run specific script if found //
			if(Scripting->Script != NULL){
				if(Scripting->Script->Instructions.size() < 1)
					return 0;
					// test some stuff //
					//ScriptArguement* arg1 = new ScriptArguement(new WstringBlock(L"25"), DATABLOCK_TYPE_WSTRING, true);
					//wstring lolly = *(wstring*)(*arg1);

				vector<ScriptNamedArguement*> Params;
				Params.push_back(new ScriptNamedArguement(L"Source", new IntBlock((*(Int2*)(*pEvent)->Data)[0]), DATABLOCK_TYPE_INT, false, true));
				Params.push_back(new ScriptNamedArguement(L"InstanceID", new IntBlock(this->ID), DATABLOCK_TYPE_INT, false, true)); // OLD:needs to be false to prevent deleting THIS! object

				shared_ptr<ScriptArguement> returned = shared_ptr<ScriptArguement>(ScriptInterface::Get()->ExecuteIfExistsScript(Scripting, L"OnHide", Params, GetCallerForObjectType(this), false));
				// delete parameters //
				while(Params.size() != 0){
					SAFE_DELETE(Params[0]); // this should not have been deleted by the scripting engine //
					Params.erase(Params.begin());
				}

				// check did it exist //
				int Value = (int)(*returned.get());
				if(Value == 80000802){
					// script didn't exist //
					return 0;
				}

				return Value;
				//Gui_QueueAnimationActionMove(this->ID, 5,0, GUI_ANIMATION_TYPEMOVE_PRIORITY_X, 1.6f);
				//Gui_QueueAnimationActionVisibility(this->ID, false);

				//return 1;
			} else {
				return 0;
			}

			//Queue.push_back(new AnimationAction(GUI_ANIMATION_MOVE, new GuiAnimationTypeMove(10, 2,GUI_ANIMATION_TYPEMOVE_PRIORITY_Y, 0.1f), 0, false));
			//Queue.push_back(new AnimationAction(GUI_ANIMATION_HIDE, NULL, 0, false));
		}
	break;
	case EVENT_TYPE_SHOW:
		{
			// run specific script if found //
			if(Scripting->Script != NULL){
				if(Scripting->Script->Instructions.size() < 1)
					return 0;
					// test some stuff //
					//ScriptArguement* arg1 = new ScriptArguement(new WstringBlock(L"25"), DATABLOCK_TYPE_WSTRING, true);
					//wstring lolly = *(wstring*)(*arg1);

				vector<ScriptNamedArguement*> Params;
				Params.push_back(new ScriptNamedArguement(L"Source", new IntBlock((*(Int2*)(*pEvent)->Data)[0]), DATABLOCK_TYPE_INT, false, true));
				Params.push_back(new ScriptNamedArguement(L"InstanceID", new IntBlock(this->ID), DATABLOCK_TYPE_INT, false, true)); // OLD:needs to be false to prevent deleting THIS! object

				shared_ptr<ScriptArguement> returned = shared_ptr<ScriptArguement>(ScriptInterface::Get()->ExecuteIfExistsScript(Scripting, L"OnShow",
					Params, GetCallerForObjectType(this), false));
				// delete parameters //
				while(Params.size() != 0){
					SAFE_DELETE(Params[0]); // this should not have been deleted by the scripting engine //
					Params.erase(Params.begin());
				}

				// check did it exist //
				int Value = (int)(*returned.get());
				if(Value == 80000802){
					// script didn't exist //
					return 0;
				}

				return Value;
				//Gui_QueueAnimationActionVisibility(this->ID, true);
				//Gui_QueueAnimationActionMove(this->ID, 150,200, GUI_ANIMATION_TYPEMOVE_PRIORITY_Y, 1.6f);

				//return 1;
			} else {
				return 0;
			}

			// add nice animation //
			//Queue.push_back(new AnimationAction(GUI_ANIMATION_MOVE, new GuiAnimationTypeMove(10, 2,GUI_ANIMATION_TYPEMOVE_PRIORITY_Y, 0.1f), 0, false));
			//Queue.push_back(new AnimationAction(GUI_ANIMATION_HIDE, NULL, 0, false));

			//return 1;
		}
	break;

	}

	// not used request unregistration
	return -1;
}

// ------------------------------------ //


void TextLabel::SetValue(int semanticid, float val){

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
	Updated = true;

	return;
}
float TextLabel::GetValue(int semanticid){
	switch(semanticid){
	case GUI_ANIMATEABLE_SEMANTIC_X:
		{
			return (float)this->X;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_Y:
		{
			return (float)this->Y;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_WIDTH:
		{
			return (float)this->Width;
		}
	break;
	case GUI_ANIMATEABLE_SEMANTIC_HEIGHT:
		{
			return (float)this->Height;
		}
	break;
	}
	return -1.0f;
}


// ------------------------------------ //
//ScriptCaller* TextLabel::GetCallerForObjectType(TextLabel* customize){
//	if(StaticCall == NULL){
//		// generate new //
//		StaticCall = new ScriptCaller();
//		// push specific functions //
//		//StaticCall->RegisterFunction(L"TextLabel::SetText", SetText);
//	}
//	// customize certain functions //
//	return StaticCall;
//}

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
