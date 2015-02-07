// ------------------------------------ //
#ifndef LEVIATHAN_ENTITY_PROP
#include "Prop.h"
#endif
#include "ObjectFiles/ObjectFileProcessor.h"
#include "OgreMatrix4.h"
#include <Newton.h>
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "Entities/CommonStateObjects.h"
using namespace Leviathan;
using namespace Leviathan::Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::Prop::Prop(bool hidden, GameWorld* world) :
    BaseRenderable(hidden), BaseObject(IDFactory::GetID(), world), BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_PROP)
{

}

Leviathan::Entity::Prop::Prop(bool hidden, GameWorld* world, int netid) :
    BaseRenderable(hidden), BaseObject(netid, world), BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_PROP)
{

}

DLLEXPORT Leviathan::Entity::Prop::~Prop(){

    ReleaseData();
}

DLLEXPORT void Leviathan::Entity::Prop::ReleaseData(){

    StopInterpolating();

    ReleaseParentHooks();

    GUARD_LOCK_THIS_OBJECT();

    
	// Release Ogre entity //
    if(OwnedByWorld){
        if(ObjectsNode)
            OwnedByWorld->GetScene()->destroySceneNode(ObjectsNode);
        if(GraphicalObject)
            OwnedByWorld->GetScene()->destroyEntity(GraphicalObject);
    }

	GraphicalObject = NULL;
    ObjectsNode = NULL;
    
	OwnedByWorld = NULL;

	// physical entity //
	AggressiveConstraintUnlink();

    if(OwnedByWorld)
        _DestroyPhysicalBody();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Prop::Init(const wstring &modelfile){

    // Store the file //
    ModelFile = modelfile;
    
	// Parse the file //
	auto file = ObjectFileProcessor::ProcessObjectFile(modelfile);

	if(!file){

		return false;
	}

	wstring ogrefile;

	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(file->GetVariables(), L"Model-Graphical", ogrefile, L"error",
        true, L"Prop: Init: no model file");

	// Load the Ogre entity if in graphical mode //
    if(OwnedByWorld->GetScene()){
        
        GraphicalObject = OwnedByWorld->GetScene()->createEntity(Convert::WstringToString(ogrefile));

        // Create scene node for positioning //
        ObjectsNode = OwnedByWorld->GetScene()->getRootSceneNode()->createChildSceneNode();

        // attach for deletion and valid display //
        ObjectsNode->attachObject(GraphicalObject);
    }
	
	// Find the physics object definition //
	auto phyobj = file->GetObjectWithType(L"PhysicalModel");

	if(!phyobj){
		// Nothing else to handle //
		return true;
	}

	auto proplist = phyobj->GetListWithName(L"properties");

	if(!proplist){
		// Physical properties don't have to be processed //
		return true;
	}
	
	NewtonWorld* tmpworld = OwnedByWorld->GetPhysicalWorld()->GetNewtonWorld();

	// first get the type //
	wstring ptype;

	// get offset //
	wstring offsettype;
	Ogre::Matrix4 offset = Ogre::Matrix4::IDENTITY;
	// this is useful when the origin is at the bottom of the model and you don't take this into account in
    // newton primitive (using Convex hull should not require this)
	if(ObjectFileProcessor::LoadValueFromNamedVars<wstring>(proplist->GetVariables(), L"Offset", offsettype, L"", true, 
		L"Prop: Init: CreatePhysicsModel"))
	{

		if(offsettype == L"None"){
			// nothing needs to set //


		} else if(offsettype == L"BoundingBoxCenter"){

            // There needs to be a workaround in non-graphical mode //
            if(!GraphicalObject){

                Logger::Get()->Error(L"Prop: Init: trying to use BoundingBoxCenter as offset in non-graphical mode"
                    L" and there is no workaround to calculate that...yet");
                return false;
            }

			Ogre::Aabb bbox = GraphicalObject->getLocalAabb();
			offset.setTrans(bbox.mCenter);

		} else {
			Logger::Get()->Error(L"Prop: Init: invalid offset type, use None or BoundingBoxCenter for most common "
                L"cases, file: "+modelfile);
			return false;
		}
	}

	// Newton uses different handed matrices...
	Ogre::Matrix4 toffset = offset.transpose();


	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(proplist->GetVariables(), L"PrimitiveType", ptype, L"Convex");

	// Process first the most complicated one, Convex hull which is basically a prop that we need to load //
	if(ptype == L"Convex"){

		Logger::Get()->Error(L"Prop: Init: physical object Convex hull loading is not implemented!");
		return false;

	} else if(ptype == L"Sphere" || ptype == L"Ball"){

		// Sphere primitive type, now we just need to load the size parameter or
        // count it from the object bounding box
		float radius = 0.f;

		if(!ObjectFileProcessor::LoadValueFromNamedVars<float>(proplist->GetVariables(), L"Size", radius, 0.f)){
			// it should be string type //
			wstring sizesourcename;
			
			if(!ObjectFileProcessor::LoadValueFromNamedVars<wstring>(proplist->GetVariables(), L"Size", sizesourcename,
                    L"", true, L"Prop: Init: CreatePhysicsModel:"))
			{
				Logger::Get()->Error(L"Prop: Init: physical model has no size! at least specify "
                    L"\"Size = GraphicalModel;\", file: "+modelfile);
				return false;
			}

			// Process based on the source name //
			if(sizesourcename == L"GraphicalModel"){

                // There needs to be a workaround in non-graphical mode //
                if(!GraphicalObject){

                    Logger::Get()->Error(L"Prop: Init: trying to use GraphicalModel as size specification "
                        L" but in non-gui mode graphical object isn't loaded"
                        L" and there is no workaround to calculate that...yet");
                    return false;
                }
                
				// Calculate the radius from bounding box size //
				Ogre::Aabb bbox = GraphicalObject->getLocalAabb();
				Ogre::Vector3 sizes = bbox.getSize();

				// A little sanity check //
				if(sizes.x != sizes.y || sizes.x != sizes.z){
					// it's not cube //
					Logger::Get()->Warning(L"Prop: Init: physical model sphere, the bounding box of graphical model is "
                        L"not a cube, continuing anyways");
				}

				radius = sizes.x;


			} else {
				Logger::Get()->Error(L"Prop: Init: physical model has no size! unknown source: "+sizesourcename+
                    L" (\"Size = GraphicalModel;\"), file: "+modelfile);
				return false;
			}
		}
		// Radius should be fine now //

		// Create the sphere now //
		Collision = NewtonCreateSphere(tmpworld, radius, 0, &toffset[0][0]);

	} else if(ptype == L"Box"){

		DEBUG_BREAK;
		//Collision = NewtonCreateBox(tmpworld, sizes.x, sizes.y, sizes.z, NULL, &toffset[0][0]);
	} else {
		Logger::Get()->Error(L"Prop: Init: physical model has no type! unknown typename: "+ptype+
            L", \"PrimitiveType = Convex;\" for mesh collisions, file: "+modelfile);
		return false;

	}


	// Create the body //
	Ogre::Matrix4 locrot = Ogre::Matrix4::IDENTITY;
	locrot.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tlocrot = locrot.transpose();

	Body = NewtonCreateDynamicBody(tmpworld, Collision, &tlocrot[0][0]);
	
	// Add this as user data //
	NewtonBodySetUserData(Body, static_cast<BasePhysicsObject*>(this));

	// Get mass //
	float Mass = 0;
	ObjectFileProcessor::LoadValueFromNamedVars<float>(proplist->GetVariables(), L"Mass", Mass, 0.f, true,
        L"Prop: Init: CreatePhysicsModel:");

	// First calculate inertia and center of mass points //
	Float3 inertia;
	Float3 centerofmass;

	NewtonConvexCollisionCalculateInertialMatrix(Collision, &inertia.X, &centerofmass.X);

	// Apply mass to inertia 
	inertia *= Mass;

	NewtonBodySetMassMatrix(Body, Mass, inertia.X, inertia.Y, inertia.Z);
	NewtonBodySetCentreOfMass(Body, &centerofmass.X);


	// Callbacks //
	NewtonBodySetTransformCallback(Body, Prop::PropPhysicsMovedEvent);
	NewtonBodySetForceAndTorqueCallback(Body, BasePhysicsObject::ApplyForceAndTorgueEvent);
	NewtonBodySetDestructorCallback(Body, BasePhysicsObject::DestroyBodyCallback);

	// Should be fine //
	return true;
}
// ------------------------------------ //
void Leviathan::Entity::Prop::_UpdatePhysicsObjectLocation(ObjectLock &guard){
	// update physics object location which will in turn change graphical object location //

	if(Body){
		Ogre::Matrix4 matrix;
		matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

		Ogre::Matrix4 tmatrix = matrix.transpose();

		// update body //
		NewtonBodySetMatrix(Body, &tmatrix[0][0]);
	}

	// update graphical object location to have it always match up (only for static objects) //
    if(ObjectsNode){
        
        ObjectsNode->setOrientation(QuatRotation);
        ObjectsNode->setPosition(Position);
    }
    
	// Update potential children //
	_ParentableNotifyLocationDataUpdated();

    // Notify network of new position //
    _MarkDataUpdated(guard);
}

// ------------------------------------ //
void Leviathan::Entity::Prop::PropPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix,
    int threadIndex)
{

	// first create Ogre 4x4 matrix from the matrix //
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8],
        matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);

	// this might be a "cleaner" way to do this //
	//memcpy(mat.m, matrix, sizeof(float)*16);

	// needs to convert from d3d style matrix to OpenGL style matrix //
	Ogre::Matrix4 tmat = mat.transpose();

	Ogre::Vector3 vec = tmat.getTrans();

	Float3 position = vec;

	// rotation //
	Float4 quat(tmat.extractQuaternion());

	// apply to graphical object //
	Prop* tmp = static_cast<Prop*>(reinterpret_cast<BasePhysicsObject*>(NewtonBodyGetUserData(body)));

    // The object needs to be locked here //
    GUARD_LOCK_OTHER_OBJECT(tmp);
    
    if(tmp->ObjectsNode){
        
        tmp->ObjectsNode->setOrientation(quat);
        tmp->ObjectsNode->setPosition(position);
    }
    
	// also update these so if only one is updated it doesn't force last value to rotation or location //
	tmp->Position = position;
	tmp->QuatRotation = quat;
    
	// Update potential children //
	tmp->_ParentableNotifyLocationDataUpdated();

    tmp->_MarkDataUpdated(guard);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Prop::SendCustomMessage(int entitycustommessagetype, void* dataptr){
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
void Leviathan::Entity::Prop::_GetCurrentActualPosition(Float3 &pos){

    GetPos(pos);
}
        
void Leviathan::Entity::Prop::_GetCurrentActualRotation(Float4 &rot){

    GetOrientation(rot);
}
// ------------------------------------ //
bool Leviathan::Entity::Prop::_LoadOwnDataFromPacket(sf::Packet &packet){

    BasePositionData posdata;
    BasePhysicsData phydata;

    if(!LoadPositionFromPacketToHolder(packet, posdata)){

        // It failed (the packet was invalid) //
        Logger::Get()->Error("Prop: packet has invalid format");
        return false;
    }

    if(!LoadPhysicalStateFromPacket(packet, phydata)){

        Logger::Get()->Error("Prop: packet has invalid format");
        return false;
    }

    packet >> ModelFile;

    // TODO: move this to physical state and unify with Brush
    bool physics;
    int physid;
    
    packet >> physics;
    
    if(physics){
        
        packet >> physid;
    }


    if(!packet){

        Logger::Get()->Error("Prop: packet has invalid format");
        return false;
    }

    if(!Init(ModelFile)){

        // This shouldn't happen //
        Logger::Get()->Error("Prop: failed to create from packet, Init failed");
        return false;
    }

    // Then set the position //
    ApplyPositionDataObject(posdata);

    // And velocity //
    ApplyPhysicalState(phydata);

    // Apply hidden state //
    _OnHiddenStateUpdated();

    // Apply physical material //
    if(physics){

        if(physid >= 0)
            SetPhysicalMaterialID(physid);
    }

    
    return true;
}

void Leviathan::Entity::Prop::_SaveOwnDataToPacket(sf::Packet &packet){
    GUARD_LOCK_THIS_OBJECT();
    
    // The hidden state needs to be the first thing
    packet << Hidden;
    
    // Before adding our data make base classes add stuff //
    AddPositionAndRotationToPacket(packet);

    // Physics state //
    AddPhysicalStateToPacket(packet);
    
    packet << ModelFile;

    packet << (GetPhysicsBody() ? true: false);

    // Add the mass if it is applicable //
    if(GetPhysicsBody()){

        packet << AppliedPhysicalMaterial;
    }
}
// ------------------------------------ //
void Leviathan::Entity::Prop::_SendCreatedConstraint(BaseConstraintable* other, Entity::BaseConstraint* ptr){
    _SendNewConstraint(static_cast<BaseConstraintable*>(this), other, ptr);
}
// ------------------------------------ //
BaseConstraintable* Leviathan::Entity::Prop::BasePhysicsGetConstraintable(){
    return static_cast<BaseConstraintable*>(this);
}
// ------------------------------------ //
DLLEXPORT shared_ptr<ObjectDeltaStateData> Leviathan::Entity::Prop::CaptureState(){
    
    return shared_ptr<ObjectDeltaStateData>(
        PositionablePhysicalDeltaState::CaptureState(*this).release());
}

DLLEXPORT void Leviathan::Entity::Prop::VerifyOldState(ObjectDeltaStateData* serversold, ObjectDeltaStateData* ourold,
    int tick)
{
    CheckOldPhysicalState(static_cast<PositionablePhysicalDeltaState*>(serversold),
        static_cast<PositionablePhysicalDeltaState*>(ourold), tick, this);
}

void Leviathan::Entity::Prop::OnBeforeResimulateStateChanged(){
    StartInterpolating(GetPos(), GetOrientation());
}

DLLEXPORT shared_ptr<ObjectDeltaStateData> Leviathan::Entity::Prop::CreateStateFromPacket(sf::Packet &packet) const{
    
    try{
        
        return make_shared<PositionablePhysicalDeltaState>(packet);
        
    } catch(ExceptionInvalidArgument &e){

        Logger::Get()->Warning("Prop: failed to CreateStateFromPacket, exception:");
        e.PrintToLog();
        return nullptr;
    }
    
}
