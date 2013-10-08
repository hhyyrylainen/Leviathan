#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASEOBJECT
#include "BaseGuiObject.h"
#endif
#include "Utility\Iterators\WstringIterator.h"
#include "ObjectFiles\ObjectFileProcessor.h"
#include "Script\ScriptModule.h"
#include <boost\assign\list_of.hpp>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::BaseGuiObject::BaseGuiObject(GuiManager* owner, const wstring &name, int fakeid, shared_ptr<ScriptScript> 
	script /*= NULL*/) : OwningInstance(owner), FileID(fakeid), Name(name), Scripting(script), Element(NULL)
{
	ID = IDFactory::GetID();
}

DLLEXPORT Leviathan::Gui::BaseGuiObject::~BaseGuiObject(){
	// script has smart pointer //

	// this should always be safe //
	_UnhookAllRocketListeners();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::BaseGuiObject::LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects, 
	ObjectFileObject& dataforthis, GuiLoadedSheet* sheet)
{
	// parse fake id from prefixes //
	int fakeid = 0;

	for(size_t i = 0; i < dataforthis.Prefixes.size(); i++){

		if(Misc::WstringStartsWith(*dataforthis.Prefixes[i], L"ID")){
			// get id number //
			WstringIterator itr(dataforthis.Prefixes[i].get(), false);

			auto tempnumber = itr.GetNextNumber(DECIMALSEPARATORTYPE_NONE);

			fakeid = Convert::WstringToInt(*tempnumber);
			// nothing more to find //
			break;
		}
	}

	unique_ptr<BaseGuiObject> tmpptr(new BaseGuiObject(owner, dataforthis.Name, fakeid, dataforthis.Script));

	Rocket::Core::Element* element = NULL;

	shared_ptr<NamedVariableList> listenon;

	// setup listeners //
	for(size_t i = 0; i < dataforthis.Contents.size(); i++){
		if(Misc::WstringCompareInsensitiveRefs(dataforthis.Contents[i]->Name, L"params")){

			wstring rocketobjectname;

			if(ObjectFileProcessor::LoadValueFromNamedVars<wstring>(dataforthis.Contents[i]->Variables, L"RocketID", rocketobjectname, L"", true,
				L"BaseGuiObject: LoadFromFileStructure: "))
			{
				element = sheet->GetElementByID(Convert::WstringToString(rocketobjectname));
			}

			listenon = dataforthis.Contents[i]->Variables.GetValueDirect(L"ListenOn");
		}
	}
	if(element){
		tmpptr->Element = element;
	} else {
		Logger::Get()->Warning(L"BaseGuiObject: LoadFromFileStructure: probably missing 'l params{' block in file, name: "+tmpptr->Name);
	}
	// listening start //
	if(listenon.get()){
		tmpptr->StartMonitoring(listenon->GetValues());
	}


	if(tmpptr.get()){
		tmpptr->_HookRocketListeners();
		tempobjects.push_back(tmpptr.release());
		return true;
	}

	return false;
}
// ------------------------------------ //
void Leviathan::Gui::BaseGuiObject::_HookRocketListeners(){
	// if we don't have object attached return an error //
	if(!Element){

		Logger::Get()->Error(L"BaseGuiObject: _HookRocketListeners: no element found with id");
		return;
	}

	// first we need to get probably right listeners and register with them //
	ScriptModule* mod = Scripting->GetModule();

	std::vector<wstring> containedlisteners;

	mod->GetListOfListeners(containedlisteners);

	for(size_t i = 0; i < containedlisteners.size(); i++){

		Rocket::Core::String tohook = LeviathanToRocketEventTranslate[containedlisteners[i]];

		if(tohook.Length() > 0){
			Element->AddEventListener(tohook, this);
			// store to be able to detach later //
			HookedRocketEvents.push_back(tohook);
		}
	}
}
// ------------------------------------ //
DLLEXPORT int Leviathan::Gui::BaseGuiObject::OnEvent(Event** pEvent){

	// call script to handle the event //

	switch((*pEvent)->GetType()){

	case EVENT_TYPE_LISTENERVALUEUPDATED:
		{
			_CallScriptListener(GUIOBJECT_LISTENERTYPE_LISTENERVALUE, pEvent);
		}
	break;

	}

	return 0;
}

DLLEXPORT bool Leviathan::Gui::BaseGuiObject::OnUpdate(const shared_ptr<NamedVariableList> &updated){
	ValuesUpdated = true;

	// push to update vector //
	UpdatedValues.push_back(updated);

	// fire an event //
	Event* tmpevent = new Event(EVENT_TYPE_LISTENERVALUEUPDATED, NULL, false);
	
	OnEvent(&tmpevent);

	tmpevent->Release();
	return true;
}



void Leviathan::Gui::BaseGuiObject::OnDetach(Rocket::Core::Element* element){
	//Element = NULL;
	//// don't want to accidentally do unhooking //
	//HookedRocketEvents.clear();
}

void Leviathan::Gui::BaseGuiObject::ProcessEvent(Rocket::Core::Event& event){
	// call script to handle the event //
	DEBUG_BREAK;
}

void Leviathan::Gui::BaseGuiObject::_UnhookAllRocketListeners(){
	if(HookedRocketEvents.empty() || Element == NULL)
		return;
	for(std::list<Rocket::Core::String>::iterator iter = HookedRocketEvents.begin(); iter != HookedRocketEvents.end(); iter++){

		Rocket::Core::String unregister = *iter;

		Element->RemoveEventListener(unregister, this);
	}
	HookedRocketEvents.clear();
}

void Leviathan::Gui::BaseGuiObject::_CallScriptListener(GUIOBJECT_LISTENERTYPE type, Event** pEvent){

	switch(type){
	case GUIOBJECT_LISTENERTYPE_LISTENERVALUE:
		{
			// check does the script contain right listeners //
			ScriptModule* mod = Scripting->GetModule();

			if(mod->DoesListenersContainSpecificListener(LISTENERNAME_ONLISTENUPDATE)){
				// setup parameters //
				vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"BaseGuiObject"))
					(new NamedVariableBlock(new VoidPtrBlock(*pEvent), L"Event"));
				// we are returning ourselves so increase refcount
				AddRef();
				(*pEvent)->AddRef();

				ScriptRunningSetup sargs;
				sargs.SetEntrypoint(mod->GetListeningFunctionName(LISTENERNAME_ONLISTENUPDATE)).SetArguements(Args);
				// run the script //
				shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);
				// do something with result //
			}
		}
		break;
	}

}

DLLEXPORT bool Leviathan::Gui::BaseGuiObject::SetInternalRMLWrapper(string rmlcode){
	Element->SetInnerRML(rmlcode.c_str());
	return true;
}

// ------------------------------------ //





std::map<wstring, Rocket::Core::String> Leviathan::Gui::BaseGuiObject::LeviathanToRocketEventTranslate = boost::assign::map_list_of
	(LISTENERNAME_ONSHOW, "show")
	(LISTENERNAME_ONHIDE, "hide")

;

