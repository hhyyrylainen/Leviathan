// ------------------------------------ //
#include "EventHandler.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
EventHandler::EventHandler(){

}

EventHandler::~EventHandler(){

}
// ------------------------------------ //
bool EventHandler::Init(){
	return true;
}

void EventHandler::Release(){
	GUARD_LOCK();

    
    
	// Release listeners //
	EventListeners.clear();
	GenericEventListeners.clear();
}
// ------------------------------------ //
void EventHandler::CallEvent(Event* event){

    const auto type = event->GetType();

    GUARD_LOCK();

	for(auto iter = EventListeners.begin(); iter != EventListeners.end(); ){

		if(std::get<1>(*iter) == type){

			const auto result = std::get<0>(*iter)->OnEvent(event);

            if(result == -1){

                // Unregister requested //
                iter = EventListeners.erase(iter);
                continue;
            }
		}

        ++iter;
	}

    event->Release();
}

DLLEXPORT void Leviathan::EventHandler::CallEvent(GenericEvent* event){

    const auto type = event->GetType();
    
	GUARD_LOCK();
    
	// Loop generic listeners //
	for(auto iter = GenericEventListeners.begin(); iter != GenericEventListeners.end(); ){

		if(std::get<1>(*iter) == type){

			const auto result = std::get<0>(*iter)->OnGenericEvent(event);

            if(result == -1){

                // Unregister requested //
                iter = GenericEventListeners.erase(iter);
                continue;
            }
		}

        ++iter;
	}

    event->Release();
}
// ------------------------------------ //
bool EventHandler::RegisterForEvent(CallableObject* toregister, EVENT_TYPE totype){
	GUARD_LOCK();
	EventListeners.push_back(std::make_tuple(toregister, totype));
	return true;
}

DLLEXPORT bool Leviathan::EventHandler::RegisterForEvent(CallableObject* toregister,
    const std::string &genericname)
{
	GUARD_LOCK();
	GenericEventListeners.push_back(std::make_tuple(toregister, genericname));
	return true;
}

void EventHandler::Unregister(CallableObject* caller, EVENT_TYPE type, bool all){
	GUARD_LOCK();
    
	// Loop and remove wanted objects //
	for(auto iter = EventListeners.begin(); iter != EventListeners.end(); ){

		if(std::get<0>(*iter) == caller){
    
			// check type or if all is specified delete //
			if(all || type == std::get<1>(*iter)){
                
				iter = EventListeners.erase(iter);
                continue;
			}
		}

        ++iter;
	}
}

DLLEXPORT void Leviathan::EventHandler::Unregister(CallableObject* caller,
    const std::string &genericname,
    bool all /*= false*/)
{
	GUARD_LOCK();
    
	// Loop and remove wanted objects //
	for(auto iter = GenericEventListeners.begin(); iter != GenericEventListeners.end(); ){

		if(std::get<0>(*iter) == caller){
    
			// check type or if all is specified delete //
			if(all || genericname == std::get<1>(*iter)){
                
				iter = GenericEventListeners.erase(iter);
                continue;
			}
		}

        ++iter;
	}
}
// ------------------------------------ //

