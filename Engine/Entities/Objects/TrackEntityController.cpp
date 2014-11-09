// ------------------------------------ //
#ifndef LEVIATHAN_TRACKENTITYCONTROLLER
#include "TrackEntityController.h"
#endif
#include "../GameWorld.h"
#include "../Bases/BasePhysicsObject.h"
#include "Common/Misc.h"
#include "../Bases/BaseNotifiableEntity.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::TrackEntityController::TrackEntityController(GameWorld* world) :
    BaseObject(IDFactory::GetID(), world), ReachedNode(-1), NodeProgress(0.f), ChangeSpeed(0.f),
    ForceTowardsPoint(TRACKCONTROLLER_DEFAULT_APPLYFORCE), RequiresUpdate(true),
    BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER)
{

}

Leviathan::Entity::TrackEntityController::TrackEntityController(int netid, GameWorld* world) :
    BaseObject(netid, world), ReachedNode(-1), NodeProgress(0.f), ChangeSpeed(0.f),
    ForceTowardsPoint(TRACKCONTROLLER_DEFAULT_APPLYFORCE), RequiresUpdate(true),
    BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER)
{


}

DLLEXPORT Leviathan::Entity::TrackEntityController::~TrackEntityController(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::TrackEntityController::Init(){
	// Start listening //
	RegisterForEvent(EVENT_TYPE_PHYSICS_BEGIN);

	// Set current node and percentages and possibly update connected objects //
	_SanityCheckNodeProgress();
    
	// Cannot fail //
	return true;
}

DLLEXPORT void Leviathan::Entity::TrackEntityController::ReleaseData(){
    GUARD_LOCK_THIS_OBJECT();
    
	// Release the hooks //
	ReleaseChildHooks();

	// Nodes should not be hooked anymore //
	TrackNodes.clear();

	// Stop listening //
	UnRegister(EVENT_TYPE_PHYSICS_BEGIN, true);
}
// ------------------------------------ //
DLLEXPORT int Leviathan::Entity::TrackEntityController::OnEvent(Event** pEvent){
	// Update positions //
	if((*pEvent)->GetType() == EVENT_TYPE_PHYSICS_BEGIN){

		// Get data //
		PhysicsStartEventData* dataptr = (*pEvent)->GetDataForPhysicsStartEvent();
		assert(dataptr && "Invalid physics event");
		// Skip if wrong world //
		if(dataptr->GameWorldPtr != static_cast<void*>(OwnedByWorld)){
			return 0;
		}
        
		// This object's parent world is being updated //
		UpdateControlledPositions(dataptr->TimeStep);
		return 1;
	}

	// This should signal disconnecting //
	return -1;
}

DLLEXPORT int Leviathan::Entity::TrackEntityController::OnGenericEvent(GenericEvent** pevent){
	return -1;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrackEntityController::UpdateControlledPositions(float timestep){
    GUARD_LOCK_THIS_OBJECT();
	// Update progress and node number //
	if(ChangeSpeed != 0.f){
		// Update position //

		NodeProgress += ChangeSpeed*timestep;

		// Check did node change //
		if(NodeProgress > 1.f){
			// Next node //
			if(ReachedNode+1 >= (int)TrackNodes.size()){
				ReachedNode++;
				NodeProgress = 0.f;
			} else {
				NodeProgress = 1.f;
			}
		} else if(NodeProgress < 0.f){
			// Previous node, if possible //
			if(ReachedNode > 0){
				ReachedNode--;
				NodeProgress = 1.f;
			} else {
				NodeProgress = 0.f;
			}
		}
	}

	// Send the positions to the objects //
	_ApplyTrackPositioning(timestep);
}
// ------------------------------------ //
void Leviathan::Entity::TrackEntityController::_OnNotifiableDisconnected(BaseNotifiableEntity* childtoremove){
	// Cast to right type //
	LocationNode* tmpptr = static_cast<LocationNode*>(childtoremove);

	// Check does it match any nodes //
	for(auto iter = TrackNodes.begin(); iter != TrackNodes.end(); ++iter){

		if(*iter == tmpptr){
			// Remove it //

			TrackNodes.erase(iter);
			_SanityCheckNodeProgress();
			return;
		}
	}

	// It is a controlled object //
	wstring defaulttrackname = L"";

	// Remove the force //
	childtoremove->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_REMOVEAPPLYFORCE, &defaulttrackname);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrackEntityController::SetProgressTowardsNextNode(float progress){
    GUARD_LOCK_THIS_OBJECT();
    
	NodeProgress = progress;
	// Update now //
	_SanityCheckNodeProgress();
	RequiresUpdate = true;
}

void Leviathan::Entity::TrackEntityController::_SanityCheckNodeProgress(){
	if(ReachedNode < 0 || ReachedNode >= (int)TrackNodes.size()){
		ReachedNode = 0;
	}
	if(NodeProgress < 0.f || NodeProgress > 1.f || !Misc::IsFiniteNumber(NodeProgress)){
		NodeProgress = 0.f;
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::TrackEntityController::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	// First check if it is a request //
	if(entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_DATAREQUEST){
		// Check through components //
		ObjectDataRequest* tmprequest = reinterpret_cast<ObjectDataRequest*>(dataptr);

		

		return false;
	}

	// Check through components //
	


	// This specific //


	// Check if a node has been updated //
	if(entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_LOCATIONDATA_UPDATED || entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_POSITIONUPDATED ||
		entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_ORIENTATIONUPDATED)
	{
        GUARD_LOCK_THIS_OBJECT();
		RequiresUpdate = true;
		return true;
	}

	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrackEntityController::AddLocationToTrack(const Float3 &pos, const Float4 &dir){
	// Create the node //
	unique_ptr<LocationNode> tmpnode(new LocationNode(OwnedByWorld, pos, dir, true));

	// Add to world //
	OwnedByWorld->AddObject(shared_ptr<BaseObject>(tmpnode.get()));

    GUARD_LOCK_THIS_OBJECT();
    
	// Link //
	ConnectToNotifiable(tmpnode.get());

	// Add to nodes //
	TrackNodes.push_back(tmpnode.release());
}
// ------------------------------------ //
void Leviathan::Entity::TrackEntityController::_ApplyTrackPositioning(float timestep){
	// Calculate the position //
	Float3 TrackPos(0.f);
	Float4 TrackDir(Float4::IdentityQuaternion());

	// Interpolate the values //
	if(TrackNodes.size() == 1 || ReachedNode+1 >= (int)TrackNodes.size() || NodeProgress == 0.f){
		// The reached node is completely in control of the position //
		TrackPos = TrackNodes[ReachedNode]->GetPos();
		TrackDir = TrackNodes[ReachedNode]->GetOrientation();
	} else {
		// We need interpolation //
		TrackPos = TrackNodes[ReachedNode]->GetPos()*(1.f-NodeProgress)+TrackNodes[ReachedNode+1]->GetPos()*
            NodeProgress;
		TrackDir = TrackNodes[ReachedNode]->GetOrientation().Slerp(TrackNodes[ReachedNode+1]->GetOrientation(),
            NodeProgress);
	}

	// Apply forces to all objects //
	for(auto iter = ConnectedChildren.begin(); iter != ConnectedChildren.end(); ++iter){
		// Skip this child if it is a node //
		bool skip = false;

		// Cast to right type //
		LocationNode* tmpptr = static_cast<LocationNode*>(*iter);

		for(auto iternodes = TrackNodes.begin(); iternodes != TrackNodes.end(); ++iternodes){

			if(*iternodes == tmpptr){
				skip = true;
				break;
			}
		}

		if(!skip){

			// Request position //
			ObjectDataRequest request(ENTITYDATA_REQUESTTYPE_WORLDPOSITION);

			(*iter)->GetActualPointerToNotifiableObject()->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_DATAREQUEST, &request);

			// If non positionable skip //
			if(request.RequestResult == NULL){

				continue;
			}

			// \todo implement different types //
			if(false){

				// Apply force //
				Float3 forcetoapply = TrackPos-*reinterpret_cast<Float3*>(request.RequestResult);

				unique_ptr<ApplyForceInfo> tmpapply(new ApplyForceInfo(forcetoapply, true, true));

				(*iter)->GetActualPointerToNotifiableObject()->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_ADDAPPLYFORCE, tmpapply.get());
			} else if(true){
				// Add velocity method //
				Float3 wantedspeed = TrackPos-*reinterpret_cast<Float3*>(request.RequestResult);


				wantedspeed = wantedspeed*ForceTowardsPoint;

				(*iter)->GetActualPointerToNotifiableObject()->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_SETVELOCITY, &wantedspeed);
			} else {
				// Set position method //
				(*iter)->GetActualPointerToNotifiableObject()->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_CHANGEWORLDPOSITION, &TrackPos);
			}
		}

	}

}
// ------------------------------------ //
DLLEXPORT Float3 Leviathan::Entity::TrackEntityController::GetCurrentNodePosition(){
    GUARD_LOCK_THIS_OBJECT();
	if(TrackNodes.size() < 1)
		return Float3(0);

	// Get the current reached nodes' position //
	return TrackNodes[ReachedNode]->GetPos();
}

