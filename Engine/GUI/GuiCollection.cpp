#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUICOLLECTION
#include "GuiCollection.h"
#endif
#include <boost/assign/list_of.hpp>
#include "Script/ScriptInterface.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "GuiManager.h"
#include "CEGUI/GUIContext.h"
#include "CEGUI/Window.h"
using namespace Leviathan;
using namespace Gui;
// ------------------------------------ //
Leviathan::Gui::GuiCollection::GuiCollection(const wstring &name, GuiManager* manager, int id, const wstring &toggle, 
	bool strict /*= false*/, bool enabled /*= true*/, bool keepgui, bool allowenable, const wstring &autotarget) : 
	Name(name), ID(id), Enabled(enabled), Strict(strict), KeepsGuiOn(keepgui), OwningManager(manager), AllowEnable(allowenable), 
		AutoTarget(autotarget)
{
	Toggle = GKey::GenerateKeyFromString(toggle);
}

Leviathan::Gui::GuiCollection::~GuiCollection(){
	// release script //

	// release reference //
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiCollection::UpdateState(bool newstate){
	// First update the object //
	Enabled = newstate;

	// notify GUI //
	OwningManager->PossiblyGUIMouseDisable();


	// Set the auto target visibility if the target is set //
	if(!AutoTarget.empty()){

		// Find it and set it //
		// Find the CEGUI object //
		CEGUI::Window* foundobject = NULL;
		try{

			foundobject = OwningManager->GetMainContext()->getRootWindow()->getChild(Convert::WstringToString(AutoTarget));

		} catch(const CEGUI::UnknownObjectException &e){

			// Not found //
			Logger::Get()->Error(L"GuiCollection: couldn't find an AutoTarget CEGUI window with name: "+AutoTarget+L":");
			Logger::Get()->Write(L"\t> "+Convert::CharPtrToWstring(e.what()));
		}

		if(foundobject){

			// Set it's visible flag //
			foundobject->setVisible(Enabled);


		} else {

			// Call script?
		}

		return;
	}


	// Call the script //
	ScriptScript* tmpscript = Scripting.get();

	if(tmpscript){
		// check does the script contain right listeners //
		ScriptModule* mod = tmpscript->GetModule();

		const wstring &listenername = newstate ? LISTENERNAME_ONSHOW: LISTENERNAME_ONHIDE;
		
		if(mod->DoesListenersContainSpecificListener(listenername)){
			// create event to use //
			Event* onevent = new Event(EVENT_TYPE_SHOW, new ShowEventData(newstate));

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
}
// ------------------------------------ //
bool Leviathan::Gui::GuiCollection::LoadCollection(GuiManager* gui, const ObjectFileObject &data){
	// load a GuiCollection from the structure //

	wstring Toggle = L"";
	bool Enabled = true;
	bool Strict = false;
	bool GuiOn = false;
	bool allowenable = true;
	wstring autotarget = L"";

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

		ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist->GetVariables(), L"AutoTarget", autotarget, L"");
	}


	// allocate new Collection object //
	GuiCollection* cobj = new GuiCollection(data.GetName(), gui, IDFactory::GetID(), Toggle, Strict, Enabled, GuiOn, allowenable, autotarget);
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

