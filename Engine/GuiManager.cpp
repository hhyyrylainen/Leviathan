#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_MAIN
#include "GuiManager.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include "Engine.h"
#include "ScriptInterface.h"
#include "FileSystem.h"

#ifdef _DEBUG
//#pragma warning (disable : 4800)
#endif

Leviathan::GuiManager::GuiManager() : ReceivedPresses(){
	ObjectAmountChanged = false;
	Foreground = NULL;

	MainInput = NULL;

	staticaccess = this;
	GKeyPressesConsumed = true;
}
Leviathan::GuiManager::~GuiManager(){
	// on quit calls //
	SAFE_DELETE(MainInput);
	Release();

	staticaccess = NULL;
}

GuiManager* Leviathan::GuiManager::staticaccess = NULL;
GuiManager* Leviathan::GuiManager::Get(){
	return staticaccess; 
}
// ------------------------------------ //
bool Leviathan::GuiManager::Init(AppDef* vars){

	graph = Engine::GetEngine()->GetGraphics();
	// create press listeners //

	MainInput = new KeyListener(this, Engine::GetEngine()->GetKeyPresManager());
	
	// load main Gui file //
	ExecuteGuiScript(FileSystem::GetScriptsFolder()+L"MainGui.txt");



	return true;
}

void Leviathan::GuiManager::Release(){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->HigherLevel == true){
			((RenderableGuiObject*)Objects[i])->Release();
			delete Objects[i];
		} else {
			SAFE_DELETE(Objects[i]);
		}


		Objects.erase(Objects.begin()+i);
		i--;
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GuiManager::ClearKeyReceivingState(){
	// reset state //
	GKeyPressesConsumed = false;
	// ensure that the vector is empty //
	ReceivedPresses.clear();
}

DLLEXPORT void Leviathan::GuiManager::AddKeyPress(int keyval, InputEvent** originalevent){
	ReceivedPresses.push_back(shared_ptr<GuiReceivedKeyPress>(new GuiReceivedKeyPress(GUI_KEYSTATE_TYPE_KEYPRESS, keyval, originalevent)));
}

DLLEXPORT void Leviathan::GuiManager::AddKeyDown(int keyval, InputEvent** originalevent){
	ReceivedPresses.push_back(shared_ptr<GuiReceivedKeyPress>(new GuiReceivedKeyPress(GUI_KEYSTATE_TYPE_KEYDOWN, keyval, originalevent)));
}

DLLEXPORT bool Leviathan::GuiManager::IsEventConsumed(InputEvent** ev){
	// check is it still in key presses (if it is it hasn't been consumed) //
	for(size_t i = 0; i < ReceivedPresses.size(); i++){

		if(*ReceivedPresses[i]->MatchingEvent == *ev){
			// wasn't consumed, consume now //
			ReceivedPresses.erase(ReceivedPresses.begin()+i);
			return false;
		}
	}
	// not found, has been consumed //
	return true;
}

DLLEXPORT void Leviathan::GuiManager::SetKeyContainedValuesAsConsumed(const GKey &k){
	wchar_t chara = (wchar_t)k.GetCharacter();
	bool Shift, Ctrl, Alt;
	Shift = Ctrl = Alt = false;
	GKey::DeConstructSpecial(k.GetAdditional(), Shift, Alt, Ctrl);

	for(size_t i = 0; i < ReceivedPresses.size(); i++){
		// check does this match anything that the key has //
		if(ReceivedPresses[i]->KeyCode == chara){


		}

	}
}

DLLEXPORT void Leviathan::GuiManager::ProcessKeyPresses(){
	bool Shift = false;
	bool Ctrl = false;
	bool Alt = false;

	// check special keys //
	for(size_t i = 0; i < ReceivedPresses.size(); i++){
		// type doesn't matter here //
		if(ReceivedPresses[i]->KeyCode == VK_SHIFT){
			Shift = true;
			continue;
		}
		if(ReceivedPresses[i]->KeyCode == VK_CONTROL){
			Ctrl = true;
			continue;
		}
		if(ReceivedPresses[i]->KeyCode == VK_MENU){
			Alt = true;
			continue;
		}

	}
	//CallEvent(new Event(EVENT_TYPE_GUIENABLE, NULL));

	//Engine::GetEngine()->SetGuiActive(true);
	// pop action //
	//ReceivedPresses.erase(ReceivedPresses.begin()+i);
	
	if(DataStore::Get()->GetGUiActive()){
		// send presses to objects //

		for(unsigned int i = 0; i < ReceivedPresses.size(); i++){
			if(ReceivedPresses[i]->MatchingEvent != GUI_KEYSTATE_TYPE_KEYPRESS){
				// process key down by sending events //

				continue;
			}
			// is a key press //
			// generate key //
			GKey current = GKey(ReceivedPresses[i]->KeyCode, Leviathan::GKey::ConstructSpecial(Shift, Alt, Ctrl));


		}

	} else {
		// check should Gui turn on //

	}
}

