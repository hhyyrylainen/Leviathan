#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EVENTABLESCRIPTOBJECT
#include "EventableScriptObject.h"
#endif
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::EventableScriptObject::EventableScriptObject(
    shared_ptr<ScriptScript> script /*= nullptr*/) :
    Scripting(script)
{

}

DLLEXPORT Leviathan::EventableScriptObject::~EventableScriptObject(){

}
// ------------------------------------ //
DLLEXPORT int Leviathan::EventableScriptObject::OnEvent(Event* event){
	// call script to handle the event //
	_CallScriptListener(event, nullptr);

	return 0;
}

DLLEXPORT int Leviathan::EventableScriptObject::OnGenericEvent(GenericEvent* event){
	// call script to handle the event //
	_CallScriptListener(nullptr, event);

	return 0;
}

DLLEXPORT bool Leviathan::EventableScriptObject::OnUpdate(
    const std::shared_ptr<NamedVariableList> &updated)
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


