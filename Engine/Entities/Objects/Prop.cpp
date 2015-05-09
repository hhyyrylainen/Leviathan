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
#include "Exceptions.h"
#include "../../Handlers/IDFactory.h"
using namespace Leviathan;
using namespace Entity;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::Prop::Prop(bool hidden, GameWorld* world) :
    BaseRenderable(hidden), BaseObject(IDFactory::GetID(), world), BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_PROP)
{
    ListeningForEvents = false;
}

Leviathan::Entity::Prop::Prop(bool hidden, GameWorld* world, int netid) :
    BaseRenderable(hidden), BaseObject(netid, world), BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE_PROP)
{
    ListeningForEvents = false;
}

DLLEXPORT Leviathan::Entity::Prop::~Prop(){
    
    ReleaseData();
}

DLLEXPORT void Leviathan::Entity::Prop::ReleaseData(){

    UnRegisterAllEvents();    

    ReleaseParentHooks();

    {
        GUARD_LOCK_NAME(lockit);

        AggressiveConstraintUnlink(lockit);
    }

    GUARD_LOCK();

    
	// Release Ogre entity //
    if(OwnedByWorld){
        if(ObjectsNode)
            OwnedByWorld->GetScene()->destroySceneNode(ObjectsNode);
        if(GraphicalObject)
            OwnedByWorld->GetScene()->destroyEntity(GraphicalObject);
    }

	GraphicalObject = NULL;
    ObjectsNode = NULL;
    
    if(OwnedByWorld)
        _DestroyPhysicalBody(guard);

    OwnedByWorld = NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Prop::Init(Lock &guard, const string &modelfile){

    // Store the file //
    ModelFile = modelfile;
    
	// Parse the file //
	auto file = ObjectFileProcessor::ProcessObjectFile(modelfile);

	if(!file){

		return false;
	}

	string ogrefile;

	ObjectFileProcessor::LoadValueFromNamedVars<string>(file->GetVariables(), "Model-Graphical",
        ogrefile, "error", true, "Prop: Init: no model file");

	// Load the Ogre entity if in graphical mode //
    if(OwnedByWorld->GetScene()){
        
        GraphicalObject =
            OwnedByWorld->GetScene()->createEntity(ogrefile);

        // Create scene node for positioning //
        ObjectsNode = OwnedByWorld->GetScene()->getRootSceneNode()->createChildSceneNode();

        // attach for deletion and valid display //
        ObjectsNode->attachObject(GraphicalObject);
    }
	
	// Find the physics object definition //
	auto phyobj = file->GetObjectWithType("PhysicalModel");

	if(!phyobj){
		// Nothing else to handle //
		return true;
	}

	auto proplist = phyobj->GetListWithName("properties");

	if(!proplist){
		// Physical properties don't have to be processed //
		return true;
	}
	
	NewtonWorld* tmpworld = OwnedByWorld->GetPhysicalWorld()->GetNewtonWorld();

	// first get the type //
	string ptype;

	// get offset //
	string offsettype;
	Ogre::Matrix4 offset = Ogre::Matrix4::IDENTITY;
	// this is useful when the origin is at the bottom of the model and you don't take
    // this into account in newton primitive (using Convex hull should not require this)
	if(ObjectFileProcessor::LoadValueFromNamedVars<string>(proplist->GetVariables(), "Offset",
            offsettype, "", true, "Prop: Init: CreatePhysicsModel"))
	{

		if(offsettype == "None"){
			// nothing needs to set //


		} else if(offsettype == "BoundingBoxCenter"){

            // There needs to be a workaround in non-graphical mode //
            if(!GraphicalObject){

                Logger::Get()->Error("Prop: Init: trying to use BoundingBoxCenter as offset in "
                    "non-graphical mode and there is no workaround to calculate that...yet");
                return false;
            }

			Ogre::Aabb bbox = GraphicalObject->getLocalAabb();
			offset.setTrans(bbox.mCenter);

		} else {
            
			Logger::Get()->Error("Prop: Init: invalid offset type, use None or BoundingBoxCenter "
                "for most common cases, file: "+modelfile);
			return false;
		}
	}

	// Newton uses different handed matrices...
	Ogre::Matrix4 toffset = offset.transpose();


	ObjectFileProcessor::LoadValueFromNamedVars<string>(proplist->GetVariables(),
        "PrimitiveType", ptype, "Convex");

	// Process first the most complicated one, Convex hull which is basically a prop that
    // we need to load
	if(ptype == "Convex"){

		Logger::Get()->Error("Prop: Init: physical object Convex hull loading is not "
            "implemented!");
		return false;

	} else if(ptype == "Sphere" || ptype == "Ball"){

		// Sphere primitive type, now we just need to load the size parameter or
        // count it from the object bounding box
		float radius = 0.f;

		if(!ObjectFileProcessor::LoadValueFromNamedVars<float>(proplist->GetVariables(), "Size",
                radius, 0.f))
        {
			// it should be string type //
			string sizesourcename;
			
			if(!ObjectFileProcessor::LoadValueFromNamedVars<string>(proplist->GetVariables(),
                    "Size", sizesourcename, "", true, "Prop: Init: CreatePhysicsModel:"))
			{
				Logger::Get()->Error("Prop: Init: physical model has no size! at least specify "
                    "\"Size = GraphicalModel;\", file: "+modelfile);
				return false;
			}

			// Process based on the source name //
			if(sizesourcename == "GraphicalModel"){

                // There needs to be a workaround in non-graphical mode //
                if(!GraphicalObject){

                    Logger::Get()->Error("Prop: Init: trying to use GraphicalModel as size "
                        "specification  but in non-gui mode graphical object isn't loaded"
                        " and there is no workaround to calculate that...yet");
                    return false;
                }
                
				// Calculate the radius from bounding box size //
				Ogre::Aabb bbox = GraphicalObject->getLocalAabb();
				Ogre::Vector3 sizes = bbox.getSize();

				// A little sanity check //
				if(sizes.x != sizes.y || sizes.x != sizes.z){
					// it's not cube //
					Logger::Get()->Warning("Prop: Init: physical model sphere, the bounding box "
                        "of graphical model is not a cube, continuing anyways");
				}

				radius = sizes.x;

			} else {
				Logger::Get()->Error("Prop: Init: physical model has no size! unknown source: "
                    +sizesourcename+" (\"Size = GraphicalModel;\"), file: "+modelfile);
				return false;
			}
		}
		// Radius should be fine now //

		// Create the sphere now //
		Collision = NewtonCreateSphere(tmpworld, radius, 0, &toffset[0][0]);

	} else if(ptype == "Box"){

		DEBUG_BREAK;
		//Collision = NewtonCreateBox(tmpworld, sizes.x, sizes.y, sizes.z, NULL, &toffset[0][0]);
	} else {
		Logger::Get()->Error("Prop: Init: physical model has no type! unknown typename: "+ptype+
            ", \"PrimitiveType = Convex;\" for mesh collisions, file: "+modelfile);
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
	ObjectFileProcessor::LoadValueFromNamedVars<float>(proplist->GetVariables(), "Mass",
        Mass, 0.f, true, "Prop: Init: CreatePhysicsModel:");

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
void Leviathan::Entity::Prop::_UpdatePhysicsObjectLocation(Lock &guard){
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
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6],
        matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13],
        matrix[14], matrix[15]);

	// this might be a "cleaner" way to do this //
	//memcpy(mat.m, matrix, sizeof(float)*16);

	// needs to convert from d3d style matrix to OpenGL style matrix //
	Ogre::Matrix4 tmat = mat.transpose();

	Ogre::Vector3 vec = tmat.getTrans();

	Float3 position = vec;

	// rotation //
	Float4 quat(tmat.extractQuaternion());

	// apply to graphical object //
	Prop* tmp = static_cast<Prop*>(reinterpret_cast<BasePhysicsObject*>(
            NewtonBodyGetUserData(body)));

    // The object needs to be locked here //
    GUARD_LOCK_OTHER(tmp);
    
    if(tmp->ObjectsNode){
        
        tmp->ObjectsNode->setOrientation(quat);
        tmp->ObjectsNode->setPosition(position);
    }
    
	// Also update these so if only one is updated it doesn't force last value to
    // rotation or location
	tmp->Position = position;
	tmp->QuatRotation = quat;
    
	// Update potential children //
	tmp->_ParentableNotifyLocationDataUpdated();

    tmp->_MarkDataUpdated(guard);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Prop::SendCustomMessage(int entitycustommessagetype,
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
bool Leviathan::Entity::Prop::_LoadOwnDataFromPacket(Lock &guard, sf::Packet &packet){

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

    if(!Init(guard, ModelFile)){

        // This shouldn't happen //
        Logger::Get()->Error("Prop: failed to create from packet, Init failed");
        return false;
    }

    // Then set the position //
    ApplyPositionDataObject(guard, posdata);

    // And velocity //
    ApplyPhysicalState(guard, phydata);

    // Apply hidden state //
    _OnHiddenStateUpdated(guard);

    // Apply physical material //
    if(physics){

        if(physid >= 0)
            SetPhysicalMaterialID(guard, physid);
    }
    
    return true;
}

void Leviathan::Entity::Prop::_SaveOwnDataToPacket(Lock &guard, sf::Packet &packet){
    
    // The hidden state needs to be the first thing
    packet << Hidden;
    
    // Before adding our data make base classes add stuff //
    AddPositionAndRotationToPacket(guard, packet);

    // Physics state //
    AddPhysicalStateToPacket(guard, packet);
    
    packet << ModelFile;

    packet << (GetPhysicsBody(guard) ? true: false);

    // Add the mass if it is applicable //
    if(GetPhysicsBody(guard)){
        
        packet << AppliedPhysicalMaterial;
    }
}
// ------------------------------------ //
void Leviathan::Entity::Prop::_SendCreatedConstraint(BaseConstraintable* other,
    Entity::BaseConstraint* ptr)
{
    _SendNewConstraint(static_cast<BaseConstraintable*>(this), other, ptr);
}
// ------------------------------------ //
BaseConstraintable* Leviathan::Entity::Prop::BasePhysicsGetConstraintable(){
    return static_cast<BaseConstraintable*>(this);
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<ObjectDeltaStateData> Leviathan::Entity::Prop::CaptureState(Lock &guard,
    int tick)
{
    return std::shared_ptr<ObjectDeltaStateData>(
        PositionableRotationableDeltaState::CaptureState(guard, *this, tick).release());
}

DLLEXPORT std::shared_ptr<ObjectDeltaStateData> Leviathan::Entity::Prop::CreateStateFromPacket(
    int tick, sf::Packet &packet)
    const
{
    
    try{
        return make_shared<PositionableRotationableDeltaState>(tick, packet);
        
    } catch(const Exception &e){

        Logger::Get()->Warning("Prop: failed to CreateStateFromPacket, exception:");
        e.PrintToLog();
        return nullptr;
    }
    
}
// ------------------------------------ //
void Prop::_OnNewStateReceived(){

    if(!ListeningForEvents){
            
        RegisterForEvent(EVENT_TYPE_CLIENT_INTERPOLATION);
        ListeningForEvents = true;
    }
}

DLLEXPORT int Prop::OnEvent(Event** pEvent){

    if((*pEvent)->GetType() == EVENT_TYPE_CLIENT_INTERPOLATION){
        
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

DLLEXPORT int Prop::OnGenericEvent(GenericEvent** pevent){
    return -1;
}

