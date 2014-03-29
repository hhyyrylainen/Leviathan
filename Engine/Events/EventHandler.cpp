#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EVENTHANDLER
#include "EventHandler.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
EventHandler::EventHandler(){
	main = this;
}
EventHandler::~EventHandler(){

}

DLLEXPORT EventHandler* Leviathan::EventHandler::Get(){
	return main;
}

EventHandler* Leviathan::EventHandler::main = NULL;
// ------------------------------------ //
bool EventHandler::Init(){
	return true;
}
void EventHandler::Release(){
	GUARD_LOCK_THIS_OBJECT();
	// release listeners //
	SAFE_DELETE_VECTOR(EventListeners);
	SAFE_DELETE_VECTOR(GenericEventListeners);
}
// ------------------------------------ //
void EventHandler::CallEvent(Event* pEvent){
	GUARD_LOCK_THIS_OBJECT();
	// Loop and call all listeners which have a valid type //
	for(size_t i = 0; i < EventListeners.size(); i++){

		if(EventListeners[i]->Type == pEvent->GetType()){

			EventListeners[i]->Receiver->OnEvent(&pEvent);
			if((pEvent) == NULL)
				// callable destroyed message //
				break;

		}
	}
	SAFE_RELEASE(pEvent);
}

DLLEXPORT void Leviathan::EventHandler::CallEvent(GenericEvent* pEvent){
	GUARD_LOCK_THIS_OBJECT();
	// Loop generic listeners //
	for(size_t i = 0; i < GenericEventListeners.size(); i++){

		if(GenericEventListeners[i]->Type == pEvent->GetType()){

			GenericEventListeners[i]->Receiver->OnGenericEvent(&pEvent);
			if((pEvent) == NULL)
				// callable destroyed message //
				break;

		}
	}
	SAFE_RELEASE(pEvent);
}
// ------------------------------------ //
bool EventHandler::RegisterForEvent(CallableObject* toregister, EVENT_TYPE totype){
	GUARD_LOCK_THIS_OBJECT();
	EventListeners.push_back(new RegisteredCallback(toregister, totype));
	return true;
}

DLLEXPORT bool Leviathan::EventHandler::RegisterForEvent(CallableObject* toregister, const wstring &genericname){
	GUARD_LOCK_THIS_OBJECT();
	GenericEventListeners.push_back(new GenericRegisteredCallback(toregister, genericname));
	return true;
}

void EventHandler::Unregister(CallableObject* caller, EVENT_TYPE type, bool all){
	GUARD_LOCK_THIS_OBJECT();
	// loop and remove wanted objects //
	for(size_t i = 0; i < EventListeners.size(); i++){
		if(EventListeners[i]->Receiver == caller){
			// check type or if all is specified delete //
			if(all || type == EventListeners[i]->Type){
				delete EventListeners[i];
				EventListeners.erase(EventListeners.begin()+i);
				i--;
			}
		}
	}
}

DLLEXPORT void Leviathan::EventHandler::Unregister(CallableObject* caller, const wstring &genericname, bool all /*= false*/){
	GUARD_LOCK_THIS_OBJECT();
	// loop and remove wanted objects //
	for(size_t i = 0; i < GenericEventListeners.size(); i++){
		if(GenericEventListeners[i]->Receiver == caller){
			// check type or if all is specified delete //
			if(all || GenericEventListeners[i]->Type == genericname){
				delete GenericEventListeners[i];
				GenericEventListeners.erase(GenericEventListeners.begin()+i);
				i--;
			}
		}
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::EventHandler::CallEventGenericProxy(GenericEvent* genericevent){
	CallEvent(genericevent);
}