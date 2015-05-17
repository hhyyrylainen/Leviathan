#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TRAILEMITTER
#include "TrailEmitter.h"
#endif
#include "OgreSceneManager.h"
#include "../../Handlers/IDFactory.h"
#include "../../Utility/Convert.h"
using namespace Leviathan;
using namespace Entity;
using namespace std;
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::TrailEmitter::ReleaseData(){
    // Only release if in graphical mode //
    auto scene = OwnedByWorld->GetScene();

    if(!scene)
        return;
    
	// Destroy the Ogre resources //
	scene->destroySceneNode(TrailLocation);
	// This might be needed //
	scene->getRootSceneNode()->detachObject(TrailEntity);
	scene->destroyRibbonTrail(TrailEntity);

	TrailEntity = NULL;
	TrailLocation = NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::TrailEmitter::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	// First check if it is a request //
	if(entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_DATAREQUEST){
		// Check through components //
		ObjectDataRequest* tmprequest = reinterpret_cast<ObjectDataRequest*>(dataptr);

		BASEPOSITIONAL_CUSTOMMESSAGE_GET_CHECK;
		BASEPARENTABLE_CUSTOMMESSAGE_GET_CHECK;

		return false;
	}

	// Check through components //
	BASEPOSITIONAL_CUSTOMMESSAGE_DATA_CHECK;
	BASEPARENTABLE_CUSTOMMESSAGE_DATA_CHECK;


	// This specific //

	return false;
}
// ------------------------------------ //
void Leviathan::Entity::TrailEmitter::PosUpdated(Lock &guard){
	// Set node property //
	if(TrailLocation){
		TrailLocation->setPosition(Position);
	}
}

void Leviathan::Entity::TrailEmitter::OrientationUpdated(Lock &guard){
	// Set node property //
	if(TrailLocation){
		TrailLocation->setOrientation(QuatRotation);
	}
}

void Leviathan::Entity::TrailEmitter::_OnHiddenStateUpdated(){
	// Set node state //
	if(TrailLocation){

		TrailLocation->setVisible(!Hidden);
	}
}
// ------------------ TrailProperties ------------------ //
DLLEXPORT TrailProperties& Leviathan::Entity::TrailProperties::operator=(const TrailProperties &other){
    
	TrailLenght = other.TrailLenght;
	MaxDistance = other.MaxDistance;
	MaxChainElements = other.MaxChainElements;
	CastShadows = other.CastShadows;
    
	// We need to allocate new vector for us //
	SAFE_DELETE_VECTOR(ElementProperties);
	ElementProperties.resize(other.ElementProperties.size());
    
	for(size_t i = 0; i < ElementProperties.size(); i++){

		ElementProperties[i] = new TrailElementProperties(*other.ElementProperties[i]);
	}

	return *this;
}
