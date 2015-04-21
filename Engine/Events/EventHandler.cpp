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
	EventListeners.clear();
	SAFE_DELETE_VECTOR(GenericEventListeners);
}
// ------------------------------------ //
void EventHandler::CallEvent(Event* pEvent){
	GUARD_LOCK_THIS_OBJECT();

    const auto type = pEvent->GetType();

	for(auto iter = EventListeners.begin(); iter != EventListeners.end(); ){

		if((*iter)->Type == type){

			const auto result = (*iter)->Receiver->OnEvent(&pEvent);

            if(result == -1){

                // Unregister requested //
                iter = EventListeners.erase(iter);
                continue;
            }
		}

        ++iter;
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
	EventListeners.push_back(move(make_unique<RegisteredCallback>(toregister, totype)));
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

				EventListeners.erase(EventListeners.begin()+i);
				i--;
			}
		}
	}
}

DLLEXPORT void Leviathan::EventHandler::Unregister(CallableObject* caller, const wstring &genericname,
    bool all /*= false*/)
{
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
