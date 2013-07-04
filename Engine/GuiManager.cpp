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

DLLEXPORT void Leviathan::GuiManager::AddKeyPress(int keyval, InputEvent* originalevent){
	ReceivedPresses.push_back(shared_ptr<GuiReceivedKeyPress>(new GuiReceivedKeyPress(GUI_KEYSTATE_TYPE_KEYPRESS, keyval, originalevent)));
}

DLLEXPORT void Leviathan::GuiManager::AddKeyDown(int keyval, InputEvent* originalevent){
	ReceivedPresses.push_back(shared_ptr<GuiReceivedKeyPress>(new GuiReceivedKeyPress(GUI_KEYSTATE_TYPE_KEYDOWN, keyval, originalevent)));
}

DLLEXPORT bool Leviathan::GuiManager::IsEventConsumed(InputEvent** ev){
	// key presses must be processed before this can be used //
	if(!GKeyPressesConsumed){
		// presses haven't been processed yet //

		ProcessKeyPresses();

		// now processed //
		GKeyPressesConsumed = true;
	}

	// check is it still in key presses (if it is it hasn't been consumed) //
	for(size_t i = 0; i < ReceivedPresses.size(); i++){

		if(ReceivedPresses[i]->MatchingEvent == *ev){
			// wasn't consumed, consume now //
			ReceivedPresses.erase(ReceivedPresses.begin()+i);
			return false;
		}
	}
	// not found, has been consumed //
	return true;
}

DLLEXPORT void Leviathan::GuiManager::SetKeyContainedValuesAsConsumed(const GKey &k){
	int chara = k.GetCharacter();
	bool Shift, Ctrl, Alt;
	Shift = Ctrl = Alt = false;

	bool characterfound = false;

	GKey::DeConstructSpecial(k.GetAdditional(), Shift, Alt, Ctrl);

	for(size_t i = 0; i < ReceivedPresses.size(); i++){
		// check does this match anything that the key has //
		if(ReceivedPresses[i]->KeyCode == chara){

			characterfound = true;
			goto erasepressedkeyonindex;
		}
		if(Shift && ReceivedPresses[i]->KeyCode == VK_SHIFT){

			Shift = false;
			goto erasepressedkeyonindex;
		}
		if(Ctrl && ReceivedPresses[i]->KeyCode == VK_CONTROL){

			Ctrl = false;
			goto erasepressedkeyonindex;
		}
		if(Alt && ReceivedPresses[i]->KeyCode == VK_MENU){

			Alt = false;
			goto erasepressedkeyonindex;
		}

		continue;
erasepressedkeyonindex:
		ReceivedPresses.erase(ReceivedPresses.begin()+i);
		i--;

		// check ending //
		if(!Shift && !Ctrl && !Alt && characterfound){
			// we have already found everything we need, we can quit //
			break;
		}
	}
}

