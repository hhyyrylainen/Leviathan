// ------------------------------------ //
#include "GuiCollection.h"

#include "../CEGUIInclude.h"
#include "../Common/ThreadSafe.h"
#include "GuiManager.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Script/ScriptExecutor.h"
#include <boost/assign/list_of.hpp>
using namespace Leviathan;
using namespace GUI;
using namespace std;
// ------------------------------------ //
GuiCollection::GuiCollection(const std::string& name, GuiManager* manager, int id,
    const std::string& toggle, std::vector<unique_ptr<std::string>>& inanimations,
    std::vector<unique_ptr<std::string>>& outanimations,
    std::shared_ptr<ScriptScript> scripting, bool strict /*= false*/, bool enabled /*= true*/,
    bool keepgui /*= false*/, bool allowenable /*= true*/,
    const std::string& autotarget /*= ""*/, bool applyanimstochildren) :
    EventableScriptObject(scripting),
    Name(name), ID(id), Enabled(enabled), Strict(strict), KeepsGuiOn(keepgui),
    AllowEnable(allowenable), AutoTarget(autotarget),
    AutoAnimationOnEnable(std::move(inanimations)),
    AutoAnimationOnDisable(std::move(outanimations)),
    ApplyAnimationsToChildren(applyanimstochildren), OwningManager(manager)
{
    Toggle = GKey::GenerateKeyFromString(toggle);

    if(AutoAnimationOnEnable.size() % 2 != 0) {

        DEBUG_BREAK;
        LOG_ERROR("GuiCollection: AutoAnimationOnEnable size not dividable by 2");
    }

    if(AutoAnimationOnDisable.size() % 2 != 0) {

        DEBUG_BREAK;
        LOG_ERROR("GuiCollection: AutoAnimationOnEnable size not dividable by 2");
    }
}

GuiCollection::~GuiCollection()
{
    UnRegisterAllEvents();
    // release script //

    // release reference //
}
// ------------------------------------ //
DLLEXPORT void GuiCollection::UpdateState(bool newstate)
{
    // Don't do anything if the state didn't actually change //
    if(Enabled == newstate)
        return;

    // First update the object //
    Enabled = newstate;

    // notify GUI //
    OwningManager->PossiblyGUIMouseDisable();

    // Are we using auto animations for this? //

    if(Enabled && !AutoAnimationOnEnable.empty()) {

        // Play the animations //
        _PlayAnimations(AutoAnimationOnEnable);

    } else if(!Enabled && !AutoAnimationOnDisable.empty()) {

        // Play the animations //
        _PlayAnimations(AutoAnimationOnDisable);
    } else {

        // Set the auto target visibility if the target is set //
        if(!AutoTarget.empty()) {

            // Find it and set it //
            // Find the CEGUI object //
            CEGUI::Window* foundobject = NULL;
            try {

                foundobject =
                    OwningManager->GetMainContext()->getRootWindow()->getChild(AutoTarget);

            } catch(const CEGUI::UnknownObjectException& e) {

                // Not found //
                LOG_ERROR("GuiCollection: couldn't find an AutoTarget CEGUI window with "
                          " name: " +
                          AutoTarget + ":");

                LOG_WRITE("\t> " + string(e.what()));
            }

            if(foundobject) {

                // Set it's visible flag //
                foundobject->setVisible(Enabled);


            } else {

                // Call script?
            }
        }
    }

    // Call the script //
    if(Scripting) {

        // Attempt to call the thing
        Event* onevent = new Event(newstate ? EVENT_TYPE_SHOW : EVENT_TYPE_HIDE, nullptr);
        _CallScriptListener(onevent, nullptr);
        onevent->Release();
    }
}
// ------------------------------------ //
bool GuiCollection::LoadCollection(
    GuiManager* gui, const ObjectFileObject& data, const ExtraParameters& extra)
{
    // Load a GuiCollection from the structure //

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

    if(varlist) {

        // get values //
        if(!ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
               varlist->GetVariables(), "ToggleOn", Toggle, "")) {
            // Extra name check //
            ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
                varlist->GetVariables(), "Toggle", Toggle, "");
        }

        ObjectFileProcessor::LoadValueFromNamedVars<bool>(varlist->GetVariables(), "Enabled",
            Enabled, false, Logger::Get(), "GuiCollection: LoadCollection:");

        ObjectFileProcessor::LoadValueFromNamedVars<bool>(
            varlist->GetVariables(), "KeepsGUIOn", GuiOn, false);
        ObjectFileProcessor::LoadValueFromNamedVars<bool>(
            varlist->GetVariables(), "AllowEnable", allowenable, true);

        ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
            varlist->GetVariables(), "AutoTarget", autotarget, "");


        ObjectFileProcessor::LoadVectorOfTypeUPtrFromNamedVars<std::string>(
            varlist->GetVariables(), "AutoAnimationIn", autoinanimation, 2);
        ObjectFileProcessor::LoadVectorOfTypeUPtrFromNamedVars<std::string>(
            varlist->GetVariables(), "AutoAnimationOut", autooutanimation, 2);

        ObjectFileProcessor::LoadValueFromNamedVars<bool>(
            varlist->GetVariables(), "AutoAnimateChildren", recursiveanims, false);
    }


    // allocate new Collection object //
    GuiCollection* cobj = new GuiCollection(data.GetName(), gui, IDFactory::GetID(), Toggle,
        autoinanimation, autooutanimation, data.GetScript(), Strict, Enabled, GuiOn,
        allowenable, autotarget, recursiveanims);

    // Apply extra flags //
    if(cobj->Scripting) {

        if(auto mod = cobj->Scripting->GetModule(); mod) {

            mod->AddAccessRight(extra.ExtraAccess);
        }

        // This builds the listener list and register generic events
        // The module is also built here so this has to be after the access rights
        std::vector<std::shared_ptr<ValidListenerData>> listeners;
        cobj->Scripting->GetModule()->GetListOfListeners(listeners);

        cobj->RegisterStandardScriptEvents(listeners);

        // Call the Init function //
        auto event = Event::MakeShared<Event>(EVENT_TYPE_INIT, nullptr);
        cobj->OnEvent(event.get());
    }

    // Add to the collection list //
    gui->AddCollection(cobj);

    // loading succeeded //
    return true;
}

