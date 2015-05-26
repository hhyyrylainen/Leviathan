// ------------------------------------ //
#include "GuiCollection.h"

#include <boost/assign/list_of.hpp>
#include "Script/ScriptExecutor.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "GuiManager.h"
#include "../CEGUIInclude.h"
#include "../Common/ThreadSafe.h"
using namespace Leviathan;
using namespace Gui;
using namespace std;
// ------------------------------------ //
Leviathan::Gui::GuiCollection::GuiCollection(const std::string &name, GuiManager* manager, int id,
    const std::string &toggle, std::vector<unique_ptr<std::string>> &inanimations,
    std::vector<unique_ptr<std::string>> &outanimations, bool strict /*= false*/, 
	bool enabled /*= true*/, bool keepgui /*= false*/, bool allowenable /*= true*/,
    const std::string &autotarget /*= ""*/, 

	bool applyanimstochildren) : 
	Name(name), ID(id), Enabled(enabled), Strict(strict), KeepsGuiOn(keepgui),
    OwningManager(manager), AllowEnable(allowenable), 
	AutoTarget(autotarget), ApplyAnimationsToChildren(applyanimstochildren),
    AutoAnimationOnEnable(move(inanimations)), 
	AutoAnimationOnDisable(move(outanimations))
{
	Toggle = GKey::GenerateKeyFromString(toggle);
}

Leviathan::Gui::GuiCollection::~GuiCollection(){
	// release script //

	// release reference //
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiCollection::UpdateState(Lock &managerlock, bool newstate){
	// Don't do anything if the state didn't actually change //
	if(Enabled == newstate)
		return;

	// First update the object //
	Enabled = newstate;

	// notify GUI //
	OwningManager->PossiblyGUIMouseDisable();

	// Are we using auto animations for this? //

	if(Enabled && !AutoAnimationOnEnable.empty()){

		// Play the animations //
		_PlayAnimations(managerlock, AutoAnimationOnEnable);
		return;

	} else if(!Enabled && !AutoAnimationOnDisable.empty()){
		
		// Play the animations //
		_PlayAnimations(managerlock, AutoAnimationOnDisable);
		return;
	}

	// Set the auto target visibility if the target is set //
	if(!AutoTarget.empty()){

		// Find it and set it //
		// Find the CEGUI object //
		CEGUI::Window* foundobject = NULL;
		try{

			foundobject = OwningManager->GetMainContext(managerlock)
                ->getRootWindow()->getChild(AutoTarget);

		} catch(const CEGUI::UnknownObjectException &e){

			// Not found //
			Logger::Get()->Error("GuiCollection: couldn't find an AutoTarget CEGUI window with "
                " name: "+AutoTarget+":");
            
			Logger::Get()->Write("\t> "+string(e.what()));
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

		const std::string &listenername = newstate ? LISTENERNAME_ONSHOW: LISTENERNAME_ONHIDE;
		
		if(mod->DoesListenersContainSpecificListener(listenername)){
			// create event to use //
			Event* onevent = new Event(EVENT_TYPE_SHOW, new ShowEventData(newstate));

			// call it //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(
                new NamedVariableBlock(new VoidPtrBlock(this), "GuiCollection"))
				(new NamedVariableBlock(new VoidPtrBlock(onevent), "Event"));

			onevent->AddRef();
			AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername)).SetArguments(Args);

			ScriptExecutor::Get()->RunSetUp(tmpscript, &sargs);

			onevent->Release();
		}
	}
}
// ------------------------------------ //
bool Leviathan::Gui::GuiCollection::LoadCollection(Lock &guilock, GuiManager* gui,
    const ObjectFileObject &data)
{
	// load a GuiCollection from the structure //

	std::string Toggle = "";
	bool Enabled = true;
	bool Strict = false;
	bool GuiOn = false;
	bool allowenable = true;
	std::string autotarget = "";
	std::vector<unique_ptr<std::string>> autoinanimation;
	std::vector<unique_ptr<std::string>> autooutanimation;
	bool recursiveanims = false;

	auto varlist = data.GetListWithName("params");

	if(varlist){

		// get values //
		if(!ObjectFileProcessor::LoadValueFromNamedVars<std::string>(varlist->GetVariables(),
                "ToggleOn", Toggle, "", false)){
			// Extra name check //
			ObjectFileProcessor::LoadValueFromNamedVars<std::string>(varlist->GetVariables(),
                "Toggle", Toggle, "", false);
		}

		ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist->GetVariables(), "Enabled",
            Enabled, false, true,
			"GuiCollection: LoadCollection:");

		ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist->GetVariables(), "KeepsGUIOn",
            GuiOn, false);
		ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist->GetVariables(), "AllowEnable",
            allowenable, true);

		ObjectFileProcessor::LoadValueFromNamedVars<std::string>(varlist->GetVariables(),
            "AutoTarget", autotarget, "");
		

		ObjectFileProcessor::LoadVectorOfTypeUPtrFromNamedVars<std::string>(varlist->GetVariables(),
            "AutoAnimationIn", autoinanimation, 2);
		ObjectFileProcessor::LoadVectorOfTypeUPtrFromNamedVars<std::string>(varlist->GetVariables(),
            "AutoAnimationOut", autooutanimation, 2);

		ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist->GetVariables(),
            "AutoAnimateChildren", recursiveanims, false);
		
	}


	// allocate new Collection object //
	GuiCollection* cobj = new GuiCollection(data.GetName(), gui, IDFactory::GetID(), Toggle,
        autoinanimation, autooutanimation, Strict, Enabled, 
		GuiOn, allowenable, autotarget, recursiveanims);
    
	// copy script data over //
	cobj->Scripting = data.GetScript();

	// Add to the collection list //
	gui->AddCollection(guilock, cobj);

	// loading succeeded //
	return true;
}

DLLEXPORT void Leviathan::Gui::GuiCollection::UpdateAllowEnable(bool newstate){
	AllowEnable = newstate;
}
// ------------------------------------ //
void Leviathan::Gui::GuiCollection::_PlayAnimations(Lock &lock, const std::vector<unique_ptr<std::string>> &anims){

	assert(anims.size() % 2 == 0 && "_PlayAnimations has invalid vector, size non dividable by 2");

	// Loop the animations and start them //
	for(size_t i = 0; i < anims.size(); i += 2){

		const std::string& targetanim = *anims[i];

		if(targetanim == "AutoTarget"){

			if(!OwningManager->PlayAnimationOnWindow(lock, AutoTarget, *anims[i+1],
                    ApplyAnimationsToChildren, GuiManager::GetCEGUITypesWithBadAnimations()))
			{

				Logger::Get()->Error("GuiCollection: _PlayAnimations: failed to play animation("+
                    *anims[i+1]+") on window "+AutoTarget);
			}

		} else {

			if(!OwningManager->PlayAnimationOnWindow(lock, targetanim, *anims[i+1],
                    ApplyAnimationsToChildren, GuiManager::GetCEGUITypesWithBadAnimations()))
			{

				Logger::Get()->Error("GuiCollection: _PlayAnimations: failed to play animation("+
                    *anims[i+1]+") on window "+targetanim);
			}
		}
	}
}

