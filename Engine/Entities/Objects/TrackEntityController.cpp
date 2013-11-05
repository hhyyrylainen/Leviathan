#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TRACKENTITYCONTROLLER
#include "TrackEntityController.h"
#endif
#include "..\GameWorld.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::TrackEntityController::TrackEntityController(GameWorld* world) : BaseObject(IDFactory::GetID(), world){

}

DLLEXPORT Leviathan::Entity::TrackEntityController::~TrackEntityController(){

}
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

DLLEXPORT int Leviathan::Entity::TrackEntityController::OnGenericEvent(GenericEvent** pevent){
	return -1;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrackEntityController::UpdateControlledPositions(int mspassed){

}
// ------------------------------------ //
void Leviathan::Entity::TrackEntityController::_OnNotifiableDisconnected(BaseNotifiable* childtoremove){
	// Check does it match any nodes //
	for(auto iter = TrackNodes.begin(); iter != TrackNodes.end(); ++iter){

		if(static_cast<BaseNotifiable*>(*iter) == childtoremove){
			// Remove it //

			TrackNodes.erase(iter);
			_SanityCheckNodeProgress();
			return;
		}
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrackEntityController::SetProgressTowardsNextNode(float progress){

}

void Leviathan::Entity::TrackEntityController::_SanityCheckNodeProgress(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::TrackEntityController::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	DEBUG_BREAK;
	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrackEntityController::AddLocationToTrack(const Float3 &pos, const Float4 &dir){
	// Create the node //
	unique_ptr<LocationNode> tmpnode(new LocationNode(LinkedToWorld, pos, dir, true));

	// Add to world //
	LinkedToWorld->AddObject(shared_ptr<BaseObject>(tmpnode.get()));

	// Link //
	ConnectToNotifiable(tmpnode.get());

	// Add to nodes //
	TrackNodes.push_back(tmpnode.release());
}
// ------------------------------------ //


