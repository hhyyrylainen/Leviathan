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

void Leviathan::CallableObject::RegisterForEvent(EVENT_TYPE toregister){
	EventHandler::Get()->RegisterForEvent(this, toregister);
}

// ------------------------------------ //