DLLEXPORT Float3 Leviathan::Entity::TrackEntityController::GetNextNodePosition(){
    GUARD_LOCK_THIS_OBJECT();
	// Check if there is a next node //
	if((size_t)(ReachedNode+1) >= TrackNodes.size())
		return Float3(0);
	// Return the position of the node //
	return TrackNodes[ReachedNode+1]->GetPos();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrackEntityController::AddUpdateToPacket(sf::Packet &packet, ConnectionInfo*
    receiver)
{
    DEBUG_BREAK;
}

DLLEXPORT bool Leviathan::Entity::TrackEntityController::LoadUpdateFromPacket(sf::Packet &packet){

    DEBUG_BREAK;
}
// ------------------------------------ //
bool Leviathan::Entity::TrackEntityController::_LoadOwnDataFromPacket(sf::Packet &packet){
    int reachednode;
    float nodeprogress, changespeed, force;
    int32_t nodecount;

    packet >> reachednode >> nodeprogress >> changespeed >> force;
    packet >> nodecount;

    if(!packet){

        Logger::Get()->Error("TrackEntityController: packet has invalid format");
        return false;
    }

    for(int32_t i = 0; i < nodecount; i++){

        // Create a new node to hold the position //
        unique_ptr<LocationNode> curnode(new LocationNode(OwnedByWorld));

        if(!curnode){

            Logger::Get()->Error("TrackEntityController: load data from packet: failed to allocate LocationNode");
            return false;
        }

        // Apply position //
        curnode->ApplyPositionAndRotationFromPacket(packet);
        
        // Link //
        ConnectToNotifiable(curnode.get());
        
        TrackNodes.push_back(curnode.release());
    }

    if(!packet){

        Logger::Get()->Error("TrackEntityController: packet has invalid format");
        return false;
    }

    Init();

    return true;
}

void Leviathan::Entity::TrackEntityController::_SaveOwnDataToPacket(sf::Packet &packet){
    GUARD_LOCK_THIS_OBJECT();

    // First dump our state //
    packet << ReachedNode << NodeProgress << ChangeSpeed << ForceTowardsPoint;

    // Then put all of our location nodes //
    int32_t nodecount = TrackNodes.size();
    packet << nodecount;
    
    for(int32_t i = 0; i < nodecount; i++){

        TrackNodes[i]->AddPositionAndRotationToPacket(packet);
    }
    
}

