#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_MAIN
#include "GuiManager.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include "Engine.h"
#include "Script\ScriptInterface.h"
#include "FileSystem.h"
#include <boost\assign\list_of.hpp>

Leviathan::Gui::GuiManager::GuiManager() : ReceivedPresses(), ID(IDFactory::GetID()), Foreground(NULL), MainInput(NULL){
	ObjectAmountChanged = false;

	staticaccess = this;
	GKeyPressesConsumed = true;
}
Leviathan::Gui::GuiManager::~GuiManager(){
	// on quit calls //
	SAFE_DELETE(MainInput);
	Release();

	staticaccess = NULL;
}

GuiManager* Leviathan::Gui::GuiManager::staticaccess = NULL;
GuiManager* Leviathan::Gui::GuiManager::Get(){
	return staticaccess; 
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::Init(AppDef* vars, Graphics* graph){

	ThisRenderer = graph;

	// create press listeners //
	MainInput = new KeyListener(this, Engine::GetEngine()->GetKeyPresManager());
	
	// load main Gui file //
	ExecuteGuiScript(FileSystem::GetScriptsFolder()+L"MainGui.txt");


	return true;
}

void Leviathan::Gui::GuiManager::Release(){
	for(unsigned int i = 0; i < Objects.size(); i++){
		// object's release function will do everything needed (even deleted if last reference) //
		SAFE_RELEASE(Objects[i]);

		Objects.erase(Objects.begin()+i);
		i--;
	}

	SAFE_DELETE_VECTOR(Collections);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::ClearKeyReceivingState(){
	// reset state //
	GKeyPressesConsumed = false;
	// ensure that the vector is empty //
	ReceivedPresses.clear();
}

DLLEXPORT void Leviathan::Gui::GuiManager::AddKeyPress(int keyval, InputEvent* originalevent){
	ReceivedPresses.push_back(shared_ptr<GuiReceivedKeyPress>(new GuiReceivedKeyPress(GUI_KEYSTATE_TYPE_KEYPRESS, keyval, originalevent)));
}

DLLEXPORT void Leviathan::Gui::GuiManager::AddKeyDown(int keyval, InputEvent* originalevent){
	ReceivedPresses.push_back(shared_ptr<GuiReceivedKeyPress>(new GuiReceivedKeyPress(GUI_KEYSTATE_TYPE_KEYDOWN, keyval, originalevent)));
}

DLLEXPORT bool Leviathan::Gui::GuiManager::IsEventConsumed(InputEvent** ev){
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

DLLEXPORT void Leviathan::Gui::GuiManager::SetKeyContainedValuesAsConsumed(const GKey &k){
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

DLLEXPORT void Leviathan::Gui::GuiManager::ProcessKeyPresses(){
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

	const KEYSPECIAL specialstates = GKey::ConstructSpecial(Shift, Alt, Ctrl);
	
	if(DataStore::Get()->GetGUiActive()){
		// send presses to objects //
		bool lol = false;

		for(unsigned int i = 0; i < ReceivedPresses.size(); i++){
			// generate key //
			GKey current = GKey(ReceivedPresses[i]->KeyCode, specialstates);
						
			// first we need to check the foreground object if it wants this key press //
			if(Foreground){

				if(Foreground->ObjectFlags & GUIOBJECTHAS_EVENTABLE){
					// can send //
					BaseGuiEventable* temp = dynamic_cast<BaseGuiEventable*>(Foreground);
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

				if(!lol){
					lol = true;
					DEBUG_OUTPUT_AUTOPLAINTEXT(L"--------------");
				}

				DEBUG_OUTPUT_AUTO(current.GenerateWstringMessage());


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

void Leviathan::Gui::GuiManager::GuiTick(int mspassed){
	// send tick event //

}
void Leviathan::Gui::GuiManager::AnimationTick(int mspassed){
	// animations //
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->ObjectFlags & GUIOBJECTHAS_ANIMATEABLE){

			dynamic_cast<GuiAnimateable*>(Objects[i])->AnimationTime(mspassed);
		}
	}
}
void Leviathan::Gui::GuiManager::Render(){
	UpdateArrays();

	for(unsigned int i = 0; i < NeedRendering.size(); i++){
		NeedRendering[i]->Render(ThisRenderer);
	}
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::OnResize(){
	// call events //
	this->CallEvent(new Event(EVENT_TYPE_WINDOW_RESIZE, (void*)new Int2(DataStore::Get()->GetWidth(), DataStore::Get()->GetHeight())));
}

// ------------------------------------ //
bool Leviathan::Gui::GuiManager::AddGuiObject(BaseGuiObject* obj){
	ObjectAmountChanged = true;

	Objects.push_back(obj);
	return true;
}

bool Leviathan::Gui::GuiManager::AddGuiObject(BaseGuiObject* obj, int collectionid){
	// new objects will be added //
	ObjectAmountChanged = true;

	Objects.push_back(obj);

	// add to collection //
	GuiCollection* collection = GetCollection(collectionid);
	if(collection){
		// add
		collection->children.push_back(obj);
	}
	return true;
}

void Leviathan::Gui::GuiManager::DeleteObject(int id){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id){
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

			SAFE_RELEASE(Objects[i]);
			Objects.erase(Objects.begin()+i);
			return;
		}
	}
}

int Leviathan::Gui::GuiManager::GetObjectIndexFromId(int id){
	for(unsigned int i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id)
			return i;
	}
	return -1;
}

BaseGuiObject* Leviathan::Gui::GuiManager::GetObject(unsigned int index){
	ARR_INDEX_CHECK(index, Objects.size()){
		return Objects[index];
	}
	return NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::ExecuteGuiScript(const wstring &file){
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
			// delete rest of the object //
			goto guiprocessguifileloopdeleteprocessedobject;
		}
		if(data[i]->TName == L"TextLabel"){

			// try to load //
			if(!TextLabel::LoadFromFileStructure(this, TempOs, FakeIDRealID, *data[i])){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load TextLabel, named "+data[i]->Name);
				continue;
			}
			// delete rest of the object //
			goto guiprocessguifileloopdeleteprocessedobject;
		}

		Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: Unrecognized type! typename: "+data[i]->TName);

guiprocessguifileloopdeleteprocessedobject:

		// delete current //
		data.erase(data.begin()+i);
		i--;
	}
	// attach objects //
	for(size_t i = 0; i < TempOs.size(); i++){
		// get fake id //
		int FakeID = -1;

		for(unsigned int a = 0; a < FakeIDRealID.size(); a++){
			if(FakeIDRealID[a][1] == TempOs[i]->GetID()){
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
		// should be hidden if collection is hidden //
		if(!GetCollection(CollID)->Visible){
			if(TempOs[i]->ObjectFlags & GUIOBJECTHAS_RENDERABLE){

				dynamic_cast<RenderableGuiObject*>(TempOs[i])->SetHiddenState(true);
			}
		}
	}

}
DLLEXPORT bool Leviathan::Gui::GuiManager::LoadCollection(vector<Int2> &membermapping, vector<shared_ptr<ObjectFileObject>> &data, ObjectFileObject &collectiondata){
	// load a GuiCollection from the structure //
	int createdid = IDFactory::GetID();

	wstring Toggle = L"";
	bool Enabled = true;
	bool Visible = true;
	bool Strict = false;

	for(size_t a = 0; a < collectiondata.Contents.size(); a++){

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
	// text block processing //
	for(size_t a = 0; a < collectiondata.TextBlocks.size(); a++){
		if(collectiondata.TextBlocks[a]->Name == L"members"){
			for(size_t tind = 0; tind < collectiondata.TextBlocks[a]->Lines.size(); tind++){

				WstringIterator itr(collectiondata.TextBlocks[a]->Lines[tind], false);

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
	}

	// allocate new Collection object //
	GuiCollection* cobj = new GuiCollection(collectiondata.Name, createdid, Visible, Toggle, Strict, Enabled);
	// copy script data over //
	cobj->Scripting = collectiondata.Script;
	// add to collection list //
	CreateCollection(cobj);

	// loading succeeded //
	return true;
}

DLLEXPORT void Leviathan::Gui::GuiManager::WriteGuiToFile(const wstring &file){

}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::UpdateArrays(){
	if(!ObjectAmountChanged)
		return;

	ObjectAmountChanged = false;
	NeedRendering.clear();
	for(unsigned int i = 0; i < Objects.size(); i++){
		// check does it contain renderable data //
		if(Objects[i]->ObjectFlags & GUIOBJECTHAS_RENDERABLE){
			// copy to renderables vector to reduce dynamic casts per frame //
			NeedRendering.push_back(dynamic_cast<RenderableGuiObject*>(Objects[i]));
		}
	}
}

// ----------------- event handler part --------------------- //
void GuiManager::AddListener(BaseGuiEventable* receiver, EVENT_TYPE tolisten){
	Listeners.push_back(new GuiEventListener(receiver, tolisten));
}

void GuiManager::RemoveListener(Gui::BaseGuiEventable* receiver, EVENT_TYPE type, bool all){
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

bool Leviathan::Gui::GuiManager::CallEvent(Event* pEvent){
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
int GuiManager::CallEventOnObject(BaseGuiEventable* receive, Event* pEvent){
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
// ----------------- collection managing --------------------- //
void GuiManager::CreateCollection(GuiCollection* add){
	Collections.push_back(add);
}
GuiCollection* Leviathan::Gui::GuiManager::GetCollection(const int &id, const wstring &name){
	// look for collection based on id or name //
	for(size_t i = 0; i < Collections.size(); i++){
		if(id >= 0){
			if(Collections[i]->ID != id){
				// no match //
				continue;
			}
		} else {
			// name should be specified, check for it //
			if(Collections[i]->Name != name){
				continue; 
			}
		}

		// match
		return Collections[i];
	}

	return NULL;
}

bool Leviathan::Gui::GuiManager::GuiComboPress(GKey &key){
	for(unsigned int i = 0; i < Collections.size(); i++){
		if(Collections[i]->Toggle.Match(key, false)){
			// disable unwanted keys if possible //
			key.SetAdditional(Collections[i]->Toggle.GetAdditional());

			// is a match, toggle //
			SetCollectionState(Collections[i]->ID, !Collections[i]->Visible, Collections[i]->Exclusive);

			return true;
		}
	}
	return false;
}

void GuiManager::SetCollectionState(const int &id, const bool &visible, const bool &exclusive){
	// if exclusive and becoming visible disable all others, but not self of course //
	if(exclusive && visible){
		for(size_t i = 0; i < Collections.size(); i++){
			if(Collections[i]->ID != id){
				SetCollectionState(Collections[i]->ID, false, false);
			}
		}
	}
	
	GuiCollection* collection = GetCollection(id);
	if(!collection){

		Logger::Get()->Error(L"GuiManager: SetCollectionState: invalid collection");
		return;
	}
	// call script //
	ScriptScript* tmpscript = collection->Scripting.get();

	if(tmpscript){
		// check does the script contain right listeners //
		ScriptModule* mod = tmpscript->GetModule();

		const wstring &listenername = visible ? L"OnEnable": L"OnDisable";

		if(mod->DoesListenersContainSpecificListener(listenername)){
			// call it //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new IntBlock(EVENT_SOURCE_MANAGER), L"source"))
				(new NamedVariableBlock(new IntBlock(id), L"Instance"));

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(Convert::WstringToString(listenername)).SetArguements(Args);

			ScriptInterface::Get()->ExecuteScript(tmpscript, &sargs);
		}
	}

	collection->Visible = visible;
	// set all child object's visibilities, also //
	for(size_t i = 0; i < collection->children.size(); i++){

		// check is it eventable //
		if(collection->children[i]->ObjectFlags & GUIOBJECTHAS_EVENTABLE){
			// event can be sent //
			BaseGuiEventable* temp = dynamic_cast<BaseGuiEventable*>(collection->children[i]);
			int returnval = CallEventOnObject(temp, new Event(visible ? EVENT_TYPE_SHOW: EVENT_TYPE_HIDE, new Int2(EVENT_SOURCE_MANAGER, false /* non fast */)));
			if(returnval == 1){
				// asks to be allowed to show itself
				continue;
			}

		}
		// object is letting Collection to set it's visibility state //
		if(collection->children[i]->ObjectFlags & GUIOBJECTHAS_RENDERABLE){
			// set as visible //
			dynamic_cast<RenderableGuiObject*>(collection->children[i])->SetHiddenState(!visible);
		}
	}
	

}
// -------------------------------------- //
bool GuiManager::HasForeGround(){
	return (Foreground != NULL);
}
// ----------------- GuiCollection --------------------- //
Leviathan::Gui::GuiCollection::GuiCollection(const wstring &name, int id, bool visible, const wstring &toggle, bool strict /*= false*/, 
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
	additional = GKey::ConstructSpecial(Shift, Alt, Ctrl);
	Toggle = GKey((int)chara, additional);

	Strict = strict;
}
Leviathan::Gui::GuiCollection::~GuiCollection(){
	// release script //

	// possibly release children here //

}
