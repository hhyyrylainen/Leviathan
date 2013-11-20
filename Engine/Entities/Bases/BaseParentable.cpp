#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASEPARENTABLE
#include "BaseParentable.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseParentable::BaseParentable() : BaseObject(-1, NULL){

}

DLLEXPORT Leviathan::BaseParentable::~BaseParentable(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseParentable::AddNonPhysicsChild(BaseParentable* childobject){
	// Add as notifiable child //
	if(!ConnectToNotifiable(childobject)){

		return false;
	}

	// Notify the child that it is now parented to this object //
	childobject->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_PARENTCONNECTED, static_cast<BasePositionable*>(this));
	return true;
}

DLLEXPORT bool Leviathan::BaseParentable::AddNonPhysicsParent(BaseParentable* parentobject){
	// Add as parent and call position update on ourself //
	if(!ConnectToNotifier(parentobject)){

		return false;
	}

	// Send to self //
	SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_PARENTCONNECTED, static_cast<BasePositionable*>(parentobject));
	return true;
}
// ------------------------------------ //
void Leviathan::BaseParentable::_ParentableNotifyLocationDataUpdated(){
	// Notify child objects //
	SendCustomMessageToChildren(ENTITYCUSTOMMESSAGETYPE_PARENTPOSITIONUPDATED, static_cast<BasePositionable*>(this), true);
}
// ------------------------------------ //
void Leviathan::BaseParentable::_OnParentablePositionUpdated(){
	// Default callback //

}
// ------------------------------------ //
bool Leviathan::BaseParentable::BaseParentableCustomMessage(int message, void* data){


	switch(message){
	case ENTITYCUSTOMMESSAGETYPE_PARENTPOSITIONUPDATED: case ENTITYCUSTOMMESSAGETYPE_PARENTCONNECTED:
		{
			BasePositionable* parentpos = reinterpret_cast<BasePositionable*>(data);
			// Apply parent positioning //
			Float3 posdifference = parentpos->GetPos()-ParentOldPos;
			Float4 orientdifference = (parentpos->GetOrientation()-ParentOldRot).Normalize();

			// Update old //
			ParentOldRot = parentpos->GetOrientation();
			ParentOldPos = parentpos->GetPos();

			// Apply position change //
			Position += posdifference;

			// Apply rotation if applicable //
			if(orientdifference.HAddAbs() != 0){

				QuatRotation = QuatRotation*orientdifference;
			}

			// Call callbacks //
			_OnParentablePositionUpdated();
			PosUpdated();
			OrientationUpdated();
			return true;
		}


	}
	return false;

}

bool Leviathan::BaseParentable::BaseParentableCustomGetData(ObjectDataRequest* data){
	// No data to return //
	return false;
}
