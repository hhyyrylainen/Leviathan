// ------------------------------------ //
#ifndef LEVIATHAN_ENTITY_BRUSH
#include "Brush.h"
#endif
#include "OgreManualObject.h"
#include "OgreMeshManager.h"
#include "FileSystem.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "Entities/CommonStateObjects.h"
#include "Exceptions.h"
#include "../../Handlers/IDFactory.h"
using namespace Leviathan;
using namespace Entity;
using namespace std;
// ------------------------------------ //
DLLEXPORT void Leviathan::Entity::Brush::AddPhysicalObject(Lock &guard,
    const float &mass /*= 0.f*/)
{
	// destroy old first //

    AggressiveConstraintUnlink(guard);

	_DestroyPhysicalBody(guard);


    
}
// ------------------------------------ //
void Leviathan::Entity::Brush::_UpdatePhysicsObjectLocation(Lock &guard){
	// Update physics object location which will in turn change graphical object location //

	Ogre::Matrix4 matrix;
	matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tmatrix = matrix.transpose();

	// Update body //
	NewtonBodySetMatrix(Body, &tmatrix[0][0]);
    
    if(ObjectsNode){
        // Update graphical object location to have it always match up //
        ObjectsNode->setOrientation(QuatRotation);
        ObjectsNode->setPosition(Position);
    }

    
	// Update potential children //
	_ParentableNotifyLocationDataUpdated();

    // Notify network of new position //
    _MarkDataUpdated(guard);
}
// ------------------------------------ //
void Leviathan::Entity::Brush::BrushPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix,
    int threadIndex)
{
	// First create Ogre 4x4 matrix from the matrix //
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8],
        matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);

	// Needs to convert from d3d style matrix to OpenGL style matrix //
	Ogre::Matrix4 tmat = mat.transpose();
	Ogre::Vector3 vec = tmat.getTrans();

	Float3 position = vec;

	// Rotation //
	Float4 quat(tmat.extractQuaternion());

	// Apply to graphical object //
	Brush* tmp = static_cast<Brush*>(reinterpret_cast<BasePhysicsObject*>(NewtonBodyGetUserData(body)));
    
    // The object needs to be locked here //
    GUARD_LOCK_OTHER(tmp);
    
    if(tmp->ObjectsNode){
        
        tmp->ObjectsNode->setOrientation(quat);
        tmp->ObjectsNode->setPosition(position);
    }
    
	// Also update these so if only one is updated it doesn't force last value to rotation or location //
	tmp->Position = position;
	tmp->QuatRotation = quat;
    
	// Update potential children //
	tmp->_ParentableNotifyLocationDataUpdated();

    tmp->_MarkDataUpdated(guard);
}
// ------------------------------------ //
bool Leviathan::Entity::Brush::_LoadOwnDataFromPacket(Lock &guard, sf::Packet &packet){

    // First get the base class data //
    BasePositionData pdata;

    if(!LoadPositionFromPacketToHolder(packet, pdata)){

        // It failed (the packet was invalid) //
        Logger::Get()->Error("Brush: packet has invalid format");
        return false;
    }

    float x, y, z;
    bool physics;
    float mass;
    string matname;
    int physid;

    // Get all the things at once and then check for invalid state //
    packet >> x >> y >> z;
    packet >> physics;
    
    if(physics){
        
        packet >> mass >> physid;
    }

    packet >> matname;

    // Don't apply data (Init) if the packet was invalid //
    if(!packet){

        Logger::Get()->Error("Brush: packet has invalid format");
        return false;
    }
        
    // We always create the physical object ourselves if wanted
    if(!Init(guard, Float3(x, y, z), matname, false)){

        // This shouldn't happen //
        Logger::Get()->Error("Brush: failed to create from packet, Init failed");
        return false;
    }

    if(physics){

        AddPhysicalObject(guard, mass);
        
        if(physid >= 0)
            SetPhysicalMaterialID(guard, physid);
    }
    
    // Then set the position //
    ApplyPositionDataObject(guard, pdata);

    // Apply hidden state //
    _OnHiddenStateUpdated(guard);
    
    return true;
}

void Leviathan::Entity::Brush::_SaveOwnDataToPacket(Lock &guard, sf::Packet &packet){

    // The hidden state needs to be the first thing
    packet << Hidden;
    
    // Before adding our data make base classes add stuff //
    AddPositionAndRotationToPacket(guard, packet);

    // First add the size //
    packet << Sizes.X << Sizes.Y << Sizes.Z;

    // Then whether we have a physical object or not //
    packet << (GetPhysicsBody(guard) ? true: false);

    // Add the mass if it is applicable //
    if(GetPhysicsBody(guard)){

        packet << Mass << AppliedPhysicalMaterial;
    }

    // And finally our material //
    packet << Material;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Brush::SendCustomMessage(int entitycustommessagetype,
    void* dataptr)
{
	// First check if it is a request //
	if(entitycustommessagetype == ENTITYCUSTOMMESSAGETYPE_DATAREQUEST){
		// Check through components //
		ObjectDataRequest* tmprequest = reinterpret_cast<ObjectDataRequest*>(dataptr);

		BASEPOSITIONAL_CUSTOMMESSAGE_GET_CHECK;
		BASEPHYSICS_CUSTOMMESSAGE_GET_CHECK;
		BASEPARENTABLE_CUSTOMMESSAGE_GET_CHECK;

		return false;
	}

	// Check through components //
	BASEPOSITIONAL_CUSTOMMESSAGE_DATA_CHECK;
	BASEPHYSICS_CUSTOMMESSAGE_DATA_CHECK;
	BASEPARENTABLE_CUSTOMMESSAGE_DATA_CHECK;

	// This specific //

	return false;
}
// ------------------------------------ //
void Leviathan::Entity::Brush::_SendCreatedConstraint(BaseConstraintable* other, Entity::BaseConstraint* ptr){
    _SendNewConstraint(static_cast<BaseConstraintable*>(this), other, ptr);
}
// ------------------------------------ //
BaseConstraintable* Leviathan::Entity::Brush::BasePhysicsGetConstraintable(){
    return static_cast<BaseConstraintable*>(this);
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<ObjectDeltaStateData> Leviathan::Entity::Brush::CaptureState(
    Lock &guard, int tick)
{
    return std::shared_ptr<ObjectDeltaStateData>(
        PositionableRotationableDeltaState::CaptureState(guard, *this, tick).release());
}

DLLEXPORT std::shared_ptr<ObjectDeltaStateData> Leviathan::Entity::Brush::CreateStateFromPacket(
    int tick, sf::Packet &packet) const
{
    
    try{
        
        return make_shared<PositionableRotationableDeltaState>(tick, packet);
        
    } catch(const InvalidArgument &e){

        Logger::Get()->Warning("Brush: failed to CreateStateFromPacket, exception:");
        e.PrintToLog();
        return nullptr;
    }
    
}
// ------------------------------------ //
void Brush::_OnNewStateReceived(){

    if(!ListeningForEvents){
            
        RegisterForEvent(EVENT_TYPE_CLIENT_INTERPOLATION);
        ListeningForEvents = true;
    }
}

DLLEXPORT int Brush::OnEvent(Event** pEvent){
    
    if((*pEvent)->GetType() == EVENT_TYPE_CLIENT_INTERPOLATION){

        if(!ListeningForEvents)
            DEBUG_BREAK;
        
        auto data = (*pEvent)->GetDataForClientInterpolationEvent();

        std::shared_ptr<ObjectDeltaStateData> first;
        std::shared_ptr<ObjectDeltaStateData> second;

        float progress = data->Percentage;
        
        try{
                
            GetServerSentStates(first, second, data->TickNumber, progress);
                
        } catch(const InvalidState&){

            // No more states to use //
            Logger::Get()->Write("Entity stopping interpolation");

            ListeningForEvents = false;
            return -1;
        }

        InterpolatePositionableState(static_cast<PositionableRotationableDeltaState&>(*first),
            static_cast<PositionableRotationableDeltaState&>(*second), progress);

        return 1;
    }

    return -1;
}

DLLEXPORT int Brush::OnGenericEvent(GenericEvent** pevent){
    return -1;
}


