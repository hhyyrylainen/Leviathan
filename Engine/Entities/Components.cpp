// ------------------------------------ //
#include "Components.h"

#include "CommonStateObjects.h"

#include "GameWorld.h"
#include "Networking/Connection.h"
#include "Networking/SentNetworkThing.h"

#include "OgreSceneManager.h"
#include "OgreBillboardChain.h"
#include "OgreRibbonTrail.h"
#include "OgreItem.h"

#include <limits>
using namespace Leviathan;
// ------------------------------------ //

// ------------------ RenderNode ------------------ //
DLLEXPORT void RenderNode::Release(Ogre::SceneManager* worldsscene){

    worldsscene->destroySceneNode(Node);
    Node = nullptr;
}

// // ------------------------------------ //
// // Plane
// DLLEXPORT Plane::Plane(Ogre::SceneNode* parent, const std::string &material) :
//     Component(TYPE)
// {
        
// }

// //! \brief Destroys GraphicalObject
// DLLEXPORT void Plane::Release(Ogre::SceneManager* scene){

// }

// ------------------ Physics ------------------ //
DLLEXPORT void Physics::JumpTo(Position &target){

    
    SetPosition(target.Members._Position, target.Members._Orientation);
}

DLLEXPORT bool Physics::SetPosition(const Float3 &pos, const Float4 &orientation){

    if(!Body)
        return false;
    
    Ogre::Matrix4 matrix;

    Ogre::Vector3 ogrepos = pos;
    Ogre::Quaternion ogrerot = orientation;
    matrix.makeTransform(ogrepos, Float3(1, 1, 1), ogrerot);

    Ogre::Matrix4 tmatrix = matrix.transpose();

    // Update body //
    NewtonBodySetMatrix(Body, &tmatrix[0][0]);

    return true;
}

void Physics::PhysicsMovedEvent(const NewtonBody* const body,
    const dFloat* const matrix, int threadIndex)
{

	// first create Ogre 4x4 matrix from the matrix //
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5],
        matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12],
        matrix[13], matrix[14], matrix[15]);

	// needs to convert from d3d style matrix to OpenGL style matrix //
    // TODO: do this transpose in the mat constructor
	Ogre::Matrix4 tmat = mat.transpose();

	Physics* tmp = reinterpret_cast<Physics*>(NewtonBodyGetUserData(body));

    tmp->_Position.Members._Position = tmat.getTrans();
    tmp->_Position.Members._Orientation = tmat.extractQuaternion();
    tmp->_Position.Marked = true;
    
    if(tmp->UpdateSendable){
        
        tmp->UpdateSendable->Marked = true;
    }
    
    tmp->Marked = true;
}

void Physics::ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat
    timestep, int threadIndex)
{
	// Get object from body //
	Physics* tmp = reinterpret_cast<Physics*>(NewtonBodyGetUserData(body));
    
	// Check if physics can't apply //
    // Newton won't call this if the mass is 0

	Float3 Torque(0, 0, 0);

	// Get properties from newton //
	float mass; 
	float Ixx; 
	float Iyy; 
	float Izz; 

	NewtonBodyGetMass(body, &mass, &Ixx, &Iyy, &Izz);

	// get gravity force and apply mass to it //
	Float3 Force = tmp->World->GetGravityAtPosition(tmp->_Position.Members._Position)*mass;
    
	// add other forces //
    if(!tmp->ApplyForceList.empty()){
        
        Force += tmp->_GatherApplyForces(mass);
    }
    
	NewtonBodyAddForce(body, &Force.X);
	NewtonBodyAddTorque(body, &Torque.X);
}

void Physics::DestroyBodyCallback(const NewtonBody* body){
    // This shouldn't be required as the newton world won't be cleared while running
    // Physics* tmp = reinterpret_cast<Physics*>(NewtonBodyGetUserData(body));

    // GUARD_LOCK_OTHER(tmp);
    
    // tmp->Body = nullptr;
    // tmp->Collision = nullptr;
}
// ------------------------------------ //
DLLEXPORT void Physics::GiveImpulse(const Float3 &deltaspeed, const Float3 &point
    /*= Float3(0)*/)
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    // No clue what the delta step should be
    NewtonBodyAddImpulse(Body, &deltaspeed.X, &point.X, 0.1f);
}

