// ------------------------------------ //
#ifndef LEVIATHAN_TRACKENTITYCONTROLLER
#include "TrackEntityController.h"
#endif
#include "Entities/GameWorld.h"
#include "Entities/Bases/BasePhysicsObject.h"
#include "Common/Misc.h"
#include "Entities/Bases/BaseNotifiableEntity.h"
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
    
	return true;
}

DLLEXPORT void Leviathan::Entity::TrackEntityController::ReleaseData(){

    // Stop listening //
	UnRegister(EVENT_TYPE_PHYSICS_BEGIN, true);
    
    GUARD_LOCK_THIS_OBJECT();

    AggressiveConstraintUnlink();
    
	TrackNodes.clear();
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
	if(ChangeSpeed != 0.f || RequiresUpdate){
		// Update position //
        
		NodeProgress += ChangeSpeed*timestep;
        
        _MarkDataUpdated();
        RequiresUpdate = false;

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
void Leviathan::Entity::TrackEntityController::_OnConstraintUnlink(BaseConstraint* ptr){

    auto childtoremove = ptr->GetSecondEntity();

    if(!childtoremove || ptr->GetType() != ENTITY_CONSTRAINT_TYPE_CONTROLLERCONSTRAINT)
        return;
    
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
	if(entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_LOCATIONDATA_UPDATED ||
        entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_POSITIONUPDATED ||
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
	shared_ptr<LocationNode> tmpnode(new LocationNode(OwnedByWorld, pos, dir, true));

    // Because we don't add it to the world we need to make sure to delete them afterwards //
    GUARD_LOCK_THIS_OBJECT();

	// Add to nodes //
	TrackNodes.push_back(tmpnode);
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
    auto end = PartInConstraints.end();
	for(auto iter = PartInConstraints.begin(); iter != end; ++iter){

        BaseConstraintable* obj = (*iter)->ParentPtr->GetSecondEntity();
        
        // Request position //
        ObjectDataRequest request(ENTITYDATA_REQUESTTYPE_WORLDPOSITION);

        if(obj)
            obj->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_DATAREQUEST, &request);

        // If non positionable skip //
        if(request.RequestResult == NULL){

            continue;
        }

        // \todo implement different types //
        if(false){

            // Apply force //
            Float3 forcetoapply = TrackPos-*reinterpret_cast<Float3*>(request.RequestResult);

            unique_ptr<ApplyForceInfo> tmpapply(new ApplyForceInfo(forcetoapply, true, true));

            obj->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_ADDAPPLYFORCE, tmpapply.get());
            
        } else if(true){
            
            // Add velocity method //
            Float3 wantedspeed = TrackPos-*reinterpret_cast<Float3*>(request.RequestResult);


            wantedspeed = wantedspeed*ForceTowardsPoint;

            obj->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_SETVELOCITY, &wantedspeed);
            
        } else {
            // Set position method //
            obj->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_CHANGEWORLDPOSITION, &TrackPos);
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
DLLEXPORT void Leviathan::Entity::TrackEntityController::SetTrackAdvanceSpeed(const float &speed){
    ChangeSpeed = speed;
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
        shared_ptr<LocationNode> curnode(new LocationNode(OwnedByWorld));

        if(!curnode){

            Logger::Get()->Error("TrackEntityController: load data from packet: failed to allocate LocationNode");
            return false;
        }

        // Apply position //
        curnode->ApplyPositionAndRotationFromPacket(packet);

        TrackNodes.push_back(curnode);
    }

    if(!packet){

        Logger::Get()->Error("TrackEntityController: packet has invalid format");
        return false;
    }

    // Set the things //
    // These can be set before Init as it doesn't set any default values
    ReachedNode = reachednode;
    NodeProgress = nodeprogress;
    ChangeSpeed = changespeed;
    ForceTowardsPoint = force;
    
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
// ------------------------------------ //
DLLEXPORT shared_ptr<ObjectDeltaStateData> Leviathan::Entity::TrackEntityController::CaptureState(){
    
    return shared_ptr<ObjectDeltaStateData>(
        new TrackControllerState(ReachedNode, ChangeSpeed, NodeProgress));
}

DLLEXPORT void Leviathan::Entity::TrackEntityController::VerifyOldState(ObjectDeltaStateData* serversold,
    ObjectDeltaStateData* ourold, int tick)
{
    // Check first do we need to resimulate //
    bool requireupdate = false;

    TrackControllerState* servercasted = static_cast<TrackControllerState*>(serversold);
    TrackControllerState* ourcasted = static_cast<TrackControllerState*>(ourold);
    
    if(!ourold){

        requireupdate = true;
        
    } else {
        
        // We are comparing floats here which most of the time will result in all comparisons being false... //
        if(ourcasted->ChangeSpeed != servercasted->ChangeSpeed ||
            fabs(ourcasted->NodeProgress-servercasted->NodeProgress) >= TRACKCONTROLLER_PROGRESS_THRESSHOLD ||
            ourcasted->ReachedNode != servercasted->ReachedNode)
        {
            requireupdate = true;
        }
    }

    // All good if our old state matched //
    if(!requireupdate)
        return;

    // Go back to the verified state and resimulate //
    ChangeSpeed = servercasted->ChangeSpeed;
    NodeProgress = servercasted->NodeProgress;
    ReachedNode = servercasted->ReachedNode;

    _SanityCheckNodeProgress();
    

    Logger::Get()->Info("TrackEntiyController: resimulating");
    
    // 1000 milliseconds in a second //
    UpdateControlledPositions((float)(tick*TICKSPEED)/1000);
}

DLLEXPORT shared_ptr<ObjectDeltaStateData> Leviathan::Entity::TrackEntityController::CreateStateFromPacket(
    sf::Packet &packet, shared_ptr<ObjectDeltaStateData> fillblanks) const
{
    try{
        
        return make_shared<TrackControllerState>(packet, fillblanks);
        
    } catch(ExceptionInvalidArgument &e){

        Logger::Get()->Warning("TrackEntityController: failed to CreateStateFromPacket, exception:");
        e.PrintToLog();
        return nullptr;
    }
    
}
// ------------------ TrackControllerState ------------------ //
DLLEXPORT Leviathan::Entity::TrackControllerState::TrackControllerState(int reached, float speed, float progress) :
    ReachedNode(reached), ChangeSpeed(speed), NodeProgress(progress), AddedNodes(0)
{

}

DLLEXPORT Leviathan::Entity::TrackControllerState::TrackControllerState(sf::Packet &packet,
    shared_ptr<ObjectDeltaStateData> fillblanks)
{

    int8_t packetstates = 0;

    // We need to do the opposite of what we do in CreateUpdatePacket //
    packet >> packetstates;

    if(!packet)
        throw ExceptionInvalidArgument(L"invalid packet for TrackControllerState", 0, __WFUNCTION__,
            L"packet", L"");
    
    // Warn if we can't fill in the blanks //
    if(!fillblanks){

        // These can be detected and ignored //
        NodeProgress = -1.f;            
        ReachedNode = -1;

        // But this cannot //
        if(packetstates != TRACKSTATE_UPDATED_SPEED){

            // TODO: add a mechanism for automatically reporting engine bugs that shouldn't happen //
            Logger::Get()->Error("TrackControllerState: trying to reconstruct packet update without older "
                "state (that has speed), PLEASE REPORT THIS ERROR");

            ChangeSpeed = 0.f;
        }
        
    } else {

        // Take starting values from the fillblanks one //
        (*this) = *static_cast<TrackControllerState*>(fillblanks.get());
    }

    if(packetstates & TRACKSTATE_UPDATED_NODE)
        packet >> ReachedNode;

    if(packetstates & TRACKSTATE_UPDATED_SPEED)
        packet >> ChangeSpeed;
    
    if(packetstates & TRACKSTATE_UPDATED_PROGRESS)
        packet >> NodeProgress;
}
            
DLLEXPORT void Leviathan::Entity::TrackControllerState::CreateUpdatePacket(ObjectDeltaStateData* olderstate,
    sf::Packet &packet)
{

    int8_t changedparts = 0;

    // Check which parts have changed //
    if(!olderstate){

        // When comparing against NULL state everything is updated //
        changedparts = TRACKSTATE_UPDATED_ALL;
        
    } else {

        TrackControllerState* other = static_cast<TrackControllerState*>(olderstate);

        // Node
        if(ReachedNode != other->ReachedNode)
            changedparts |= TRACKSTATE_UPDATED_NODE;

        // Speed
        if(ChangeSpeed != other->ChangeSpeed)
            changedparts |= TRACKSTATE_UPDATED_SPEED;

        // Progress
        if(NodeProgress != other->NodeProgress)
            changedparts |= TRACKSTATE_UPDATED_PROGRESS;
    }

    packet << changedparts;

    // Add the changed data to the packet //
    if(changedparts & TRACKSTATE_UPDATED_NODE)
        packet << ReachedNode;

    if(changedparts & TRACKSTATE_UPDATED_SPEED)
        packet << ChangeSpeed;
    
    if(changedparts & TRACKSTATE_UPDATED_PROGRESS)
        packet << NodeProgress;
}










