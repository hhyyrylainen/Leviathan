#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FILEREPLACENAME
#include "FILEREPLACENAME.h"
#endif
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::TrackEntityController::Init(){
	// Start listening //
	RegisterForEvent(EVENT_TYPE_FRAME_BEGIN);

	// Set current node and percentages and possibly update connected objects //
	_SanityCheckNodeProgress();
	// Cannot fail //
	return true;
}

DLLEXPORT void Leviathan::Entity::TrackEntityController::Release(){
	// Release the hooks //
	ReleaseChildHooks();

	// Nodes should not be hooked anymore //
	TrackNodes.clear();

	// Stop listening //
	UnRegister(EVENT_TYPE_FRAME_BEGIN, true);
}
// ------------------------------------ //
DLLEXPORT int Leviathan::Entity::TrackEntityController::OnEvent(Event** pEvent){
	// Update positions //
	if((*pEvent)->GetType() == EVENT_TYPE_FRAME_BEGIN){

		UpdateControlledPositions(*reinterpret_cast<int*>((*pEvent)->Data));
		return 1;
	}

	// This should signal disconnecting //
	return -1;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrackEntityController::UpdateControlledPositions(int mspassed){

}
// ------------------------------------ //
void Leviathan::Entity::TrackEntityController::_OnNotifiableDisconnected(BaseNotifiable* childtoremove){
	// Check does it match any nodes //
	for(auto iter = TrackNodes.begin(); iter != TrackNodes.end(); ++iter){

		if(&(*iter) == childtoremove){
			// Remove it //

			TrackNodes.erase(iter);
			_SanityCheckNodeProgress();
			return;
		}
	}
}
// ------------------------------------ //
void Leviathan::Entity::TrackEntityController::_SanityCheckNodeProgress(){

}

