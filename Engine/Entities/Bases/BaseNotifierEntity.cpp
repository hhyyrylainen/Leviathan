#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASENOTIFIERENTITY
#include "BaseNotifierEntity.h"
#endif
#include "Common/BaseNotifiable.h"
#include "BaseNotifiableEntity.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseNotifierEntity::BaseNotifierEntity() : BaseNotifier(this){

}

DLLEXPORT Leviathan::BaseNotifierEntity::~BaseNotifierEntity(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseNotifierEntity::SendCustomMessageToChildren(int messagetype, void* data, bool callonall /*= true*/){
	// If the loop won't stop to first successful call this is needed to know if any calls succeeded //
	bool called = false;

	for(auto iter = ConnectedChildren.begin(); iter != ConnectedChildren.end(); ++iter){

		if((*iter)->GetActualPointerToNotifiableObject()->SendCustomMessage(messagetype, data)){
			called = true;
			if(!callonall)
				return true;
		}
	}

	return called;
}
// ------------------------------------ //




