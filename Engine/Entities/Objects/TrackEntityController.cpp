// ------------------------------------ //
#ifndef LEVIATHAN_TRACKENTITYCONTROLLER
#include "TrackEntityController.h"
#endif
#include "Entities/GameWorld.h"
#include "Entities/Bases/BasePhysicsObject.h"
#include "Common/Misc.h"
#include "Entities/Bases/BaseNotifiableEntity.h"
#include "Newton/PhysicalWorld.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::TrackEntityController::TrackEntityController(GameWorld* world) :
    BaseObject(IDFactory::GetID(), world), ReachedNode(-1), NodeProgress(0.f), ChangeSpeed(0.f),
    ForceTowardsPoint(TRACKCONTROLLER_DEFAULT_APPLYFORCE), RequiresUpdate(true),
    BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER), LastResimulateTarget(NULL)
{

}

Leviathan::Entity::TrackEntityController::TrackEntityController(int netid, GameWorld* world) :
    BaseObject(netid, world), ReachedNode(-1), NodeProgress(0.f), ChangeSpeed(0.f),
    ForceTowardsPoint(TRACKCONTROLLER_DEFAULT_APPLYFORCE), RequiresUpdate(true),
    BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER), LastResimulateTarget(NULL)
{


}

DLLEXPORT Leviathan::Entity::TrackEntityController::~TrackEntityController(){

    ReleaseData();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::TrackEntityController::Init(){
	// Start listening //
	RegisterForEvent(EVENT_TYPE_PHYSICS_BEGIN);
    RegisterForEvent(EVENT_TYPE_PHYSICS_RESIMULATE_SINGLE);

	// Set current node and percentages and possibly update connected objects //
	_SanityCheckNodeProgress();
    
	return true;
}

DLLEXPORT void Leviathan::Entity::TrackEntityController::ReleaseData(){

    // Stop listening //
    // This should unregister both listeners
	UnRegister(EVENT_TYPE_PHYSICS_BEGIN, true);

    {
        UNIQUE_LOCK_THIS_OBJECT();

        AggressiveConstraintUnlink(lockit);
    }
    
    GUARD_LOCK_THIS_OBJECT();

    LastResimulateTarget = NULL;

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
        
	} else if((*pEvent)->GetType() == EVENT_TYPE_PHYSICS_RESIMULATE_SINGLE){

		// Get data //
		ResimulateSingleEventData* dataptr = (*pEvent)->GetDataForResimulateSingleEvent();
        
		assert(dataptr && "Invalid physics event");
        
		// Skip if wrong world //
		if(dataptr->GameWorldPtr != static_cast<void*>(OwnedByWorld)){
            
			return 0;
		}

        GUARD_LOCK_THIS_OBJECT();
        
		// Check whether it is our entity //
        if(dataptr->Target == LastResimulateTarget){

            // It should be ours //
            if(!_ApplyResimulateForce(dataptr->TimeInPast, LastResimulateTarget, guard)){

                // LastResimulateTarget is no longer valid //
                LastResimulateTarget = NULL;
            }
            
            return 1;
        }

        auto end = PartInConstraints.end();
        for(auto iter = PartInConstraints.begin(); iter != end; ++iter){

            auto parentpart = (*iter)->ParentPtr;
            BaseConstraintable* obj = parentpart ? parentpart->GetSecondEntity():
                (*iter)->ChildPartPtr.lock()->GetFirstEntity();

            if(obj == dataptr->Target){

                LastResimulateTarget = obj;
                _ApplyResimulateForce(dataptr->TimeInPast, LastResimulateTarget, guard);
                
                return 1;
            }
        }
            
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
        
        _MarkDataUpdated(guard);
        RequiresUpdate = false;

		// Check did node change //
		while(NodeProgress > 1.f){
            
			// Next node //
			if(ReachedNode+1 >= (int)TrackNodes.size()){
                
				ReachedNode++;
				NodeProgress -= 1.f;
                
			} else {
                
				NodeProgress = 1.f;
			}
            
		}
        
        while(NodeProgress < 0.f){
            
			// Previous node, if possible //
			if(ReachedNode > 0){
                
				ReachedNode--;
				NodeProgress += 1.f;
                
			} else {
                
				NodeProgress = 0.f;
			}
		}
	}

	// Send the positions to the objects //
	_ApplyTrackPositioning(timestep, guard);
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
void Leviathan::Entity::TrackEntityController::_ApplyTrackPositioning(float timestep, ObjectLock &guard){

    Float3 TrackPos;
	Float4 TrackDir;
    
    _GetPosAndRotForProgress(TrackPos, TrackDir, NodeProgress, ReachedNode);

	// Apply forces to all objects //
    auto end = PartInConstraints.end();
	for(auto iter = PartInConstraints.begin(); iter != end; ++iter){

        auto parentpart = (*iter)->ParentPtr;
        BaseConstraintable* obj = parentpart ? parentpart->GetSecondEntity():
            (*iter)->ChildPartPtr.lock()->GetFirstEntity();

        
        _ApplyPositioningToSingleEntity(TrackPos, TrackDir, obj);
    }
}

void Leviathan::Entity::TrackEntityController::_ApplyPositioningToSingleEntity(const Float3 &pos, const Float4 &rot,
    BaseConstraintable* obj) const
{

    // Request position //
    ObjectDataRequest request(ENTITYDATA_REQUESTTYPE_WORLDPOSITION);

    if(obj)
        obj->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_DATAREQUEST, &request);

    // If non positionable skip //
    if(request.RequestResult == NULL){

        return;
    }

    if(true){
            
        // Add velocity method //
        Float3 wantedspeed = pos-*reinterpret_cast<Float3*>(request.RequestResult);


        wantedspeed = wantedspeed*ForceTowardsPoint;

        obj->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_SETVELOCITY, &wantedspeed);
            
    } else {
        // Set position method //
        Float3 tmpval(pos);
        
        obj->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_CHANGEWORLDPOSITION, &tmpval);
    }

    // Rotation applying //


    if(true){

        Float4 currotation;

        Float4 quaterniontorque = rot.QuaternionMultiply(currotation.QuaternionReverse());

        // Extract angles along all axises //
        Float3 turnarounddirections;

        // Set it as the torque //
        
    } else {

        // Just set it as the rotation //
        
    }
}
// ------------------------------------ //
bool Leviathan::Entity::TrackEntityController::_ApplyResimulateForce(int64_t microsecondsinpast, BaseConstraintable*
    singleentity, ObjectLock &guard)
{

    // Lets go back in time and see were we are at //
    float fakeprogress = NodeProgress - ChangeSpeed*(microsecondsinpast/1000000.f);

    int fakenode = ReachedNode;
    
    // Check did node change //
    while(fakeprogress > 1.f){
            
        // Next node //
        if(fakenode+1 >= (int)TrackNodes.size()){
            fakenode++;
            fakeprogress -= 1.f;
            
        } else {
            
            fakeprogress = 1.f;
        }
    }
    
    while(fakeprogress < 0.f){
            
        // Previous node, if possible //
        if(fakenode > 0){
            fakenode--;
            fakeprogress += 1.f;
        } else {
            
            fakeprogress = 0.f;
        }
    }

    bool wasapplied = false;

    Float3 pos;
    Float4 rot;
    
    _GetPosAndRotForProgress(pos, rot, fakeprogress, fakenode);

    // Selectively apply the positioning //
    auto end = PartInConstraints.end();
	for(auto iter = PartInConstraints.begin(); iter != end; ++iter){

        auto parentpart = (*iter)->ParentPtr;
        BaseConstraintable* obj = parentpart ? parentpart->GetSecondEntity():
            (*iter)->ChildPartPtr.lock()->GetFirstEntity();

        if(singleentity != NULL && singleentity != obj)
            continue;
        
        _ApplyPositioningToSingleEntity(pos, rot, obj);
        wasapplied = true;
    }

    return wasapplied;
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
void Leviathan::Entity::TrackEntityController::_GetPosAndRotForProgress(Float3 &pos, Float4 &rot, float progress,
    int reached) const
{
	// Interpolate the values //
	if(TrackNodes.size() == 1 || reached+1 >= (int)TrackNodes.size() || progress == 0.f){
        
		// The reached node is completely in control of the position //
		pos = TrackNodes[reached]->GetPos();
		rot = TrackNodes[reached]->GetOrientation();
        
	} else {
        
		// We need interpolation //
		pos = TrackNodes[reached]->GetPos()*(1.f-progress)+TrackNodes[reached+1]->GetPos()*
            progress;
		rot = TrackNodes[reached]->GetOrientation().Slerp(TrackNodes[reached+1]->GetOrientation(),
            progress);
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
        if(((servercasted->ValidFields & TRACKSTATE_UPDATED_SPEED) &&
                ourcasted->ChangeSpeed != servercasted->ChangeSpeed) ||
            ((servercasted->ValidFields & TRACKSTATE_UPDATED_PROGRESS) &&
                fabs(ourcasted->NodeProgress-servercasted->NodeProgress) >= TRACKCONTROLLER_PROGRESS_THRESSHOLD) ||
            ((servercasted->ValidFields & TRACKSTATE_UPDATED_NODE) &&
                ourcasted->ReachedNode != servercasted->ReachedNode))
        {
            requireupdate = true;
        }
    }

    // All good if our old state matched //
    if(!requireupdate)
        return;

    GUARD_LOCK_THIS_OBJECT();

    // Go back to the verified state and resimulate //
    if(servercasted->ValidFields & TRACKSTATE_UPDATED_SPEED)
        ChangeSpeed = servercasted->ChangeSpeed;

    if(servercasted->ValidFields & TRACKSTATE_UPDATED_PROGRESS)
        NodeProgress = servercasted->NodeProgress;

    if(servercasted->ValidFields & TRACKSTATE_UPDATED_NODE)
        ReachedNode = servercasted->ReachedNode;

    _SanityCheckNodeProgress();
    
    
    const int worldtick = OwnedByWorld->GetTickNumber();
    
    // Convert from milliseconds to microseconds //
    int timetosimulate = (worldtick-tick)*TICKSPEED*1000.f;
    
    // This should hold on to the world update lock once that is required //
    auto nworld = OwnedByWorld->GetPhysicalWorld()->GetNewtonWorld();

    // We need to refill all the states that were recorded during that time //
    int advancedtick = tick;
    ReplaceOldClientState(advancedtick, CaptureState());

    int simulatedtime = 0;
    
    // And then simulate updates for that time //
	while(timetosimulate >= NEWTON_FPS_IN_MICROSECONDS){
        
        UpdateControlledPositions(NEWTON_TIMESTEP);
        timetosimulate -= NEWTON_FPS_IN_MICROSECONDS;

        // Keep track of current tick while resimulating //
        simulatedtime += NEWTON_FPS_IN_MICROSECONDS;

        if(simulatedtime >= TICKSPEED*1000){

            advancedtick++;
            simulatedtime -= TICKSPEED*1000;

            assert(advancedtick <= worldtick && "TrackEntityController resimulate assert");
            
            if(!ReplaceOldClientState(advancedtick, CaptureState())){

                // The world tick might not have been stored yet... //
                if(advancedtick != worldtick){

                    Logger::Get()->Warning("TrackEntityController("+Convert::ToString(ID)+"): resimulate: didn't find "
                        "old state for tick "+Convert::ToString(advancedtick));
                }
            }
        }
    }
    
#ifdef ALLOW_RESIMULATE_CONSUME_ALL

    if(timetosimulate > 0){

        UpdateControlledPositions(timetosimulate/1000000.f);
    }
    
#endif //ALLOW_RESIMULATE_CONSUME_ALL
}

DLLEXPORT shared_ptr<ObjectDeltaStateData> Leviathan::Entity::TrackEntityController::CreateStateFromPacket(
    sf::Packet &packet) const
{
    try{
        
        return make_shared<TrackControllerState>(packet);
        
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

DLLEXPORT Leviathan::Entity::TrackControllerState::TrackControllerState(sf::Packet &packet){

    // We need to do the opposite of what we do in CreateUpdatePacket //
    packet >> ValidFields;

    if(!packet)
        throw ExceptionInvalidArgument(L"invalid packet for TrackControllerState", 0, __WFUNCTION__,
            L"packet", L"");
    
    if(ValidFields & TRACKSTATE_UPDATED_NODE)
        packet >> ReachedNode;

    if(ValidFields & TRACKSTATE_UPDATED_SPEED)
        packet >> ChangeSpeed;
    
    if(ValidFields & TRACKSTATE_UPDATED_PROGRESS)
        packet >> NodeProgress;
}
            
DLLEXPORT void Leviathan::Entity::TrackControllerState::CreateUpdatePacket(ObjectDeltaStateData* olderstate,
    sf::Packet &packet)
{

    ValidFields = 0;

    // Check which parts have changed //
    if(!olderstate){

        // When comparing against NULL state everything is updated //
        ValidFields = TRACKSTATE_UPDATED_ALL;
        
    } else {

        TrackControllerState* other = static_cast<TrackControllerState*>(olderstate);

        // Node
        if(ReachedNode != other->ReachedNode)
            ValidFields |= TRACKSTATE_UPDATED_NODE;

        // Speed
        if(ChangeSpeed != other->ChangeSpeed)
            ValidFields |= TRACKSTATE_UPDATED_SPEED;

        // Progress
        if(NodeProgress != other->NodeProgress)
            ValidFields |= TRACKSTATE_UPDATED_PROGRESS;
    }

    packet << ValidFields;

    // Add the changed data to the packet //
    if(ValidFields & TRACKSTATE_UPDATED_NODE)
        packet << ReachedNode;

    if(ValidFields & TRACKSTATE_UPDATED_SPEED)
        packet << ChangeSpeed;
    
    if(ValidFields & TRACKSTATE_UPDATED_PROGRESS)
        packet << NodeProgress;
}










