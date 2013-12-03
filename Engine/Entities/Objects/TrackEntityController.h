#ifndef LEVIATHAN_TRACKENTITYCONTROLLER
#define LEVIATHAN_TRACKENTITYCONTROLLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\Bases\BaseEntityController.h"
#include "LocationNode.h"
#include "Events\CallableObject.h"

#define TRACKCONTROLLER_DEFAULT_APPLYFORCE		12.f

namespace Leviathan{ namespace Entity{

	// Struct that can be used to tell this object to create nodes //
	struct TrackControllerPosition{
		TrackControllerPosition(){
		}
		TrackControllerPosition(const Float3 &pos, const Float4 &orientation) : Pos(pos), Orientation(orientation){
		}

		Float3 Pos;
		Float4 Orientation;
	};


	// This class is used to create movement paths for entities //
	class TrackEntityController : public BaseEntityController, virtual public BaseObject, public CallableObject{
	public:
		DLLEXPORT TrackEntityController(GameWorld* world);
		DLLEXPORT virtual ~TrackEntityController();

		// Init starts listening to events and verify parameters //
		DLLEXPORT bool Init();
		DLLEXPORT virtual void ReleaseData();

		// Events are used to receive when render is about to happen and update positions //
		DLLEXPORT virtual int OnEvent(Event** pEvent);
		// This function doesn't do anything (except return -1) //
		DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent);

		// Directly sets the progress towards next node (if set to 1.f goes to next node) //
		DLLEXPORT void SetProgressTowardsNextNode(float progress);
		// Gets the progress towards next node, if at 1.f then last node is reached //
		DLLEXPORT float GetProgressTowardsNextNode(){
			return NodeProgress;
		}

		// Controls the speed at which the entity moves along the track (set to negative to go backwards and 0.f to stop) //
		DLLEXPORT inline void SetTrackAdvanceSpeed(const float &speed){
			ChangeSpeed = speed;
		}
		DLLEXPORT inline float GetTrackAdvanceSpeed(){
			return ChangeSpeed;
		}

		// Gets the position of the current node (or Float3(0) if no nodes exist) //
		DLLEXPORT Float3 GetCurrentNodePosition();
		// Gets the position of the next node, or Float3(0) if final node is reached. Doesn't throw exceptions //
		DLLEXPORT Float3 GetNextNodePosition();

		// This function creates a new node to the world and ads it to the track of this object //
		DLLEXPORT void AddLocationToTrack(const Float3 &pos, const Float4 &dir);

		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

		// When called updates the entity positions (You probably don't have to manually call this) //
		DLLEXPORT virtual void UpdateControlledPositions(float timestep);

	protected:
		// Internal function for making all data valid (checks for invalid reached node and progress) //
		void _SanityCheckNodeProgress();
		// Updates the controlled object //
		void _ApplyTrackPositioning(float timestep);

		// Callback for detecting node unlinks //
		virtual void _OnNotifiableDisconnected(BaseNotifiable* childtoremove);
		// ------------------------------------ //
		// Number of the node that has been reached //
		int ReachedNode;

		// Percentage between ReachedNode and next node (1.f being next node reached and progress reset to 0) //
		float NodeProgress;

		// The speed at which the node progress changes //
		float ChangeSpeed;

		// The amount of speed/force used to move the entities towards the track position //
		float ForceTowardsPoint;

		// List of positions that form the track //
		// Note these nodes are also on the inherited child object list //
		std::vector<LocationNode*> TrackNodes;

		// Internal flag for determining if an update is needed //
		bool RequiresUpdate;

	};

}}
#endif