void Leviathan::GuiManager::GuiTick(int mspassed){
	// animations are now in OnAnimationTime
	// send tick event //

}
void Leviathan::GuiManager::AnimationTick(int mspassed){
	// animations //
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->ObjectLevel >= GUI_OBJECT_LEVEL_ANIMATEABLE){
			((GuiAnimateable*)Objects[i])->AnimationTime(mspassed);
		}
	}
}
void Leviathan::GuiManager::Render(){
	UpdateArrays();

	for(unsigned int i = 0; i < NeedRendering.size(); i++){
		NeedRendering[i]->Render(graph);
	}
}
// ------------------------------------ //
void Leviathan::GuiManager::QuickSendPress(int keyinfo){

}

void Leviathan::GuiManager::OnResize(){
	// update objects that by default need this //


	// call events //
	this->CallEvent(new Event(EVENT_TYPE_WINDOW_RESIZE, (void*)new Int2(DataStore::Get()->GetWidth(), DataStore::Get()->GetHeight())));

}

// ------------------------------------ //
bool Leviathan::GuiManager::AddGuiObject(BaseGuiObject* obj){
	ObjectAmountChanged = true;

	Objects.push_back(obj);
	return true;
}

bool Leviathan::GuiManager::AddGuiObject(BaseGuiObject* obj, int collectionid){
	ObjectAmountChanged = true;


	Objects.push_back(obj);

	// add to collection //
	unsigned int index = GetCollection(L"", collectionid);
	ARR_INDEX_CHECK(index,Collections.size()){
		// add
		Collections[index]->children.push_back(obj);
	}
	return true;
}

void Leviathan::GuiManager::DeleteObject(int id){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->ID == id){
			ObjectAmountChanged = true;

			// remove from collections //
			for(unsigned int e = 0; e < Collections.size(); e++){
				for(unsigned int a = 0; a < Collections[e]->children.size(); a++){
					if(Collections[e]->children[a] == Objects[i]){
						Collections[e]->children.erase(Collections[e]->children.begin()+a);
						break;
					}
				}
			}


			if(Objects[i]->HigherLevel == true){
				((RenderableGuiObject*)Objects[i])->Release();
			}
			SAFE_DELETE(Objects[i]);
			Objects.erase(Objects.begin()+i);
			i--;
			break;
		}


	}
}

int Leviathan::GuiManager::GetObjectIndexFromId(int id){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->ID == id)
			return i;

	}
	return -1;
}

BaseGuiObject* Leviathan::GuiManager::GetObject(unsigned int index){
	 ARR_INDEX_CHECK(index, Objects.size()){
		 return Objects[index];
	 }
	 return NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GuiManager::ExecuteGuiScript(const wstring &file){
	vector<shared_ptr<NamedVar>> headerdata;
	vector<shared_ptr<ObjectFileObject>> data = ObjectFileProcessor::ProcessObjectFile(file, headerdata);
	vector<Int2> FakeIDRealID;

	vector<BaseGuiObject*> TempOs;
	vector<Int2> CollectionMember;


	for(unsigned int i = 0; i < data.size(); i++){
		// check what type the object is //
		if(data[i]->TName == L"Collection"){
			int createdid = IDFactory::GetID();

			wstring Toggle = L"";
			int Enabled = true;
			int Visible = true;
			int Strict = false;

			for(unsigned int a = 0; a < data[i]->Contents.size(); a++){
				if(data[i]->Contents[a]->Name == L"members"){
					for(unsigned int tind = 0; tind < data[i]->Contents[a]->Lines.size(); tind++){
						vector<wstring> Words;
						Misc::CutWstring(*data[i]->Contents[a]->Lines[tind], L" ", Words);

						for(unsigned int word = 0; word < Words.size(); word++){
							if(Words[word] == L"ID"){
								if(word+1 < Words.size()){
									int AddID = Convert::WstringToInt(Words[word+1]);
									CollectionMember.push_back(Int2(createdid, AddID));
									break;
								}
							}

						}
					}
					continue;
				}
				if(data[i]->Contents[a]->Name == L"params"){

					data[i]->Contents[a]->Variables->GetValue(L"ToggleOn", Toggle);
					data[i]->Contents[a]->Variables->GetValue(L"Enabled", Enabled);
					data[i]->Contents[a]->Variables->GetValue(L"Visible", Visible);
					data[i]->Contents[a]->Variables->GetValue(L"Strict", Strict);

					continue;
				}

			}
			GuiCollection* cobj = new GuiCollection(data[i]->Name, createdid, Visible != 0, Toggle, Strict != 0, Enabled != 0);

			CreateCollection(cobj);
			cobj->Scripting = shared_ptr<ScriptObject>(data[i]->CreateScriptObjectAndReleaseThis(0, GOBJECT_TYPE_TEXTLABEL));
			// this function should have deleted everything related to that object, so it should be safe to just erase it //
			data.erase(data.begin()+i);
			i--;

			continue;
		}
		if(data[i]->TName == L"TextLabel"){
			int TID = IDFactory::GetID();
			for(size_t a = 0; a < data[i]->Prefixes.size(); a++){
				if(Misc::WstringStartsWith(*data[i]->Prefixes[a], L"ID")){
					// this is a token line, split it //
					vector<Token*> tokens;
					LineTokeNizer::SplitTokenToRTokens(*data[i]->Prefixes[a], tokens);

					// check size //
					if(tokens.size() != 2){
						// invalid id //
						DEBUG_BREAK;
					}

					FakeIDRealID.push_back(Int2(Convert::WstringToInt(tokens[1]->GetChangeableData()), TID));

					// don't forget to release memory //
					SAFE_DELETE_VECTOR(tokens);
					break;
				}
			}
			// create object //
			TextLabel* curlabel = new TextLabel(TID);
			// add to temporary objects //
			TempOs.push_back(curlabel);

			int x = -36003;
			int y = -36003;
			int width = -36003;
			int height = -36003;

			wstring text(L"");

			Float4 StartColor;
			Float4 EndColor;
			Float4 TextColor;

			float TextSize = 1.0f;
			int AutoAdjust = true;
			wstring Font = L"font";
			int ListenOn = -1;

			// get values for initiation //
			for(size_t a = 0; a < data[i]->Contents.size(); a++){
				// check what list is being processed //
				if(data[i]->Contents[a]->Name == L"params"){
					int tmpi = 0;
					// get variables //
					data[i]->Contents[a]->Variables->GetValue(L"X", x);
					data[i]->Contents[a]->Variables->GetValue(L"Y", y);

					data[i]->Contents[a]->Variables->GetValue(L"Width", width);
					data[i]->Contents[a]->Variables->GetValue(L"Height", height);

					data[i]->Contents[a]->Variables->GetValue(L"TextSizeMod", tmpi);
					TextSize = (float)tmpi/100;
					data[i]->Contents[a]->Variables->GetValue(L"AutoAdjust", AutoAdjust);
					data[i]->Contents[a]->Variables->GetValue(L"ListenOn", ListenOn);

					data[i]->Contents[a]->Variables->GetValue(L"StartText", text);
					data[i]->Contents[a]->Variables->GetValue(L"Font", Font);

					for(int tval = 0; tval < 3; tval++){
						// a job for tokens //
						vector<Token*> tokens;

						// split tokens from right source //
						if(tval == 0)
							LineTokeNizer::SplitTokenToRTokens(*data[i]->Contents[a]->Variables->ReturnValue(L"StartColor"), tokens);
						if(tval == 1)
							LineTokeNizer::SplitTokenToRTokens(*data[i]->Contents[a]->Variables->ReturnValue(L"EndColor"), tokens);
						if(tval == 2)
							LineTokeNizer::SplitTokenToRTokens(*data[i]->Contents[a]->Variables->ReturnValue(L"TextColor"), tokens);

						if(tokens.size() != 2){
							// invalid definition //
							DEBUG_BREAK;
						}

						// get values from second token //
						WstringIterator itr(&tokens[1]->GetChangeableData(), false);

						float Vals[] = {0.0f, 0.0f, 0.0f, 0.0f};

						// iterate 4 decimal values //
						for(int loopitrfloat = 0; loopitrfloat < 4; loopitrfloat++){
							unique_ptr<wstring> valstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);

							// check for no characters //
							if(valstr->size() == 0){
								// damn //
								DEBUG_BREAK;
								Logger::Get()->Error(L"GuiManager: ParseGuiFileResult: Invalid color required 3 or 4 params got: only "
									+Convert::IntToWstring(loopitrfloat));
								break;
							}
							Vals[loopitrfloat] = Convert::WstringToFloat(*valstr);
						}
						// create value //
						Float4 curcolor(Vals[0], Vals[1], Vals[2], Vals[3]);

						// set to right one //
						if(tval == 0)
							StartColor = curcolor;
						if(tval == 1)
							EndColor = curcolor;
						if(tval == 2)
							TextColor = curcolor;

						// don't forget to release memory //
						SAFE_DELETE_VECTOR(tokens);
					}
				}
			}

			// init with correct values //
			curlabel->Init(x, y, width, height, text, StartColor, EndColor, TextColor, TextSize, AutoAdjust != 0, Font, ListenOn);
			curlabel->Scripting = shared_ptr<ScriptObject>(data[i]->CreateScriptObjectAndReleaseThis(0, GOBJECT_TYPE_TEXTLABEL));
			// this function should have deleted everything related to that object (smart pointers and dtors take care of rest,
			// so it should be safe to just erase it //
			data.erase(data.begin()+i);
			i--;

			continue;
		}

		Logger::Get()->Error(L"GuiManager: ParseGuiFileResult: Unrecognized type !: "+data[i]->TName);
		// delete current //
		data.erase(data.begin()+i);
		i--;
	}
	// attach objects //
	for(unsigned int i = 0; i < TempOs.size(); i++){
		// get fake id //
		int FakeID = -1;

		for(unsigned int a = 0; a < FakeIDRealID.size(); a++){
			if(FakeIDRealID[a][1] == TempOs[i]->ID){
				FakeID = FakeIDRealID[a][0];
				break;
			}
		}

		// get collection to attach to //
		int CollID = -1;

		for(unsigned int a = 0; a < CollectionMember.size(); a++){
			if(CollectionMember[a][1] == FakeID){
				CollID = CollectionMember[a][0];
				break;
			}
		}
		// add to real objects //
		AddGuiObject(TempOs[i], CollID);
		// should be visible //
		if(TempOs[i]->HigherLevel == true){
			if(!GetCollection(CollID)->Visible){
				((RenderableGuiObject*)TempOs[i])->Hidden = TRUE;
			}
		}
	}

	return;
}
DLLEXPORT void Leviathan::GuiManager::WriteGuiToFile(const wstring &file){

}
// ------------------------------------ //
void Leviathan::GuiManager::UpdateArrays(){
	if(!ObjectAmountChanged)
		return;

	ObjectAmountChanged = false;
	NeedRendering.clear();
	vector<RenderableGuiObject*> temp;
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->HigherLevel){
			// higher than base object, minimum of render able //
			temp.push_back((RenderableGuiObject*)Objects[i]);
		}


	}

	int maxzval = 0;
	for(unsigned int i = 0; i < temp.size(); i++){
		if(temp[i]->Zorder > maxzval){
			maxzval = temp[i]->Zorder;
		}

	}

	NeedRendering.resize(temp.size());
	int curzval = 0;

	int index = NeedRendering.size()-1;
	// z ordering //
	while(curzval <= maxzval){
		for(unsigned int i = 0; i < temp.size(); i++){
			if(temp[i]->Zorder == curzval){
				NeedRendering[index] = temp[i];
				index--;
				if(index < 0)
					break;
			}

		}
		curzval++;
	}

}

// ----------------- event handler part --------------------- //
void GuiManager::AddListener(BaseEventable* receiver, EVENT_TYPE tolisten){
	Listeners.push_back(new GuiEventListener(receiver, tolisten));
}

void GuiManager::RemoveListener(Gui::BaseEventable* receiver, EVENT_TYPE type, bool all){
	for(unsigned int i = 0; i < Listeners.size(); i++){
		if(Listeners[i]->Listen == receiver){
			if((all) | (Listeners[i]->Tolisten == type)){
				delete Listeners[i];
				Listeners.erase(Listeners.begin()+i);
				i--;
			}
		}
	}
}

void GuiManager::CallEvent(Event* pEvent){
	// loop through listeners and call events //
	int returval = 0;
	for(unsigned int i = 0; i < Listeners.size(); i++){
		if(Listeners[i]->Tolisten == pEvent->GetType()){
			// call
			returval = Listeners[i]->Listen->OnEvent(&pEvent);
			if(returval == -1){ // asking for deletion
				RemoveListener(Listeners[i]->Listen, pEvent->GetType());
				i--;
				continue;
			}
			// check for deletion and end if event got deleted //
			if(!pEvent){
				return;
			}
		}
	}

	// delete object ourselves //
	SAFE_DELETE(pEvent);
}
// used to send hide events to individual objects //
int GuiManager::CallEventOnObject(BaseEventable* receive, Event* pEvent){
	// find right object
	int returval = 0;

	for(unsigned int i = 0; i < Listeners.size(); i++){
		if(Listeners[i]->Listen == receive){
			// call
			returval = Listeners[i]->Listen->OnEvent(&pEvent);


			if(returval == -1){ // asking for deletion but we don't care
				//RemoveListener(Listeners[i]->Listen, pEvent->GetType());
				//i--;
				continue;
			}
			// check for deletion and end if event got deleted //
			if(!pEvent){
				return returval;
			}
			break;
		}
	}

	// delete object ourselves //
	SAFE_DELETE(pEvent);
	return returval;
}
// ---------------- animation handler part ---------------------- //
int GuiManager::HandleAnimation(AnimationAction* perform, GuiAnimateable* caller, int mspassed){
	switch(perform->GetType()){
	case GUI_ANIMATION_MOVE:
		{
			GuiAnimationTypeMove* data = (GuiAnimationTypeMove*)perform->Data;

			int x = (int)(caller->GetValue(GUI_ANIMATEABLE_SEMANTIC_X));
			int y = (int)(caller->GetValue(GUI_ANIMATEABLE_SEMANTIC_Y));

			bool finished = false;

			int amount = (int)((((((float)mspassed))*data->Speed)+0.1f)*0.35f);
			if(amount < 1) // force the object to move, to avoid bugs causing objects to stop
				amount = 1;

			// switch here based on move mode
			switch(data->Priority){
			case GUI_ANIMATION_TYPEMOVE_PRIORITY_X:
				{
					// move x if right x then move y //
					if(x == data->X){
						if(y == data->Y){
							// finished
							finished = true;
							break;
						}

						// move y
						if(y < data->Y){
							y += amount;

							if(y > data->Y)
								y = data->Y;

						} else if (y > data->Y){
							y -= amount;

							if(y < data->Y)
								y = data->Y;
						}
						break;
					}

					if(x < data->X){
						x += amount;

						if(x > data->X)
							x = data->X;

					} else if (x > data->X){
						x -= amount;

						if(x < data->X)
							x = data->X;
					}
				}
			break;
			case GUI_ANIMATION_TYPEMOVE_PRIORITY_Y:
				{
					if(y == data->Y){
						if(x == data->X){
							// finished
							finished = true;
							break;
						}
						if(x < data->X){
							x += amount;

							if(x > data->X)
								x = data->X;

						} else if (x > data->X){
							x -= amount;

							if(x < data->X)
								x = data->X;
						}

						break;
					}
					// move y
					if(y < data->Y){
						y += amount;

						if(y > data->Y)
							y = data->Y;

					} else if (y > data->Y){
						y -= amount;

						if(y < data->Y)
							y = data->Y;
					}

				}
			break;
			case GUI_ANIMATION_TYPEMOVE_PRIORITY_BOTH:
				{
					// move y
					if(y < data->Y){
						y += amount/2;

						if(y > data->Y)
							y = data->Y;

					} else if (y > data->Y){
						y -= amount/2;

						if(y < data->Y)
							y = data->Y;
					}
					if(x < data->X){
						x += amount/2;

						if(x > data->X)
							x = data->X;

					} else if (x > data->X){
						x -= amount/2;

						if(x < data->X)
							x = data->X;
					}
					// check for finishing //
					if((x == data->X) && (y == data->Y)){
						finished = true;
						break;
					}
				}
			break;
			case GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPE:
				{
					// move x and calculate required y
					int xdist = data->X-x;
					int ydist = data->Y-y;
					FORCE_POSITIVE(ydist);
					FORCE_POSITIVE(xdist);
					// would divide by zero, and we don't want that //
					if(xdist == 0){
						// change type //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_Y;
						break;
					}
					if(ydist == 0){
						// change type //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_X;
						break;
					}
					if(xdist == ydist){
						// need a strategy change //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_BOTH;
						break;
					}
					// calculate slope for other coordinate //
					float slope = (float)ydist/xdist;

					if(x < data->X){
						x += amount;

						if(x > data->X)
							x = data->X;

					} else if (x > data->X){
						x -= amount;

						if(x < data->X)
							x = data->X;
					}

					if(y < data->Y){
						y += (int)(amount*slope);

						if(y > data->Y)
							y = data->Y;

					} else if (y > data->Y){
						y -= (int)(amount*slope);

						if(y < data->Y)
							y = data->Y;
					}


					// check for finishing //
					if((x == data->X) && (y == data->Y)){
						finished = true;
						break;
					}

				}
			break;
			case GUI_ANIMATION_TYPEMOVE_PRIORITY_SLOPEY:
				{
					// move x and calculate required y
					int xdist = data->X-x;
					int ydist = data->Y-y;
					FORCE_POSITIVE(ydist);
					FORCE_POSITIVE(xdist);
					// would divide by zero, and we don't want that //
					if(xdist == 0){
						// change type //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_Y;
						break;
					}
					if(ydist == 0){
						// change type //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_X;
						break;
					}
					if(xdist == ydist){
						// need a strategy change //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_BOTH;
						break;
					}
					// calculate slope for other coordinate //
					float slope = (float)xdist/ydist;

					if(x < data->X){
						x += (int)(amount*slope);

						if(x > data->X)
							x = data->X;

					} else if (x > data->X){
						x -= (int)(amount*slope);

						if(x < data->X)
							x = data->X;
					}

					if(y < data->Y){
						y += amount;

						if(y > data->Y)
							y = data->Y;

					} else if (y > data->Y){
						y -= amount;

						if(y < data->Y)
							y = data->Y;
					}


					// check for finishing //
					if((x == data->X) && (y == data->Y)){
						finished = true;
						break;
					}

				}
			break;
			case GUI_ANIMATION_TYPEMOVE_PRIORITY_SMOOTH_DIVIDE:
				{
					int xdist = data->X-x;
					int ydist = data->Y-y;
					FORCE_POSITIVE(ydist);
					FORCE_POSITIVE(xdist);

					// would divide by zero, and we don't want that //
					if(xdist == 0){
						// change type //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_Y;
						break;
					}
					if(ydist == 0){
						// change type //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_X;
						break;
					}
					if(xdist == ydist){
						// need a strategy change //
						data->Priority = GUI_ANIMATION_TYPEMOVE_PRIORITY_BOTH;
						break;
					}

					// share amount proportionally between x and y //
					int xamount = 0, yamount = 0;
					if(xdist < ydist){

						float amountprop = 1.0f-(float)xdist/ydist;

						yamount = (int)(amountprop*(mspassed*data->Speed+1.5f));
						if(yamount < 1)
							yamount = 1;
						xamount = (int)((mspassed*data->Speed+1.5f)*((float)xdist/ydist));
						if(xamount < 1)
							xamount = 1;
					}
					if(ydist < xdist){

						float amountprop = 1.0f-(float)ydist/xdist;

						xamount = (int)(amountprop*(mspassed*data->Speed+1.5f));
						if(xamount < 1)
							xamount = 1;
						yamount = (int)((mspassed*data->Speed+1.5f)*((float)ydist/xdist));
						if(yamount < 1)
							yamount = 1;
					}

					if(x < data->X){
						x += xamount;

						if(x > data->X)
							x = data->X;

					} else if (x > data->X){
						x -= xamount;

						if(x < data->X)
							x = data->X;
					}

					if(y < data->Y){
						y += yamount;

						if(y > data->Y)
							y = data->Y;

					} else if (y > data->Y){
						y -= yamount;

						if(y < data->Y)
							y = data->Y;
					}


					// check for finishing //
					if((x == data->X) && (y == data->Y)){
						finished = true;
						break;
					}

				}
			break;
			}

			// send updated values //
			caller->SetValue(GUI_ANIMATEABLE_SEMANTIC_X, (float)x);
			caller->SetValue(GUI_ANIMATEABLE_SEMANTIC_Y, (float)y);

			if(finished){
				return 1;

			} else {
				return 0;
			}
		}
	break;


	default:
		return 404; // unrecognized type //
	}



	//return 5; // for error //
}
// ----------------- collection managing --------------------- //
void GuiManager::CreateCollection(GuiCollection* add){
	Collections.push_back(add);
}
int GuiManager::GetCollection(wstring name, int id){
	for(unsigned int i = 0; i < Collections.size(); i++){
		// if name specified check for it //
		if(name != L""){
			if(Collections[i]->Name != name)
				continue; // no match

		}
		if(id != -1){
			if(Collections[i]->ID != id)
				continue; // no match

		}
		// match
		return i;

	}

	return -1;
}
GuiCollection* GuiManager::GetCollection(int id){
	for(unsigned int i = 0; i < Collections.size(); i++){
		if(id != -1){
			if(Collections[i]->ID != id)
				continue; // no match

		}
		// match
		return Collections[i];

	}

	return NULL;
}

bool GuiManager::GuiComboPress(const GKey &key){
	for(unsigned int i = 0; i < Collections.size(); i++){
		if(Collections[i]->Toggle.Match(key, false)){
			// is a match, toggle //
			if(Collections[i]->Visible){
				DisableCollection(Collections[i]->ID, false);
			} else {
				ActivateCollection(Collections[i]->ID, Collections[i]->Exclusive);
			}
			return true;
		}
	}
	return false;
}

void GuiManager::ActivateCollection(int id, bool exclusive){
	// if exclusive disable all others, but not self of course //
	if(exclusive){
		for(unsigned int i = 0; i < Collections.size(); i++){
			if(Collections[i]->ID != id){
				DisableCollection(Collections[i]->ID, false);
			}
		}
	}


	unsigned int index = GetCollection(L"", id);
	ARR_INDEX_CHECKINV(index, Collections.size()){
		// not found
		Logger::Get()->Error(L"Gui: 404, trying to show non existing collection", id,true);
		return;
	}
	// call script //
	if(Collections[index]->Scripting->Script->Instructions.size() > 1){
		vector<shared_ptr<ScriptNamedArguement>> parameters;
		parameters.push_back(shared_ptr<ScriptNamedArguement>(new ScriptNamedArguement(L"source", new IntBlock(EVENT_SOURCE_MANAGER), 
			DATABLOCK_TYPE_INT, false, true)));
		parameters.push_back(shared_ptr<ScriptNamedArguement>(new ScriptNamedArguement(L"Instance", new IntBlock(Collections[index]->ID), 
			DATABLOCK_TYPE_INT, false, true)));

		bool existed = false;
		ScriptInterface::Get()->ExecuteIfExistsScript(Collections[index]->Scripting.get(), L"OnEnable", parameters, existed, false);
	}

	Collections[index]->Visible = true;
	// unhide objects, call events and check return values //
	for(unsigned int i = 0; i < Collections[index]->children.size(); i++){

		// check is it eventable //
		if(Collections[index]->children[i]->ObjectLevel >= GUI_OBJECT_LEVEL_EVENTABLE){
			// event can be sent //
			BaseEventable* temp = (BaseEventable*)Collections[index]->children[i];
			int returnval = CallEventOnObject(temp, new Event(EVENT_TYPE_SHOW, new Int2(EVENT_SOURCE_MANAGER, false /* non fast */)));
			if(returnval == 1){
				// asks to be allowed to show itself
				continue;
			}
			temp->Hidden = false;

		} else if (Collections[index]->children[i]->ObjectLevel >= GUI_OBJECT_LEVEL_RENDERABLE){
			// set as visible //
			((RenderableGuiObject*)Collections[index]->children[i])->Hidden = false;

		}
	}
	

}
void GuiManager::DisableCollection(int id, bool fast){
	
	unsigned int index = GetCollection(L"", id);
	ARR_INDEX_CHECKINV(index, Collections.size()){
		// not found
		Logger::Get()->Error(L"Gui: 404, trying to hide non existing collection", id,true);
		return;
	}
	// call script //
	if(Collections[index]->Scripting->Script->Instructions.size() > 1){
		vector<shared_ptr<ScriptNamedArguement>> parameters;
		parameters.push_back(shared_ptr<ScriptNamedArguement>(new ScriptNamedArguement(L"source", new IntBlock(EVENT_SOURCE_MANAGER), 
			DATABLOCK_TYPE_INT, false, true)));
		parameters.push_back(shared_ptr<ScriptNamedArguement>(new ScriptNamedArguement(L"Instance", new IntBlock(Collections[index]->ID), 
			DATABLOCK_TYPE_INT, false, true)));

		bool existed = false;
		ScriptInterface::Get()->ExecuteIfExistsScript(Collections[index]->Scripting.get(), L"OnDisable", parameters, existed, false);
	}

	Collections[index]->Visible = false;
	// call events and hide if doesn't want to hide self //
	for(unsigned int i = 0; i < Collections[index]->children.size(); i++){

		// check is it eventable //
		if(Collections[index]->children[i]->ObjectLevel >= GUI_OBJECT_LEVEL_EVENTABLE){
			// event can be sent //
			BaseEventable* temp = (BaseEventable*)Collections[index]->children[i];
			int returnval = CallEventOnObject(temp, new Event(EVENT_TYPE_HIDE, new Int2(EVENT_SOURCE_MANAGER, fast /* fast/non fast */)));
			if(returnval == 1){
				// asks to be allowed to hide itself
				continue;
			}
			temp->Hidden = true;

		} else if (Collections[index]->children[i]->ObjectLevel >= GUI_OBJECT_LEVEL_RENDERABLE){
			// set as hidden //
			((RenderableGuiObject*)Collections[index]->children[i])->Hidden = true;

		}


	}

}

// -------------------------------------- //
bool GuiManager::HasForeGround(){
	return (Foreground != NULL);
}

// ----------------- GuiCollection --------------------- //
Leviathan::GuiCollection::GuiCollection(const wstring &name, int id, bool visible, const wstring &toggle, bool strict /*= false*/, 
	bool exclusive /*= false*/, bool enabled /*= true*/)
{
	Name = name;
	ID = id;
	Visible = visible;
	Enabled = enabled;
	Exclusive = exclusive;

	wchar_t chara = L'\n';
	KEYSPECIAL additional = KEYSPECIAL_NONE;
	bool Shift = false;
	bool Alt = false;
	bool Ctrl = false;

	if(Misc::CountOccuranceWstring(toggle, L"+")){
		vector<wstring> words;
		Misc::CutWstring(toggle, L"+",words);



		for(unsigned int i = 0; i < words.size(); i++){
			if(words[i].size() == 0)
				continue;
			if(i+1 >= words.size()){
				// last must be character
				chara = words[i][0];
			}
			if(Misc::WstringCompareInsensitive(words[i], L"alt")){
				Alt = true;
				continue;
			}
			if(Misc::WstringCompareInsensitive(words[i], L"shift")){
				Shift = true;
				continue;
			}
			if(Misc::WstringCompareInsensitive(words[i], L"ctrl")){
				Ctrl = true;
				continue;
			}
		}

	} else {
		if(toggle.size() != 0)
			chara = toggle[0];
	}
	additional = Leviathan::GKey::ConstructSpecial(Shift, Alt, Ctrl);
	Toggle = GKey((int)chara, additional);

	Strict = strict;
}

Leviathan::GuiCollection::GuiCollection(){
	Name = Misc::GetErrString();
	ID = -1;
	Visible = false;
	Enabled = false;
}

Leviathan::GuiCollection::~GuiCollection(){
	// release script //

	// possibly release children here //
}
