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

EventHandler* EventHandler::main = NULL;
EventHandler* EventHandler::Get(){
	return main;
}

// ------------------------------------ //
bool EventHandler::Init(){
	return true;
}
void EventHandler::Release(){

}
// ------------------------------------ //
void EventHandler::CallEvent(Event* pEvent){
	for(unsigned int i = 0; i < EventListeners.size(); i++){
		if(EventListeners[i]->Type == pEvent->Type){
			EventListeners[i]->Receiver->OnEvent(&pEvent);
			if((pEvent) == NULL) // callable destroyed message 
				break;

		}
	}
	delete pEvent;
}
// ------------------------------------ //
bool EventHandler::RegisterForEvent(CallableObject* toregister, EVENT_TYPE totype){
	EventListeners.push_back(new RegisteredCallback(toregister, totype));
	return true;
}
void EventHandler::Unregister(CallableObject* caller, EVENT_TYPE type, bool all){
	for(unsigned int i = 0; i < EventListeners.size(); i++){
		if(EventListeners[i]->Receiver == caller){
			if(all || type == EventListeners[i]->Type){
				delete EventListeners[i];
				EventListeners.erase(EventListeners.begin()+i);
				i--;
			}
		}
	}
}
// ------------------------------------ //