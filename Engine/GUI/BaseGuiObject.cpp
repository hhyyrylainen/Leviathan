#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASEOBJECT
#include "BaseGuiObject.h"
#endif
#include "Utility\Iterators\WstringIterator.h"
#include "ObjectFiles\ObjectFileProcessor.h"
#include "Script\ScriptModule.h"
#include <boost\assign\list_of.hpp>
#include "GUI\GuiManager.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::BaseGuiObject::BaseGuiObject(GuiManager* owner, const wstring &name, int fakeid, int sheetid, shared_ptr<ScriptScript> 
	script /*= NULL*/) : OwningInstance(owner), FileID(fakeid), Name(name), Scripting(script), Element(NULL), SheetID(sheetid), ManualDetach(false)
{
	ID = IDFactory::GetID();
}

DLLEXPORT Leviathan::Gui::BaseGuiObject::~BaseGuiObject(){
	// script has smart pointer //

	// this should always be safe //
	_UnhookAllListeners();
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

	unique_ptr<BaseGuiObject> tmpptr(new BaseGuiObject(owner, dataforthis.Name, fakeid, sheet->GetID(), dataforthis.Script));

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
				// set element id //
				tmpptr->RocketObjectName = rocketobjectname;
			}

			listenon = dataforthis.Contents[i]->Variables.GetValueDirect(L"ListenOn");
		}
	}
	if(element){
		tmpptr->Element = element;
	}/* else {
	 Logger::Get()->Warning(L"BaseGuiObject: LoadFromFileStructure: probably missing 'l params{' block in file, name: "+tmpptr->Name);
	 }*/
	// listening start //
	if(listenon.get()){
		tmpptr->StartMonitoring(listenon->GetValues());
	}


	if(tmpptr.get()){
		tmpptr->_HookListeners();
		tempobjects.push_back(tmpptr.release());
		return true;
	}

	return false;
}
// ------------------------------------ //
void Leviathan::Gui::BaseGuiObject::_HookListeners(bool onlyrocket /*= false*/){
	// first we need to get probably right listeners and register with them //
	ScriptModule* mod = Scripting->GetModule();

	std::vector<wstring> containedlisteners;

	mod->GetListOfListeners(containedlisteners);

	for(size_t i = 0; i < containedlisteners.size(); i++){
		Rocket::Core::String tohook;
		try{
			tohook = LeviathanToRocketEventTranslate[containedlisteners[i]];

			if(tohook.Length() > 0){
				throw exception("check the other");
			}

		} catch(...){

			if(onlyrocket)
				continue;
			// non-Rocket listener //

			// look for global events //

			// custom event //

			continue;
		}
		if(Element){

			Element->AddEventListener(tohook, this);
			// store to be able to detach later //
			HookedRocketEvents.push_back(tohook);
			
		} else {
			// warn about this //
			Logger::Get()->Warning(L"BaseGuiObject: _HookRocketListeners: couldn't hook Rocket event "+Convert::StringToWstring(tohook.CString()));
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
	// unset when Rocket destroys the element //
	if(!ManualDetach){
		Element = NULL;
		// don't want to accidentally do unhooking //
		HookedRocketEvents.clear();
	}
}

void Leviathan::Gui::BaseGuiObject::OnAttach(Rocket::Core::Element* element){
	// attached //
	Logger::Get()->Info(L"GuiObject: "+Name+L" has been attached");
}

void Leviathan::Gui::BaseGuiObject::ProcessEvent(Rocket::Core::Event& receivedevent){
	// call script to handle the event //
	const Rocket::Core::String& eventtype = receivedevent.GetType();

	wstring listenername = L""; 

	// check does the script contain right listeners //
	ScriptModule* mod = Scripting->GetModule();

	// handle specific Rocket events //
	if(eventtype == "click"){

		if(mod->DoesListenersContainSpecificListener(LISTENERNAME_ONCLICK)){

			listenername = LISTENERNAME_ONCLICK;
		}

	} else if(eventtype == "show"){

		if(mod->DoesListenersContainSpecificListener(LISTENERNAME_ONSHOW)){

			listenername = LISTENERNAME_ONSHOW;
		}

	}
	// check if we want to call something //
	if(listenername.size() > 0){

		// setup parameters //
		vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"BaseGuiObject"))
			(new NamedVariableBlock(new VoidPtrBlock(&receivedevent), L"RocketEvent"));
		// we are returning ourselves so increase refcount
		AddRef();
		receivedevent.AddReference();

		ScriptRunningSetup sargs;
		sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername)).SetArguements(Args);
		// run the script //
		shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);

		// handle return value //
		int res = -1;
		if(result->ConvertAndAssingToVariable(res)){

			if(res == 1)
				receivedevent.StopPropagation();
		}
	}


}

void Leviathan::Gui::BaseGuiObject::_UnhookAllListeners(){
	// unregister all non-Rocket events //
	UnRegisterAllEvents();

	if(HookedRocketEvents.empty() || Element == NULL)
		return;

	ManualDetach = true;

	for(std::list<Rocket::Core::String>::iterator iter = HookedRocketEvents.begin(); iter != HookedRocketEvents.end(); iter++){

		Rocket::Core::String unregister = *iter;

		Element->RemoveEventListener(unregister, this);
	}
	HookedRocketEvents.clear();
	ManualDetach = false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::BaseGuiObject::CheckObjectLinkage(){
	if(Element)
		return true;

	// not linked check if we can get valid object with our id //
	shared_ptr<GuiLoadedSheet> sheet = OwningInstance->GetSheet(SheetID);
	if(tmp){
		// search //
		Element = sheet->GetElementByID(Convert::WstringToString(RocketObjectName));
	}

	if(Element){
		// link just Rocket events //
		_HookListeners(true);
	}
}
// ------------------------------------ //
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
	(LISTENERNAME_ONCLICK, "click")
;

