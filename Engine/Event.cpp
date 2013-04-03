#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EVENT
#include "Event.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Event::Event(){
	Type = EVENT_TYPE_ERROR;
}
Event::~Event(){
	SAFE_DELETE(Data);
}
Event::Event(EVENT_TYPE type, void* data){
	Type = type;
	Data = data;
}
// ------------------------------------ //
EVENT_TYPE Event::GetType(){
	return Type;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //