#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EVENTABLESCRIPTOBJECT
#include "EventableScriptObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::EventableScriptObject::EventableScriptObject(shared_ptr<ScriptScript> script /*= nullptr*/) : Scripting(script){

}

DLLEXPORT Leviathan::EventableScriptObject::~EventableScriptObject(){

}
// ------------------------------------ //
DLLEXPORT int Leviathan::EventableScriptObject::OnEvent(Event** pEvent){
	// call script to handle the event //
	_CallScriptListener(pEvent, NULL);

	return 0;
}

DLLEXPORT int Leviathan::EventableScriptObject::OnGenericEvent(GenericEvent** pevent){
	// call script to handle the event //
	_CallScriptListener(NULL, pevent);

	return 0;
}

DLLEXPORT bool Leviathan::EventableScriptObject::OnUpdate(const shared_ptr<NamedVariableList> &updated){
	ValuesUpdated = true;

	// push to update vector //
	UpdatedValues.push_back(updated);

	// fire an event //
	Event* tmpevent = new Event(EVENT_TYPE_LISTENERVALUEUPDATED, NULL);

	OnEvent(&tmpevent);

	tmpevent->Release();
	return true;
}
// ------------------------------------ //


