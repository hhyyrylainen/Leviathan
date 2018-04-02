// ------------------------------------ //
#include "EventableScriptObject.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT EventableScriptObject::EventableScriptObject(
    std::shared_ptr<ScriptScript> script /*= nullptr*/) :
    Scripting(script)
{
}

DLLEXPORT EventableScriptObject::~EventableScriptObject() {}
// ------------------------------------ //
DLLEXPORT void EventableScriptObject::RegisterStandardScriptEvents(
    const std::vector<std::shared_ptr<ValidListenerData>>& data)
{
    for(size_t i = 0; i < data.size(); i++) {
        if(data[i]->GenericTypeName && data[i]->GenericTypeName->size() > 0) {
            RegisterForEvent(*data[i]->GenericTypeName);
            continue;
        }

        EVENT_TYPE etype = ResolveStringToType(*data[i]->ListenerName);
        if(etype != EVENT_TYPE_ERROR) {

            RegisterForEvent(etype);
            continue;
        }

        // Ignore unknown events //
    }
}
// ------------------------------------ //
DLLEXPORT int EventableScriptObject::OnEvent(Event* event)
{
    // call script to handle the event //
    return _CallScriptListener(event, nullptr);
}

DLLEXPORT int EventableScriptObject::OnGenericEvent(GenericEvent* event)
{
    // call script to handle the event //
    return _CallScriptListener(nullptr, event);
}

DLLEXPORT bool EventableScriptObject::OnUpdate(
    const std::shared_ptr<NamedVariableList>& updated)
{
    ValuesUpdated = true;

    // push to update vector //
    UpdatedValues.push_back(updated);

    // fire an event //
    Event* tmpevent = new Event(EVENT_TYPE_LISTENERVALUEUPDATED, NULL);

    OnEvent(tmpevent);

    tmpevent->Release();
    return true;
}
// ------------------------------------ //
int EventableScriptObject::_CallScriptListener(Event* event, GenericEvent* event2)
{
    if(!Scripting)
        return -1;

    ScriptModule* mod = Scripting->GetModule();
    ScriptRunningSetup sargs;

    if(event) {
        // Get the listener name from the event type //
        const std::string& listenername = GetListenerNameFromType(event->GetType());

        // check does the script contain right listeners //
        if(!mod->DoesListenersContainSpecificListener(listenername))
            return -1;

        sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername));
    } else {
        // generic event is passed //
        if(!mod->DoesListenersContainSpecificListener("", event2->GetTypePtr()))
            return -1;

        sargs.SetEntrypoint(mod->GetListeningFunctionName("", event2->GetTypePtr()));
    }

    // Run the script //
    auto result = _DoCallWithParams(sargs, event, event2);

    if(result.Result != SCRIPT_RUN_RESULT::Success) {
        LOG_WARNING("EventableScriptObject: failed to run script listener");
        return 0;
    }

    return result.Value;
}
