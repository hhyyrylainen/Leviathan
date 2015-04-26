#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TRAILEMITTER
#include "TrailEmitter.h"
#endif
#include "OgreBillboardChain.h"
#include "OgreRibbonTrail.h"
#include "OgreSceneManager.h"
#include "../../Handlers/IDFactory.h"
#include "../../Utility/Convert.h"
using namespace Leviathan;
using namespace Entity;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::TrailEmitter::TrailEmitter(GameWorld* world, bool hidden /*= false*/) :
    BaseRenderable(hidden), BaseObject(IDFactory::GetID(), world), TrailEntity(NULL), TrailLocation(NULL)
{

}

DLLEXPORT Leviathan::Entity::TrailEmitter::~TrailEmitter(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::TrailEmitter::Init(const string &materialname, const TrailProperties &variables,
    bool allowupdate /*= true*/)
{
    
    
	// Create Ogre objects //
	Ogre::SceneManager* tmpmanager = OwnedByWorld->GetScene();

    // Only create if graphical mode is on //
    if(!tmpmanager)
        return true;
    
	// Create node //
	TrailLocation = tmpmanager->getRootSceneNode()->createChildSceneNode();

	// Trail entity //
	TrailEntity = tmpmanager->createRibbonTrail();
	TrailEntity->setName("TrailEmitter_"+Convert::ToString(ID));
	TrailEntity->setMaterialName(materialname);

	// Set dynamic update if wanted //
	if(allowupdate){
		TrailEntity->setDynamic(true);
	}

	// Apply the settings, this also adds the node to the trail //
	SetTrailProperties(variables, true);

	// Add to root node to include in the scene //
	tmpmanager->getRootSceneNode()->attachObject(TrailEntity);


	return true;
}

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
DLLEXPORT bool Leviathan::Entity::TrailEmitter::SetTrailProperties(const TrailProperties &variables,
    bool force /*= false*/)
{
	if(!TrailEntity)
		return false;

	// Set if we unconnected the node and we should reconnect it afterwards //
	bool ConnectAgain = false;

	// Determine if we need to unconnect the node //
	if(force || variables.MaxChainElements != CachedSettings.MaxChainElements){

		// This to avoid Ogre bug //
		TrailEntity->removeNode(TrailLocation);
		ConnectAgain = true;

		// Apply the properties //
		TrailEntity->setUseVertexColours(true);
		TrailEntity->setRenderingDistance(variables.MaxDistance);
		TrailEntity->setMaxChainElements(variables.MaxChainElements);
		TrailEntity->setCastShadows(variables.CastShadows);
		TrailEntity->setTrailLength(variables.TrailLenght);

	}

	// Update cached settings //
	CachedSettings = variables;

	// Apply per element properties //
	for(size_t i = 0; i < variables.ElementProperties.size(); i++){
		// Apply settings //
		const TrailElementProperties* tmp = variables.ElementProperties[i];

		if(tmp){
			TrailEntity->setInitialColour(i, tmp->InitialColour);
			TrailEntity->setInitialWidth(i, tmp->InitialSize);
			TrailEntity->setColourChange(i, tmp->ColourChange);
			TrailEntity->setWidthChange(i, tmp->SizeChange);
		}
	}

	// More bug avoiding //
	if(ConnectAgain)	
		TrailEntity->addNode(TrailLocation);

	return true;
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
void Leviathan::Entity::TrailEmitter::PosUpdated(){
	// Set node property //
	if(TrailLocation){
		TrailLocation->setPosition(Position);
	}
}

void Leviathan::Entity::TrailEmitter::OrientationUpdated(){
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
