#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EVENT
#include "Event.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Event::Event(){
	Type = EVENT_TYPE_ERROR;
	Data = NULL;
	DeleteIt = true;
}
Event::~Event(){
	// only delete the pointed data if it is the wanted action //
	if(DeleteIt)
		SAFE_DELETE(Data);
}
Event::Event(EVENT_TYPE type, void* data){
	Type = type;
	Data = data;
	DeleteIt = true;
}

DLLEXPORT Leviathan::Event::Event(EVENT_TYPE type, void* data, bool allowdelete){
	Type = type;
	Data = data;
	// user controlled deletion //
	DeleteIt = allowdelete;
}

// ------------------------------------ //
EVENT_TYPE Event::GetType(){
	return Type;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //