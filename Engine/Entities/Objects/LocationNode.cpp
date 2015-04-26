// ------------------------------------ //
#include "LocationNode.h"

#include "../GameWorld.h"
#include "../Bases/BaseObject.h"
#include "../Bases/BaseNotifierEntity.h"
#include "../../Handlers/IDFactory.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::LocationNode::LocationNode(GameWorld* world, bool deleteifnoowner /*= true*/) : BasePositionable(), 
	BaseObject(IDFactory::GetID(), world), DeleteIfNoParent(deleteifnoowner)
{

}

DLLEXPORT Leviathan::Entity::LocationNode::LocationNode(GameWorld* world, const Float3 &pos, const Float4 &orientation, bool deleteifnoowner /*= true*/) : 
	BasePositionable(pos, orientation), BaseObject(IDFactory::GetID(), world), DeleteIfNoParent(deleteifnoowner)
{

}

DLLEXPORT Leviathan::Entity::LocationNode::~LocationNode(){

}
// ------------------------------------ //
void Leviathan::Entity::LocationNode::_NotifyParentOfPosition(){
	// Send update notification to the parent //
	for(auto iter = ConnectedToParents.begin(); iter != ConnectedToParents.end(); ++iter){

		(*iter)->GetActualPointerToNotifierObject()->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_LOCATIONDATA_UPDATED, static_cast<BasePositionable*>(this));
	}
}
// ------------------------------------ //
void Leviathan::Entity::LocationNode::PosUpdated(){
	_NotifyParentOfPosition();
}

void Leviathan::Entity::LocationNode::OrientationUpdated(){
	_NotifyParentOfPosition();
}
// ------------------------------------ //
void Leviathan::Entity::LocationNode::_OnNotifierDisconnected(BaseNotifierEntity* parenttoremove){
	if(ConnectedToParents.size()-1 == 0 && DeleteIfNoParent){

		OwnedByWorld->QueueDestroyObject(ID);
	}
}

DLLEXPORT bool Leviathan::Entity::LocationNode::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	return false;
}



