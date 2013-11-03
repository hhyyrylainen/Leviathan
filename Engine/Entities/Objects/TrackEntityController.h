#ifndef LEVIATHAN_FILEREPLACENAME
#define LEVIATHAN_FILEREPLACENAME
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\Bases\BaseEntityController.h"
#include "LocationNode.h"
#include "Events\CallableObject.h"


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
		DLLEXPORT virtual void Release();

		// Events are used to receive when render is about to happen and update positions //
		DLLEXPORT virtual int OnEvent(Event** pEvent);
		DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent);

		// Directly sets the progress towards next node (if set to 1.f goes to next node) //
		DLLEXPORT void SetProgressTowardsNextNode(float progress);
		// Gets the progress towards next node, if at 1.f then last node is reached //
		DLLEXPORT float GetProgressTowardsNextNode();

		DLLEXPORT void AddLocationToTrack(const Float3 &pos, const Float4 &dir);

		// When called updates the entity positions (You probably don't have to manually call this) //
		DLLEXPORT virtual void UpdateControlledPositions(int mspassed);

	protected:
		// Internal function for making all data valid (checks for invalid reached node and progress) //
		void _SanityCheckNodeProgress();


		// Callback for detecting node unlinks //
		virtual void _OnNotifiableDisconnected(BaseNotifiable* childtoremove);
		// ------------------------------------ //
		// Number of the node that has been reached //
		int ReachedNode;

		// Percentage between ReachedNode and next node (1.f being next node reached and progress reset to 0) //


		// List of positions that form the track //
		// Note these nodes are also on the inherited child object list //
		std::vector<LocationNode*> TrackNodes;

		// Internal flag for determining if an update is needed //
		bool RequiresUpdate;

	};

}}
#endif