#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASEOBJECT
#include "BaseGuiObject.h"
#endif
#include "Utility/Iterators/WstringIterator.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Script/ScriptModule.h"
#include <boost/assign/list_of.hpp>
#include "GUI/GuiManager.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::BaseGuiObject::BaseGuiObject(GuiManager* owner, const wstring &name, int fakeid, GuiLoadedSheet* sheet,
	shared_ptr<ScriptScript> script /*= NULL*/) : EventableScriptObject(script), OwningInstance(owner), FileID(fakeid), Name(name), Element(NULL),
	ContainedInSheet(sheet), ManualDetach(false), ID(IDFactory::GetID())
{
	
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

	unique_ptr<BaseGuiObject> tmpptr(new BaseGuiObject(owner, dataforthis.Name, fakeid, sheet, dataforthis.Script));

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

	std::vector<shared_ptr<ValidListenerData>> containedlisteners;

	mod->GetListOfListeners(containedlisteners);


	for(size_t i = 0; i < containedlisteners.size(); i++){
		// generics cannot be rocket events //
		if(containedlisteners[i]->GenericTypeName && containedlisteners[i]->GenericTypeName->size() > 0){
			if(onlyrocket)
				continue;
			// custom event listener //

			RegisterForEvent(*containedlisteners[i]->GenericTypeName);

			continue;
		}

		Rocket::Core::String tohook;
		try{
			tohook = LeviathanToRocketEventTranslate[*containedlisteners[i]->ListenerName];

			if(tohook.Length() == 0){
			    // check the other //
				throw exception();
			}

		} catch(...){

			if(onlyrocket)
				continue;
			// non-Rocket listener //

			// check for specific case //
			if(*containedlisteners[i]->ListenerName == L"Init"){
				// call now the init event //

			} else if(*containedlisteners[i]->ListenerName == L"Release"){
				// call later the release event //

			}

			// look for global events //
			EVENT_TYPE etype = ResolveStringToType(*containedlisteners[i]->ListenerName);
			if(etype != EVENT_TYPE_ERROR){

				RegisterForEvent(etype);
				continue;
			}

			Logger::Get()->Warning(L"BaseGuiObject: _HookListeners: unknown event type "+*containedlisteners[i]->ListenerName+L", did you intent to use Generic type?");

			continue;
		}
		if(Element){

			Element->AddEventListener(tohook, this);
			// store to be able to detach later //
			HookedRocketEvents.push_back(tohook);

		} else {
			// warn about this //
			//Logger::Get()->Warning(L"BaseGuiObject: _HookListeners: couldn't hook Rocket event "+Convert::StringToWstring(tohook.CString()));
		}
	}
}
// ------------------ Rocket callbacks ------------------ //
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

	// Get the listener name //
	VerifyInversedEventTranslateMap();

	auto iter = RocketEventToLeviathanListenerTranslate.find(eventtype);

	if(iter == RocketEventToLeviathanListenerTranslate.end()){

		Logger::Get()->Warning(L"BaseGuiObject: ProcessEvent: cannot translate rocket event "+Convert::StringToWstring(eventtype.CString())+
			L" to Leviathan listener value");
		return;
	}

	// check does the script contain right listeners //
	ScriptModule* mod = Scripting->GetModule();

	if(mod->DoesListenersContainSpecificListener(iter->second)){
		// Call the script callback //
		// setup parameters //
		vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"BaseGuiObject"))
			(new NamedVariableBlock(new VoidPtrBlock(&receivedevent), L"RocketEvent"));
		// we are returning ourselves so increase refcount
		AddRef();
		receivedevent.AddReference();

		ScriptRunningSetup sargs;
		sargs.SetEntrypoint(mod->GetListeningFunctionName(iter->second)).SetArguments(Args);
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

	// search //
	Element = ContainedInSheet->GetElementByID(Convert::WstringToString(RocketObjectName));


	if(Element){
		// link just Rocket events //
		_HookListeners(true);
		Logger::Get()->Info(L"BaseGuiObject: CheckObjectLinkage: successfully linked with new object");
		return true;
	}

	return false;
}
// ------------------------------------ //
void Leviathan::Gui::BaseGuiObject::_CallScriptListener(Event** pEvent, GenericEvent** event2){

	ScriptModule* mod = Scripting->GetModule();

	if(pEvent){
		// Get the listener name from the event type //
		wstring listenername = GetListenerNameFromType((*pEvent)->GetType());

		// check does the script contain right listeners //
		if(mod->DoesListenersContainSpecificListener(listenername)){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"BaseGuiObject"))
				(new NamedVariableBlock(new VoidPtrBlock(*pEvent), L"Event"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*pEvent)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername)).SetArguments(Args);
			// run the script //
			shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);
			// do something with result //
		}
	} else {
		// generic event is passed //
		if(mod->DoesListenersContainSpecificListener(L"", (*event2)->TypeStr)){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"BaseGuiObject"))
				(new NamedVariableBlock(new VoidPtrBlock(*event2), L"GenericEvent"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*event2)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(L"", (*event2)->TypeStr)).SetArguments(Args);
			// run the script //
			shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);
			// do something with result //
		}
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
	(LISTENERNAME_ONVALUECHANGE, "change")
;

// This starts off empty //
std::map<Rocket::Core::String, wstring> Leviathan::Gui::BaseGuiObject::RocketEventToLeviathanListenerTranslate;

// But this function fills it //
DLLEXPORT void Leviathan::Gui::BaseGuiObject::VerifyInversedEventTranslateMap(){
	if(RocketEventToLeviathanListenerTranslate.size() != 0)
		return;

	// Inverse the other map and insert it to this map //
	for(auto iter = LeviathanToRocketEventTranslate.begin(); iter != LeviathanToRocketEventTranslate.end(); ++iter){

		RocketEventToLeviathanListenerTranslate[iter->second] = iter->first;
	}

}