DLLEXPORT void GuiCollection::UpdateAllowEnable(bool newstate)
{
    AllowEnable = newstate;
}
// ------------------------------------ //
ScriptRunResult<int> GuiCollection::_DoCallWithParams(
    ScriptRunningSetup& sargs, Event* event, GenericEvent* event2)
{
    if(event)
        return ScriptExecutor::Get()->RunScript<int>(
            Scripting->GetModuleSafe(), sargs, this, event);
    else
        return ScriptExecutor::Get()->RunScript<int>(
            Scripting->GetModuleSafe(), sargs, this, event2);
}
// ------------------------------------ //
void GuiCollection::_PlayAnimations(const std::vector<unique_ptr<std::string>>& anims)
{

    LEVIATHAN_ASSERT((anims.size() % 2) == 0,
        "_PlayAnimations has invalid vector, size non divisable by 2");

    // Loop the animations and start them //
    for(size_t i = 0; i < anims.size(); i += 2) {

        const std::string& targetanim = *anims[i];

        if(targetanim == "AutoTarget") {

            if(!OwningManager->PlayAnimationOnWindow(AutoTarget, *anims[i + 1],
                   ApplyAnimationsToChildren, GuiManager::GetCEGUITypesWithBadAnimations())) {

                LOG_ERROR("GuiCollection: _PlayAnimations: failed to play animation(" +
                          *anims[i + 1] + ") on window " + AutoTarget);
            }

        } else {

            if(!OwningManager->PlayAnimationOnWindow(targetanim, *anims[i + 1],
                   ApplyAnimationsToChildren, GuiManager::GetCEGUITypesWithBadAnimations())) {

                LOG_ERROR("GuiCollection: _PlayAnimations: failed to play animation(" +
                          *anims[i + 1] + ") on window " + targetanim);
            }
        }
    }
}