DLLEXPORT void Physics::SetVelocity(const Float3 &velocities){
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    NewtonBodySetVelocity(Body, &velocities.X);
}

DLLEXPORT Float3 Physics::GetVelocity() const{

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    Float3 vel(0);
    NewtonBodyGetVelocity(Body, &vel.X);
    return vel;
}

DLLEXPORT Float3 Physics::GetTorque(){

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    Float3 torq(0);
    NewtonBodyGetTorque(Body, &torq.X);
    return torq;
}

DLLEXPORT void Physics::SetTorque(const Float3 &torque){
    
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    NewtonBodySetTorque(Body, &torque.X);
}

DLLEXPORT void Physics::SetLinearDampening(float factor /*= 0.1f*/){
    

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    NewtonBodySetLinearDamping(Body, factor);
}
// ------------------------------------ //
Float3 Physics::_GatherApplyForces(const float &mass){
	// Return if just an empty list //
	if(ApplyForceList.empty())
		return Float3(0);

	Float3 total(0);

	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter){
		// Add to total, and multiply by mass if wanted //
        const Float3 force = (*iter)->Callback((*iter).get(), *this);
        
		total += (*iter)->MultiplyByMass ? force*mass: force;

        // We might assert/crash here if the force was removed //
	}

	return total;
}
// ------------------------------------ //
DLLEXPORT void Physics::Release(){

    if(Collision)
		NewtonDestroyCollision(Collision);
	if(Body)
		NewtonDestroyBody(Body);
    
	Body = NULL;
	Collision = NULL;
}
// ------------------------------------ //
DLLEXPORT void Physics::ApplyForce(ApplyForceInfo* pointertohandle){
    
	// Overwrite old if found //
	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter){
        
		// Check do the names match //
		if((bool)(*iter)->OptionalName == (bool)pointertohandle->OptionalName){

			if(!pointertohandle->OptionalName ||
                *pointertohandle->OptionalName == *(*iter)->OptionalName)
            {
				// it's the default, overwrite //
				**iter = *pointertohandle;
				SAFE_DELETE(pointertohandle);
				return;
			}
		}
	}
	// got here, so add a new one //
	ApplyForceList.push_back(std::shared_ptr<ApplyForceInfo>(pointertohandle));
}

DLLEXPORT bool Physics::RemoveApplyForce(const std::string &name){
    
    
	// Search for a matching name //
    auto end = ApplyForceList.end();
	for(auto iter = ApplyForceList.begin(); iter != end; ++iter){
        
		// Check do the names match //
		if((!((*iter)->OptionalName) && name.size() == 0) || ((*iter)->OptionalName &&
                *(*iter)->OptionalName == name))
        {
            
			ApplyForceList.erase(iter);
			return true;
		}
	}

	return false;
}
// ------------------------------------ //
DLLEXPORT void Physics::SetPhysicalMaterialID(int ID){
    if(!Body){
        throw InvalidState("Calling set material ID without having physical Body");
    }
    
	NewtonBodySetMaterialGroupID(Body, ID);
}
// ------------------ Received ------------------ //
DLLEXPORT void Received::GetServerSentStates(StoredState const** first,
    StoredState const** second, int tick, float &progress) const
{
    // Used to find the first tick before or on tick //
    int firstinpast = std::numeric_limits<int>::max();
    int secondfound = 0;

    for(auto& obj : ClientStateBuffer){

        if(tick - obj.Tick < firstinpast && tick - obj.Tick >= 0){

            // This is (potentially) the first state //
            firstinpast = tick - obj.Tick;

            *first = &obj;
        }

        // For this to be found the client should be around 50-100 milliseconds in the past
        if(obj.Tick > tick && (secondfound == 0 || obj.Tick - tick < secondfound)){

            // The second state //
            *second = &obj;
            
            secondfound = obj.Tick-tick;
            continue;
        }


    }

    if(firstinpast == std::numeric_limits<int>::max() || secondfound == 0){

        throw InvalidState("No stored server states around tick");
    }

    // If the range is not 1, meaning firstinpast != 0 || secondfound > 1 we need to adjust
    // progress
    int range = firstinpast + secondfound;

    if(range == 1)
        return;

    progress = ((tick + progress) - (*first)->Tick) / range;
}
// // ------------------ Trail ------------------ //
// DLLEXPORT bool Trail::SetTrailProperties(const Properties &variables, bool force /*= false*/){

