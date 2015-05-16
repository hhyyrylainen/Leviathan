// ------------------------------------ //
#ifndef LEVIATHAN_TRACKENTITYCONTROLLER
#include "TrackEntityController.h"
#endif
#include "Entities/GameWorld.h"
#include "Entities/Bases/BasePhysicsObject.h"
#include "Entities/Bases/BaseNotifiableEntity.h"
#include "Newton/PhysicalWorld.h"
#include "../../Networking/NetworkHandler.h"
#include "../../Handlers/IDFactory.h"
using namespace Leviathan;
using namespace Entity;
using namespace std;
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

        if(IsOnClient){

            // Client will only rely on snapshots //
            DEBUG_BREAK;

            // TODO: move to a new event type
            GUARD_LOCK();
            _ApplyTrackPositioning(dataptr->TimeStep*1000, guard);
            
        } else {

            UpdateControlledPositions(dataptr->TimeStep);
        }
        
		return 1;
        
	} else if((*pEvent)->GetType() == EVENT_TYPE_CLIENT_INTERPOLATION){
        
        auto data = (*pEvent)->GetDataForClientInterpolationEvent();

        std::shared_ptr<ObjectDeltaStateData> first;
        std::shared_ptr<ObjectDeltaStateData> second;

        float progress = data->Percentage;
        
        try{
                
            GetServerSentStates(first, second, data->TickNumber, progress);
                
        } catch(const InvalidState&){

            // No more states to use //
            Logger::Get()->Write("TrackController stopping interpolation");

            ListeningForEvents = false;
            return -1;
        }

        SetStateToInterpolated(*first, *second, progress);
        return 1;
        
    } else if((*pEvent)->GetType() == EVENT_TYPE_PHYSICS_RESIMULATE_SINGLE){

		// Get data //
		ResimulateSingleEventData* dataptr = (*pEvent)->GetDataForResimulateSingleEvent();
        
		// Skip if wrong world //
		if(dataptr->GameWorldPtr != static_cast<void*>(OwnedByWorld)){
            
			return 0;
		}

        GUARD_LOCK();
        
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
// ------------------------------------ //
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




