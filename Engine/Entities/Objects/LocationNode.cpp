#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LOCATIONNODEENTITY
#include "LocationNode.h"
#endif
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

		iter->SendCustomMessage(ENTITYCUSTOMMESSAGETYPE_LOCATIONDATA_UPDATED, static_cast<BasePositionable*>(this));
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
void Leviathan::Entity::LocationNode::_OnNotifierDisconnected(BaseNotifier* parenttoremove){
	if(ConnectedToParents.size() == 0 && DeleteIfNoParent){

		LinkedToWorld->QueueDestroyObject(ID);
	}
}



