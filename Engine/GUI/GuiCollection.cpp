#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUICOLLECTION
#include "GuiCollection.h"
#endif
#include <boost\assign\list_of.hpp>
#include "Script\ScriptInterface.h"
#include "ObjectFiles\ObjectFileProcessor.h"
#include "GuiManager.h"
using namespace Leviathan;
// ------------------------------------ //
Leviathan::Gui::GuiCollection::GuiCollection(const wstring &name, shared_ptr<GuiLoadedSheet> sheet, int id, const wstring &toggle, 
	bool strict /*= false*/, bool enabled /*= true*/) : Name(name), ID(id), Enabled(enabled), Strict(strict), 
	ContainedInSheet(sheet)
{
	Toggle = GKey::GenerateKeyFromString(toggle);
}

Leviathan::Gui::GuiCollection::~GuiCollection(){
	// release script //

}
// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiCollection::UpdateState(bool newstate){
	// call script //
	ScriptScript* tmpscript = Scripting.get();

	DEBUG_BREAK;

	if(tmpscript){
		// check does the script contain right listeners //
		ScriptModule* mod = tmpscript->GetModule();

		const wstring &listenername = newstate ? L"OnEnable": L"OnDisable";

		if(mod->DoesListenersContainSpecificListener(listenername)){
			// call it //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new IntBlock(0), L"source"))
				(new NamedVariableBlock(new IntBlock(ID), L"Instance"));

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(Convert::WstringToString(listenername)).SetArguements(Args);

			ScriptInterface::Get()->ExecuteScript(tmpscript, &sargs);
		}
	}

	Enabled = newstate;
}
// ------------------------------------ //
bool Leviathan::Gui::GuiCollection::LoadCollection(GuiManager* gui, const ObjectFileObject &data, shared_ptr<GuiLoadedSheet> sheet){
	// load a GuiCollection from the structure //

	wstring Toggle = L"";
	bool Enabled = true;
	bool Strict = false;

	for(size_t a = 0; a < data.Contents.size(); a++){

		if(data.Contents[a]->Name == L"params"){
			// get values //
			ObjectFileProcessor::LoadValueFromNamedVars<wstring>(data.Contents[a]->Variables, L"ToggleOn", Toggle, L"", true,
				L"GuiCollection: LoadCollection:");

			ObjectFileProcessor::LoadValueFromNamedVars<bool>(data.Contents[a]->Variables, L"Enabled", Enabled, false, true,
				L"GuiCollection: LoadCollection:");

			continue;
		}
	}

	// allocate new Collection object //
	GuiCollection* cobj = new GuiCollection(data.Name, sheet, IDFactory::GetID(),Toggle, Strict, Enabled);
	// copy script data over //
	cobj->Scripting = data.Script;
	// add to collection list //
	gui->AddCollection(cobj);

	// loading succeeded //
	return true;
}
// ------------------ GuiLoadedSheet ------------------ //
Leviathan::Gui::GuiLoadedSheet::GuiLoadedSheet(Rocket::Core::Context* context, const string &documentfile){
	// load the Rocket file here //
	Document = context->LoadDocument(documentfile.c_str());

	if(!Document)
		throw ExceptionInvalidArguement(L"invalid file provided", 0, __WFUNCTION__, L"documentfile", Convert::StringToWstring(documentfile));

	Document->Show();
}

Leviathan::Gui::GuiLoadedSheet::~GuiLoadedSheet(){
	// remove the reference so that it gets unloaded at some point //
	Document->RemoveReference();
	Document = NULL;
}

DLLEXPORT Rocket::Core::Element* Leviathan::Gui::GuiLoadedSheet::GetElementByID(const string &id){

	return Document->GetElementById(id.c_str());
}