DLLEXPORT void Leviathan::GuiManager::ProcessKeyPresses(){
	// special key states //
	bool Shift = false;
	bool Ctrl = false;
	bool Alt = false;

	// check special keys //
	for(size_t i = 0; i < ReceivedPresses.size(); i++){
		// type doesn't matter here //
		if(ReceivedPresses[i]->KeyCode == VK_SHIFT){
			Shift = true;
		} else if(ReceivedPresses[i]->KeyCode == VK_CONTROL){
			Ctrl = true;
		} else if(ReceivedPresses[i]->KeyCode == VK_MENU){
			Alt = true;
		} else {
			// don't want to do check if nothing was updated //
			continue;
		}
		// check for ending before all are handled //
		if(Shift && Ctrl && Alt)
			break;
	}
	//CallEvent(new Event(EVENT_TYPE_GUIENABLE, NULL));

	const KEYSPECIAL specialstates = Leviathan::GKey::ConstructSpecial(Shift, Alt, Ctrl);

	//Engine::GetEngine()->SetGuiActive(true);
	// pop action //
	//ReceivedPresses.erase(ReceivedPresses.begin()+i);
	
	if(DataStore::Get()->GetGUiActive()){
		// send presses to objects //

		for(unsigned int i = 0; i < ReceivedPresses.size(); i++){
			// generate key //
			GKey current = GKey(ReceivedPresses[i]->KeyCode, specialstates);

			// first we need to check the foreground object if it wants this key press //
			if(Foreground){

				if(Foreground->ObjectLevel >= GUI_OBJECT_LEVEL_EVENTABLE){
					// can send //
					BaseEventable* temp = (BaseEventable*)Foreground;
					int returnval = 0;

					if(ReceivedPresses[i]->Type == GUI_KEYSTATE_TYPE_KEYPRESS){
						returnval = CallEventOnObject(temp, new Event(EVENT_TYPE_KEYPRESS, (void*)&current, false));
					} else {
						returnval = CallEventOnObject(temp, new Event(EVENT_TYPE_KEYDOWN, (void*)&current, false));
					}
					

					if(returnval > 0){
						// processed it //
						// TODO: make sure that if the object didn't want Shift CTRL or ALT it unmarked them from the key //
						SetKeyContainedValuesAsConsumed(current);
						continue;
					}
					// just fall through to continue sending the key //
				}
			}

			// only allow collections to act to key presses //
			if(ReceivedPresses[i]->Type == GUI_KEYSTATE_TYPE_KEYPRESS){
				// check does a collection want to do something //
				if(GuiComboPress(current)){
					// press is now consumed //
					SetKeyContainedValuesAsConsumed(current);
					continue;
				}
			}

			// nobody wanted this, fire an event and delete if it got processed , false is important for not deleting stack object //
			bool processed = false;

			if(ReceivedPresses[i]->Type == GUI_KEYSTATE_TYPE_KEYPRESS){
				processed = CallEvent(new Event(EVENT_TYPE_KEYPRESS, (void*)&current, false));
			} else {
				processed = CallEvent(new Event(EVENT_TYPE_KEYDOWN, (void*)&current, false));
			}

			if(processed){
				// TODO: make sure that if the object didn't want Shift CTRL or ALT it unmarked them from the key //
				SetKeyContainedValuesAsConsumed(current);
			}
			// nobody wanted it //
		}

	} else {
		// check should Gui turn on //
		for(unsigned int i = 0; i < ReceivedPresses.size(); i++){
			if(ReceivedPresses[i]->Type != GUI_KEYSTATE_TYPE_KEYPRESS){
				// needs to be skipped //
				continue;
			}

			// generate key //
			GKey current = GKey(ReceivedPresses[i]->KeyCode, specialstates);

			// check does a collection want to do something //
			if(GuiComboPress(current)){
				// press is now consumed //
				SetKeyContainedValuesAsConsumed(current);
				continue;
			}
			// nothing //
		}
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
	// header flag definitions //
	vector<shared_ptr<NamedVariableList>> headerdata;

	// parse file to structure //
	vector<shared_ptr<ObjectFileObject>> data = ObjectFileProcessor::ProcessObjectFile(file, headerdata);


	// temporary object data stores //
	vector<Int2> FakeIDRealID;
	vector<BaseGuiObject*> TempOs;
	vector<Int2> CollectionMember;
	// reserve space //
	TempOs.reserve(data.size());
	FakeIDRealID.reserve(data.size());
	CollectionMember.reserve(data.size());


	for(size_t i = 0; i < data.size(); i++){
		// check what type the object is //
		if(data[i]->TName == L"Collection"){

			if(!LoadCollection(CollectionMember, data, *data[i])){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load collection, named "+data[i]->Name);
				continue;
			}
			// this function should have deleted everything related to that object, so it should be safe to just erase it //
			data.erase(data.begin()+i);
			i--;

			continue;
		}
		if(data[i]->TName == L"TextLabel"){

			// try to load //
			if(!TextLabel::LoadFromFileStructure(TempOs, FakeIDRealID, *data[i])){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load TextLabel, named "+data[i]->Name);
				continue;
			}

			// this function should have deleted everything related to that object (smart pointers and dtors take care of rest,
			// so it should be safe to just erase it //
			data.erase(data.begin()+i);
			i--;

			continue;
		}

		Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: Unrecognized type! typename: "+data[i]->TName);
		// delete current //
		data.erase(data.begin()+i);
		i--;
	}
	// attach objects //
	for(size_t i = 0; i < TempOs.size(); i++){
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

}
DLLEXPORT bool Leviathan::GuiManager::LoadCollection(vector<Int2> &membermapping, vector<shared_ptr<ObjectFileObject>> &data, ObjectFileObject &collectiondata){
	// load a GuiCollection from the structure //
	int createdid = IDFactory::GetID();

	wstring Toggle = L"";
	bool Enabled = true;
	bool Visible = true;
	bool Strict = false;

	for(size_t a = 0; a < collectiondata.Contents.size(); a++){
		if(collectiondata.Contents[a]->Name == L"members"){
			for(size_t tind = 0; tind < collectiondata.Contents[a]->Lines.size(); tind++){
				
				WstringIterator itr(collectiondata.Contents[a]->Lines[tind], false);
				
				unique_ptr<wstring> itrresult = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE);

				// check first word //
				if(*itrresult == L"ID"){

					// get next number, which should be id of member object //
					itrresult = itr.GetNextNumber(DECIMALSEPARATORTYPE_NONE);

					membermapping.push_back(Int2(createdid, Convert::WstringTo<int>(*itrresult)));

				} else {

					// unknown member type //
					Logger::Get()->Error(L"GuiManager: LoadCollection: invalid member definition: "+*itrresult);
				}
			}
			continue;
		}
		if(collectiondata.Contents[a]->Name == L"params"){
			// get values //



			ObjectFileProcessor::LoadValueFromNamedVars<wstring>(collectiondata.Contents[a]->Variables, L"ToggleOn", Toggle, L"", true,
				L"GuiManager: LoadCollection:");

			ObjectFileProcessor::LoadValueFromNamedVars<bool>(collectiondata.Contents[a]->Variables, L"Enabled", Enabled, false, true,
				L"GuiManager: LoadCollection:");

			ObjectFileProcessor::LoadValueFromNamedVars<bool>(collectiondata.Contents[a]->Variables, L"Visible", Visible, false, true,
				L"GuiManager: LoadCollection:");

			ObjectFileProcessor::LoadValueFromNamedVars<bool>(collectiondata.Contents[a]->Variables, L"Strict", Strict, false, true,
				L"GuiManager: LoadCollection:");

			continue;
		}
	}
	// allocate new GUI object //
	GuiCollection* cobj = new GuiCollection(collectiondata.Name, createdid, Visible, Toggle, Strict, Enabled);
	// copy script data over //
	cobj->Scripting = shared_ptr<ScriptObject>(collectiondata.CreateScriptObjectAndReleaseThis(0, GOBJECT_TYPE_COLLECTION));
	// add to collection list //
	CreateCollection(cobj);

	// loading succeeded //
	return true;
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

bool Leviathan::GuiManager::CallEvent(Event* pEvent){
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
				return true;
			}
		}
	}

	// delete object ourselves //
	SAFE_DELETE(pEvent);
	// nobody wanted the event //
	return false;
}
// used to send hide events to individual objects //
int GuiManager::CallEventOnObject(BaseEventable* receive, Event* pEvent){
	// find right object
	int returval = -3;

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
int Leviathan::GuiManager::GetCollection(const wstring &name, int id){
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
	for(size_t i = 0; i < Collections.size(); i++){
		// compare IDs of collections //
		if(Collections[i]->ID != id)
			continue; // no match
		// match
		return Collections[i];
	}

	return NULL;
}

bool Leviathan::GuiManager::GuiComboPress(GKey &key){
	for(unsigned int i = 0; i < Collections.size(); i++){
		if(Collections[i]->Toggle.Match(key, false)){
			// disable unwanted keys if possible //
			key.SetAdditional(Collections[i]->Toggle.GetAdditional());

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
		vector<shared_ptr<NamedVariableBlock>> parameters;
		parameters.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock(EVENT_SOURCE_MANAGER), L"source")));
		parameters.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock(Collections[index]->ID), L"Instance")));

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
		vector<shared_ptr<NamedVariableBlock>> parameters;
		parameters.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock(EVENT_SOURCE_MANAGER), L"source")));
		parameters.push_back(shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock(Collections[index]->ID), L"Instance")));

		bool existed = false;
		ScriptInterface::Get()->ExecuteIfExistsScript(Collections[index]->Scripting.get(), L"OnEnable", parameters, existed, false);
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
