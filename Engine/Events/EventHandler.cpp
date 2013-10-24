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
	// release listeners //
	SAFE_DELETE_VECTOR(EventListeners);
	SAFE_DELETE_VECTOR(GenericEventListeners);
}
// ------------------------------------ //
void EventHandler::CallEvent(Event* pEvent){
	// Loop and call all listeners which have a valid type //
	for(size_t i = 0; i < EventListeners.size(); i++){

		if(EventListeners[i]->Type == pEvent->Type){

			EventListeners[i]->Receiver->OnEvent(&pEvent);
			if((pEvent) == NULL)
				// callable destroyed message //
				break;

		}
	}
	SAFE_DELETE(pEvent);
}

DLLEXPORT void Leviathan::EventHandler::CallEvent(GenericEvent* pEvent){
	// Loop generic listeners //
	for(size_t i = 0; i < GenericEventListeners.size(); i++){

		if(GenericEventListeners[i]->Type == *pEvent->TypeStr){

			GenericEventListeners[i]->Receiver->OnGenericEvent(&pEvent);
			if((pEvent) == NULL)
				// callable destroyed message //
				break;

		}
	}
	SAFE_DELETE(pEvent);
}
// ------------------------------------ //
bool EventHandler::RegisterForEvent(CallableObject* toregister, EVENT_TYPE totype){
	EventListeners.push_back(new RegisteredCallback(toregister, totype));
	return true;
}

DLLEXPORT bool Leviathan::EventHandler::RegisterForEvent(CallableObject* toregister, const wstring &genericname){
	GenericEventListeners.push_back(new GenericRegisteredCallback(toregister, genericname));
	return true;
}

void EventHandler::Unregister(CallableObject* caller, EVENT_TYPE type, bool all){
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