//     

//     if(!TrailEntity || !_RenderNode)
//         return false;

//     // Set if we unconnected the node and we should reconnect it afterwards //
// 	bool ConnectAgain = false;

// 	// Determine if we need to unconnect the node //
// 	if(force || variables.MaxChainElements != CurrentSettings.MaxChainElements){

// 		// This to avoid Ogre bug //
// 		TrailEntity->removeNode(_RenderNode->Node);
// 		ConnectAgain = true;

// 		// Apply the properties //
// 		TrailEntity->setUseVertexColours(true);
// 		TrailEntity->setRenderingDistance(variables.MaxDistance);
// 		TrailEntity->setMaxChainElements(variables.MaxChainElements);
// 		TrailEntity->setCastShadows(variables.CastShadows);
// 		TrailEntity->setTrailLength(variables.TrailLenght);
// 	}

// 	// Update cached settings //
// 	CurrentSettings = variables;

// 	// Apply per element properties //
// 	for(size_t i = 0; i < variables.Elements.size(); i++){
// 		// Apply settings //
// 		const ElementProperties& tmp = variables.Elements[i];

//         TrailEntity->setInitialColour(i, tmp.InitialColour);
//         TrailEntity->setInitialWidth(i, tmp.InitialSize);
//         TrailEntity->setColourChange(i, tmp.ColourChange);
//         TrailEntity->setWidthChange(i, tmp.SizeChange);
// 	}

// 	// More bug avoiding //
// 	if(ConnectAgain)	
// 		TrailEntity->addNode(_RenderNode->Node);

// 	return true;
// }
// // ------------------------------------ //
// DLLEXPORT void Trail::Release(Ogre::SceneManager* scene){

//     if(TrailEntity){

//         scene->destroyRibbonTrail(TrailEntity);
//         TrailEntity = nullptr;
//     }
// }
// ------------------ Sendable ------------------ //
DLLEXPORT void Sendable::ActiveConnection::CheckReceivedPackets(){

    if(SentPackets.empty())
        return;

    // Looped in reverse to hopefully remove only last elements //
    for(int i = static_cast<int>(SentPackets.size() - 1); i >= 0; ){

        const auto& tuple = SentPackets[i];

        if(std::get<2>(tuple)->IsFinalized()){

            if(std::get<2>(tuple)->GetStatus()){

                // Succeeded //
                if(std::get<0>(tuple) > LastConfirmedTickNumber){

                    LastConfirmedTickNumber = std::get<0>(tuple);
                    LastConfirmedData = std::get<1>(tuple);
                }
            }

            SentPackets.erase(SentPackets.begin()+i);

            if(SentPackets.empty())
                break;
            
        } else {

            i--;
        }
    }

    if(SentPackets.capacity() > 10){

        Logger::Get()->Warning("Sendable::ActiveConnection: SentPackets has space for over 10 "
            "sent packets");
        SentPackets.shrink_to_fit();
    }
}
// ------------------------------------ //
DLLEXPORT Model::Model(Ogre::SceneManager* scene, Ogre::SceneNode* parent,
    const std::string &meshname) :
    Component(TYPE)
{
    GraphicalObject = scene->createItem(meshname);
    parent->attachObject(GraphicalObject);
}

DLLEXPORT void Model::Release(Ogre::SceneManager* scene){

    if(GraphicalObject){

        scene->destroyItem(GraphicalObject);
        GraphicalObject = nullptr;
    }
}

// ------------------ ManualObject ------------------ //
DLLEXPORT void ManualObject::Release(Ogre::SceneManager* scene){

    if(Object){

        scene->destroyManualObject(Object);
        Object = nullptr;
    }

    CreatedMesh.clear();
}



