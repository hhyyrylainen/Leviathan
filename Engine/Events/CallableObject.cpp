#include "Include.h"
#ifndef LEVIATHAN_CALLABLE_OBJECT
#include "CallableObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "EventHandler.h"

CallableObject::CallableObject(){

}
CallableObject::~CallableObject(){

}
// ------------------------------------ //
void Leviathan::CallableObject::UnRegister(EVENT_TYPE from, bool all){
	EventHandler::Get()->Unregister(this, from, all);
}

void Leviathan::CallableObject::UnRegister(const wstring &genericname, bool all /*= false*/){
	EventHandler::Get()->Unregister(this, genericname, all);
}

void Leviathan::CallableObject::RegisterForEvent(EVENT_TYPE toregister){
	EventHandler::Get()->RegisterForEvent(this, toregister);
}

void Leviathan::CallableObject::RegisterForEvent(const wstring &genericname){
	EventHandler::Get()->RegisterForEvent(this, genericname);
}

DLLEXPORT int Leviathan::CallableObject::OnEvent(Event** pEvent){
	return -1;
}

DLLEXPORT int Leviathan::CallableObject::OnGenericEvent(GenericEvent** pevent){
	return -1;
}

void Leviathan::CallableObject::UnRegisterAllEvents(){
	EventHandler::Get()->Unregister(this, EVENT_TYPE_ALL, true);
	EventHandler::Get()->Unregister(this, L"", true);
}
// ------------------------------------ //
DLLEXPORT EVENT_TYPE Leviathan::CallableObject::ResolveStringToType(const wstring &type){
	// lookup map and return //
	auto iter = EventListenerNameToEventMap.find(type);

	if(iter != EventListenerNameToEventMap.end()){

		return iter->second;
	}
	return EVENT_TYPE_ERROR;
}

DLLEXPORT wstring Leviathan::CallableObject::GetListenerNameFromType(EVENT_TYPE type){
	for(auto iter = EventListenerNameToEventMap.begin(); iter != EventListenerNameToEventMap.end(); ++iter){
		// when the type matches return the string associated with that value//
		if(iter->second == type)
			return iter->first;
	}
	return L"";
}
