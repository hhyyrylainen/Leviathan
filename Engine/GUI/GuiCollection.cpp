#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUICOLLECTION
#include "GuiCollection.h"
#endif
#include <boost/assign/list_of.hpp>
#include "Script/ScriptInterface.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "GuiManager.h"
using namespace Leviathan;
using namespace Gui;
// ------------------------------------ //
Leviathan::Gui::GuiCollection::GuiCollection(const wstring &name, GuiManager* manager, int id, const wstring &toggle, 
	bool strict /*= false*/, bool enabled /*= true*/, bool keepgui, bool allowenable) : Name(name), ID(id), Enabled(enabled), Strict(strict), 
	KeepsGuiOn(keepgui), OwningManager(manager), AllowEnable(allowenable)
{
	Toggle = GKey::GenerateKeyFromString(toggle);
}

Leviathan::Gui::GuiCollection::~GuiCollection(){
	// release script //

	// release reference //
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiCollection::UpdateState(bool newstate){
	// call script //
	ScriptScript* tmpscript = Scripting.get();

	if(tmpscript){
		// check does the script contain right listeners //
		ScriptModule* mod = tmpscript->GetModule();

		const wstring &listenername = newstate ? LISTENERNAME_ONSHOW: LISTENERNAME_ONHIDE;
		
		if(mod->DoesListenersContainSpecificListener(listenername)){
			// create event to use //
			Event* onevent = new Event(EVENT_TYPE_SHOW, new ShowEventData(false));

			// call it //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"GuiCollection"))
				(new NamedVariableBlock(new VoidPtrBlock(onevent), L"Event"));

			onevent->AddRef();
			AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername)).SetArguments(Args);

			ScriptInterface::Get()->ExecuteScript(tmpscript, &sargs);

			onevent->Release();
		}
	}

	Enabled = newstate;

	// notify GUI //
	OwningManager->PossiblyGUIMouseDisable();
}
// ------------------------------------ //
bool Leviathan::Gui::GuiCollection::LoadCollection(GuiManager* gui, const ObjectFileObject &data){
	// load a GuiCollection from the structure //

	wstring Toggle = L"";
	bool Enabled = true;
	bool Strict = false;
	bool GuiOn = false;
	bool allowenable = true;

	auto varlist = data.GetListWithName(L"params");

	if(varlist){

		// get values //
		if(!ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist->GetVariables(), L"ToggleOn", Toggle, L"", false)){
			// Extra name check //
			ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist->GetVariables(), L"Toggle", Toggle, L"", false);
		}

		ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist->GetVariables(), L"Enabled", Enabled, false, true,
			L"GuiCollection: LoadCollection:");

		ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist->GetVariables(), L"KeepsGUIOn", GuiOn, false);
		ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist->GetVariables(), L"AllowEnable", allowenable, true);
	}


	// allocate new Collection object //
	GuiCollection* cobj = new GuiCollection(data.GetName(), gui, IDFactory::GetID(), Toggle, Strict, Enabled, GuiOn, allowenable);
	// copy script data over //
	cobj->Scripting = data.GetScript();

	// Add to the collection list //
	gui->AddCollection(cobj);

	// loading succeeded //
	return true;
}

DLLEXPORT void Leviathan::Gui::GuiCollection::UpdateAllowEnable(bool newstate){
	AllowEnable = newstate;
}

