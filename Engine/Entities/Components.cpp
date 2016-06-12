// ------------------------------------ //
#include "Components.h"

#include "OgreSceneManager.h"
#include "OgreBillboardChain.h"
#include "OgreRibbonTrail.h"
#include "CommonStateObjects.h"
#include "GameWorld.h"
#include "../Networking/ConnectionInfo.h"
#include <limits>
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

// ------------------ Position ------------------ //
DLLEXPORT Position::Position(const Float3 pos, const Float4 rot) :
    _Position(pos), _Orientation(rot)
{
    Marked = true;
}
// ------------------------------------ //
DLLEXPORT void Position::ApplyPositionData(const PositionData &data){

    _Position = data._Position;
    _Orientation = data._Orientation;

    Marked = true;
}

DLLEXPORT void Position::LoadDataFromPacket(sf::Packet &packet, PositionData &data){

    packet >> data._Position >> data._Orientation;

    if(!packet)
        throw InvalidArgument("Packet has invalid format");
}
// ------------------------------------ //
DLLEXPORT void Position::AddDataToPacket(sf::Packet &packet) const{

    packet << _Position << _Orientation;
}

DLLEXPORT void Position::ApplyDataFromPacket(sf::Packet &packet){

    packet >> _Position >> _Orientation;

    if(!packet)
        throw InvalidArgument("Packet has invalid format");

    Marked = true;
}
// ------------------------------------ //
DLLEXPORT void Position::Interpolate(const PositionDeltaState &from, const PositionDeltaState &to,
    float progress)
{
    Float3 pos = _Position;
    Float4 rot = _Orientation;

    // Position
    if(to.ValidFields & PRDELTAUPDATED_POS_X){

        if(from.ValidFields & PRDELTAUPDATED_POS_X){
            
            pos.X = from.Position.X + (to.Position.X-from.Position.X)*progress;
        } else {

            pos.X = to.Position.X;
        }
        
    } else if(from.ValidFields & PRDELTAUPDATED_POS_X){
        
        pos.X = from.Position.X;
    }

    if(to.ValidFields & PRDELTAUPDATED_POS_Y){

        if(from.ValidFields & PRDELTAUPDATED_POS_Y){
            
            pos.Y = from.Position.Y + (to.Position.Y-from.Position.Y)*progress;
        } else {

            pos.Y = to.Position.Y;
        }
        
    } else if(from.ValidFields & PRDELTAUPDATED_POS_Y){
        
        pos.Y = from.Position.Y;
    }
    
    if(to.ValidFields & PRDELTAUPDATED_POS_Z){

        if(from.ValidFields & PRDELTAUPDATED_POS_Z){
            
            pos.Z = from.Position.Z + (to.Position.Z-from.Position.Z)*progress;
        } else {

            pos.Z = to.Position.Z;
        }
        
    } else if(from.ValidFields & PRDELTAUPDATED_POS_Z){
        
        pos.Z = from.Position.Z;
    }
    
    // Rotation
    // TODO: spherical interpolation for rotation
    if(to.ValidFields & PRDELTAUPDATED_ROT_X){

        if(from.ValidFields & PRDELTAUPDATED_ROT_X){
            
            rot.X = from.Rotation.X + (to.Rotation.X-from.Rotation.X)*progress;
        } else {

            rot.X = to.Rotation.X;
        }
        
    } else if(from.ValidFields & PRDELTAUPDATED_ROT_X){

        rot.X = from.Rotation.X;
    }

    if(to.ValidFields & PRDELTAUPDATED_ROT_Y){

        if(from.ValidFields & PRDELTAUPDATED_ROT_Y){
            
            rot.Y = from.Rotation.Y + (to.Rotation.Y-from.Rotation.Y)*progress;
        } else {

            rot.Y = to.Rotation.Y;
        }
        
    } else if(from.ValidFields & PRDELTAUPDATED_ROT_Y){

        rot.Y = from.Rotation.Y;
    }
    
    if(to.ValidFields & PRDELTAUPDATED_ROT_Z){

        if(from.ValidFields & PRDELTAUPDATED_ROT_Z){
            
            rot.Z = from.Rotation.Z + (to.Rotation.Z-from.Rotation.Z)*progress;
        } else {

            rot.Z = to.Rotation.Z;
        }
        
    } else if(from.ValidFields & PRDELTAUPDATED_ROT_Z){

        rot.Z = from.Rotation.Z;
    }
    
    if(to.ValidFields & PRDELTAUPDATED_ROT_W){

        if(from.ValidFields & PRDELTAUPDATED_ROT_W){
            
            rot.W = from.Rotation.W + (to.Rotation.W-from.Rotation.W)*progress;
        } else {

            rot.W = to.Rotation.W;
        }
        
    } else if(from.ValidFields & PRDELTAUPDATED_ROT_W){

        rot.W = from.Rotation.W;
    }
    
    _Position = pos;
    _Orientation = rot;

    Marked = true;
}
// ------------------ RenderNode ------------------ //
DLLEXPORT RenderNode::RenderNode(){

    Marked = false;
}

DLLEXPORT bool RenderNode::Init(){

    Node = nullptr;
    return true;
}

DLLEXPORT void RenderNode::Release(Ogre::SceneManager* worldsscene){

    worldsscene->destroySceneNode(Node);
    Node = nullptr;
}


// ------------------ Physics ------------------ //
DLLEXPORT Physics::ApplyForceInfo::ApplyForceInfo(bool addmass,
    std::function<Float3(ApplyForceInfo* instance, Physics &object, Lock &objectlock)> getforce,
    std::unique_ptr<std::string> name) :
    OptionalName(move(name)), MultiplyByMass(addmass), Callback(getforce)
{

}
        
DLLEXPORT Physics::ApplyForceInfo::ApplyForceInfo(const ApplyForceInfo &other) :
    MultiplyByMass(other.MultiplyByMass), Callback(other.Callback)
{
    if(other.OptionalName)
        OptionalName = move(make_unique<string>(*other.OptionalName));
}

DLLEXPORT Physics::ApplyForceInfo::ApplyForceInfo(ApplyForceInfo &&other) :
    OptionalName(move(other.OptionalName)), MultiplyByMass(move(other.MultiplyByMass)),
    Callback(move(other.Callback))
{
    
}

DLLEXPORT Physics::ApplyForceInfo& Physics::ApplyForceInfo::operator =(
    const Physics::ApplyForceInfo &other){

    if(other.OptionalName)
        OptionalName = move(make_unique<string>(*other.OptionalName));

    MultiplyByMass = other.MultiplyByMass;
    Callback = other.Callback;

    return *this;
}
// ------------------------------------ //
DLLEXPORT Physics::Physics(const Arguments &args) :
    World(args.world), _Position(args.updatepos), UpdateSendable(args.updatesendable), Mass(0.f),
    ApplyGravity(true), AppliedPhysicalMaterial(0), Collision(nullptr), Body(nullptr),
    ThisEntity(args.id)
{

}
// ------------------------------------ //
DLLEXPORT void Physics::JumpTo(Position &target){

    GUARD_LOCK();
    SetPosition(guard, target._Position, target._Orientation);
}

DLLEXPORT bool Physics::SetPosition(Lock &guard, const Float3 &pos, const Float4 &orientation){

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
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6],
        matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13],
        matrix[14], matrix[15]);

	// needs to convert from d3d style matrix to OpenGL style matrix //
    // TODO: do this transpose in the mat constructor
	Ogre::Matrix4 tmat = mat.transpose();

	Physics* tmp = reinterpret_cast<Physics*>(NewtonBodyGetUserData(body));

    // The object needs to be locked here //
    GUARD_LOCK_OTHER(tmp);
    
    GUARD_LOCK_OTHER_NAME((&tmp->_Position), guard2);

    tmp->_Position._Position = tmat.getTrans();
    tmp->_Position._Orientation = tmat.extractQuaternion();
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

	NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);

    GUARD_LOCK_OTHER(tmp);

	// get gravity force and apply mass to it //
	Float3 Force = tmp->World->GetGravityAtPosition(tmp->_Position._Position)*mass;
    
	// add other forces //
    if(!tmp->ApplyForceList.empty())
        Force += tmp->_GatherApplyForces(guard, mass);
    

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
DLLEXPORT void Physics::GiveImpulse(const Float3 &deltaspeed, const Float3 &point /*= Float3(0)*/){
    GUARD_LOCK();

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
		NewtonBodyAddImpulse(Body, &deltaspeed.X, &point.X);
}

DLLEXPORT void Physics::SetVelocity(Lock &guard, const Float3 &velocities){
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    NewtonBodySetVelocity(Body, &velocities.X);
}

DLLEXPORT Float3 Physics::GetVelocity(Lock &guard) const{

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    Float3 vel(0);
    NewtonBodyGetVelocity(Body, &vel.X);
    return vel;
}

DLLEXPORT Float3 Physics::GetTorque(Lock &guard){

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    Float3 torq(0);
    NewtonBodyGetTorque(Body, &torq.X);
    return torq;
}

DLLEXPORT void Physics::SetTorque(Lock &guard, const Float3 &torque){
    
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    NewtonBodySetTorque(Body, &torque.X);
}

DLLEXPORT void Physics::SetLinearDampening(float factor /*= 0.1f*/){
    GUARD_LOCK();

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    NewtonBodySetLinearDamping(Body, factor);
}
// ------------------------------------ //
DLLEXPORT NewtonBody* Physics::GetBody() const{

    return Body;
}
// ------------------------------------ //
Float3 Physics::_GatherApplyForces(Lock &guard, const float &mass){
	// Return if just an empty list //
	if(ApplyForceList.empty())
		return Float3(0);

	Float3 total(0);

	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter){
		// Add to total, and multiply by mass if wanted //
        const Float3 force = (*iter)->Callback((*iter).get(), *this, guard);
        
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
    GUARD_LOCK();
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
	ApplyForceList.push_back(shared_ptr<ApplyForceInfo>(pointertohandle));
}

DLLEXPORT bool Physics::RemoveApplyForce(const std::string &name){
    GUARD_LOCK();
    
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
DLLEXPORT void Physics::SetPhysicalMaterialID(Lock &guard, int ID){

    if(!Body)
        throw InvalidState("Calling set material ID without having physical Body");

	NewtonBodySetMaterialGroupID(Body, ID);
}
// ------------------------------------ //
DLLEXPORT void Physics::InterpolatePhysicalState(PhysicalDeltaState &first,
    PhysicalDeltaState &second, float progress)
{

    // TODO: fix using partial first states
    DEBUG_BREAK;
    
    if(progress < 0.f)
        progress = 0.f;

    if(progress > 1.f)
        progress = 1.f;

    GUARD_LOCK();
    
    Float3 pos = _Position._Position;
    Float4 rot = _Position._Orientation;
    Float3 vel = GetVelocity(guard);
    Float3 tor = GetTorque(guard);

    // First check does the second state have a changed value for each component
    // If it does interpolate from the first state, which should have almost all values, or if it doesn't error
    // so the sendable class can properly fill the states (this should be never the case as long as sendable class is
    // correct)
    // And if it doesn't snap to the value of the first state, which should have been the end state of an interpolation
    // that was before this

    // Position
    if(second.ValidFields & PPDELTAUPDATED_POS_X){

        pos.X = first.Position.X + (second.Position.X-first.Position.X)*progress;
        
    } else {

        pos.X = first.Position.X;
    }

    if(second.ValidFields & PPDELTAUPDATED_POS_Y){

        pos.Y = first.Position.Y + (second.Position.Y-first.Position.Y)*progress;
        
    } else {

        pos.Y = first.Position.Y;
    }

    if(second.ValidFields & PPDELTAUPDATED_POS_Z){

        pos.Z = first.Position.Z + (second.Position.Z-first.Position.Z)*progress;
        
    } else {

        pos.Z = first.Position.Z;
    }

    // Rotation
    // TODO: spherical interpolation for rotation
    if(second.ValidFields & PPDELTAUPDATED_ROT_X){

        rot.X = first.Rotation.X + (second.Rotation.X-first.Rotation.X)*progress;
        
    } else {

        rot.X = first.Rotation.X;
    }

    if(second.ValidFields & PPDELTAUPDATED_ROT_Y){

        rot.Y = first.Rotation.Y + (second.Rotation.Y-first.Rotation.Y)*progress;
        
    } else {

        rot.Y = first.Rotation.Y;
    }

    if(second.ValidFields & PPDELTAUPDATED_ROT_Z){

        rot.Z = first.Rotation.Z + (second.Rotation.Z-first.Rotation.Z)*progress;
        
    } else {

        rot.Z = first.Rotation.Z;
    }

    if(second.ValidFields & PPDELTAUPDATED_ROT_W){

        rot.W = first.Rotation.W + (second.Rotation.W-first.Rotation.W)*progress;
        
    } else {

        rot.W = first.Rotation.W;
    }

    // Velocity
    if(second.ValidFields & PPDELTAUPDATED_VEL_X){

        vel.X = first.Velocity.X + (second.Velocity.X-first.Velocity.X)*progress;
        
    } else {

        vel.X = first.Velocity.X;
    }

    if(second.ValidFields & PPDELTAUPDATED_VEL_Y){

        vel.Y = first.Velocity.Y + (second.Velocity.Y-first.Velocity.Y)*progress;
        
    } else {

        vel.Y = first.Velocity.Y;
    }

    if(second.ValidFields & PPDELTAUPDATED_VEL_Z){

        vel.Z = first.Velocity.Z + (second.Velocity.Z-first.Velocity.Z)*progress;
        
    } else {

        vel.Z = first.Velocity.Z;
    }

    // Torque
    if(second.ValidFields & PPDELTAUPDATED_TOR_X){

        tor.X = first.Torque.X + (second.Torque.X-first.Torque.X)*progress;
        
    } else {

        tor.X = first.Torque.X;
    }

    if(second.ValidFields & PPDELTAUPDATED_TOR_Y){

        tor.Y = first.Torque.Y + (second.Torque.Y-first.Torque.Y)*progress;
        
    } else {

        tor.Y = first.Torque.Y;
    }

    if(second.ValidFields & PPDELTAUPDATED_TOR_Z){

        tor.Z = first.Torque.Z + (second.Torque.Z-first.Torque.Z)*progress;
        
    } else {

        tor.Z = first.Torque.Z;
    }

    _Position._Position = pos;
    _Position._Orientation = rot;

    _Position.Marked = true;
    
    SetVelocity(guard, vel);
    SetTorque(guard, tor);
}
// ------------------ Received ------------------ //
Received::StoredState::StoredState(std::shared_ptr<ObjectDeltaStateData> safedata,
    void* data, SENDABLE_TYPE datatype) :
    Tick(safedata->Tick), DeltaData(safedata), DirectData(data), OwnersType(datatype)
{

}
// ------------------------------------ //
DLLEXPORT Received::Received(SENDABLE_TYPE type) :
    SendableHandleType(type), ClientStateBuffer(BASESENDABLE_STORED_RECEIVED_STATES)
{

}
// ------------------------------------ //
DLLEXPORT void Received::GetServerSentStates(Lock &guard, StoredState const** first,
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

// ------------------ TrackController ------------------ //
DLLEXPORT TrackController::TrackController(PositionMarkerOwner& nodes, Sendable* sendable) :
    ForceTowardsPoint(0.3f), ReachedNode(0), NodeProgress(0.f), ChangeSpeed(0.f),
    _Sendable(sendable), Nodes(nodes)
{

}

DLLEXPORT TrackController::TrackController(const Arguments &args) :
    ReachedNode(args.ReachedNode), NodeProgress(args.NodeProgress),
    ChangeSpeed(args.ChangeSpeed), ForceTowardsPoint(args.ForceTowardsPoint),
    _Sendable(args._Sendable), Nodes(args.Nodes)
{

}
// ------------------------------------ //
DLLEXPORT void TrackController::Update(float timestep){

    if(ChangeSpeed == 0.f)
        return;

    NodeProgress += timestep * ChangeSpeed;

    if(NodeProgress < 0.f){

        int wholeunder = floor(NodeProgress);

        float singlenodepart = NodeProgress + wholeunder;

        ReachedNode -= wholeunder;

        if(ReachedNode < 0)
            ReachedNode = 0;

        NodeProgress = 1.f-singlenodepart;
        
    } else if(NodeProgress >= 1.f){

        int wholeover = floor(NodeProgress);

        float singlenodepart = NodeProgress - wholeover;

        ReachedNode += wholeover;

        if(ReachedNode >= static_cast<int>(Nodes.Markers.size()))
            ReachedNode = Nodes.Markers.size()-1;

        NodeProgress = singlenodepart;
    }
}
// ------------------------------------ //
void TrackController::_SanityCheckNodeProgress(Lock &guard){

    // Should work for NaN checks
    if(NodeProgress != NodeProgress)
        NodeProgress = 0.f; 
    
    if(ReachedNode < 0)
        ReachedNode = 0;

    if(ReachedNode >= static_cast<int>(Nodes.Markers.size()))
        ReachedNode = Nodes.Markers.size()-1;
}
// ------------------------------------ //
DLLEXPORT bool TrackController::GetNodePosition(int index, Float3 &pos, Float4 &rot) const{
    // Lock to avoid getting markers changed while being called
    GUARD_LOCK();
    
    if(index < 0 || index >= static_cast<int>(Nodes.Markers.size())){

        return false;
    }

    pos = get<1>(Nodes.Markers[index])->_Position;
    rot = get<1>(Nodes.Markers[index])->_Orientation;

    return true;
}
// ------------------------------------ //
DLLEXPORT void TrackController::GetPositionOnTrack(Float3 &pos, Float4 &rot) const{

    const auto& node = Nodes.Markers[ReachedNode];

    const auto& nodepos = std::get<1>(node);

    pos = nodepos->_Position;
    rot = nodepos->_Orientation;
}
// ------------------------------------ //
DLLEXPORT bool TrackController::Interpolate(ObjectDeltaStateData &first,
    ObjectDeltaStateData &second, float progress)
{

    const TrackControllerState& from = static_cast<const TrackControllerState&>(first);
    const TrackControllerState& to = static_cast<const TrackControllerState&>(second);

    if(progress < 0.f)
        progress = 0.f;

    if(progress > 1.f)
        progress = 1.f;
    
    GUARD_LOCK();

    if(to.ValidFields & TRACKSTATE_UPDATED_SPEED){

        ChangeSpeed = from.ChangeSpeed*(1.f-progress) + progress*to.ChangeSpeed;
        
    } else {

        ChangeSpeed = from.ChangeSpeed;
    }

    if(to.ValidFields & TRACKSTATE_UPDATED_NODE && from.ReachedNode != to.ReachedNode){

        // Node has changed //
        const float fromtotalvalue = from.ReachedNode+from.NodeProgress;

        const float tototalvalue = to.ReachedNode +
            (to.ValidFields & TRACKSTATE_UPDATED_PROGRESS ? to.NodeProgress : 0);

        const float mixed = fromtotalvalue*(1.f-progress) + tototalvalue*progress;

        ReachedNode = floor(mixed);
        NodeProgress = mixed-ReachedNode;
        
    } else {

        ReachedNode = from.ReachedNode;

        if(to.ValidFields & TRACKSTATE_UPDATED_PROGRESS){
            
            NodeProgress = from.NodeProgress*(1.f-progress) + progress*to.NodeProgress;
            
        } else {

            NodeProgress = from.NodeProgress;
        }
    }

    _SanityCheckNodeProgress(guard);
    
    return true;
}
// ------------------ Trail ------------------ //
DLLEXPORT Trail::Trail(RenderNode* node, const std::string &materialname,
    const Properties &variables) :
    TrailEntity(nullptr), _RenderNode(node), CurrentSettings(variables), Material(materialname)
{

}

DLLEXPORT bool Trail::SetTrailProperties(const Properties &variables, bool force /*= false*/){

    GUARD_LOCK();

    if(!TrailEntity || !_RenderNode)
        return false;

    // Set if we unconnected the node and we should reconnect it afterwards //
	bool ConnectAgain = false;

	// Determine if we need to unconnect the node //
	if(force || variables.MaxChainElements != CurrentSettings.MaxChainElements){

		// This to avoid Ogre bug //
		TrailEntity->removeNode(_RenderNode->Node);
		ConnectAgain = true;

		// Apply the properties //
		TrailEntity->setUseVertexColours(true);
		TrailEntity->setRenderingDistance(variables.MaxDistance);
		TrailEntity->setMaxChainElements(variables.MaxChainElements);
		TrailEntity->setCastShadows(variables.CastShadows);
		TrailEntity->setTrailLength(variables.TrailLenght);
	}

	// Update cached settings //
	CurrentSettings = variables;

	// Apply per element properties //
	for(size_t i = 0; i < variables.Elements.size(); i++){
		// Apply settings //
		const ElementProperties& tmp = variables.Elements[i];

        TrailEntity->setInitialColour(i, tmp.InitialColour);
        TrailEntity->setInitialWidth(i, tmp.InitialSize);
        TrailEntity->setColourChange(i, tmp.ColourChange);
        TrailEntity->setWidthChange(i, tmp.SizeChange);
	}

	// More bug avoiding //
	if(ConnectAgain)	
		TrailEntity->addNode(_RenderNode->Node);

	return true;
}
// ------------------------------------ //
DLLEXPORT void Trail::Release(Ogre::SceneManager* scene){

    if(TrailEntity){

        scene->destroyRibbonTrail(TrailEntity);
        TrailEntity = nullptr;
    }
}
// ------------------ Parent ------------------ //
DLLEXPORT Parent::Parent(){

}

DLLEXPORT Parent::Parent(const Data &data, GameWorld* world, Lock &worldlock){

    Children.reserve(data.EntityIDs.size());

    for(auto&& id : data.EntityIDs){

        // TODO: find already received ids from the world
        Children.push_back(make_tuple(id, nullptr));
    }
}
// ------------------------------------ //
DLLEXPORT void Parent::RemoveChildNoNotify(Parentable* which){

    GUARD_LOCK();
    for(auto iter = Children.begin(); iter != Children.end(); ++iter){

        if(get<1>(*iter) == which){

            Children.erase(iter);
            return;
        }
    }
}

DLLEXPORT void Parent::RemoveChildren(){

    GUARD_LOCK();

    for(auto& tuple : Children){

        get<1>(tuple)->OnParentInvalidate();
    }
    
    Children.clear();
}

DLLEXPORT void Parent::AddChild(ObjectID childid, Parentable &child){

    GUARD_LOCK();

    Children.push_back(make_tuple(childid, &child));
}

DLLEXPORT void Parent::AddDataToPacket(sf::Packet &packet) const{

    GUARD_LOCK();

    const auto size = static_cast<int32_t>(Children.size());
    
    packet << size;

    for(int32_t i = 0; i < size; i++){

        packet << std::get<0>(Children[i]);
    }
}
// ------------------------------------ //
DLLEXPORT Parent::Data Parent::LoadDataFromPacket(sf::Packet &packet){

    Data data;

    int32_t size;

    packet >> size;

    if(!packet)
        return data;

    data.EntityIDs.reserve(size);

    for(int32_t i = 0; i < size; i++){

        ObjectID id;

        packet >> id;
        
        data.EntityIDs.push_back(id);
    }

    return data;
}
// ------------------ Parentable ------------------ //
DLLEXPORT Parentable::Parentable() :
    ApplyRotation(true), RelativeToParent(0), AttachedParent(nullptr)
{

}

DLLEXPORT void Parentable::DetachFromParent(){

    GUARD_LOCK();

    AttachedParent = nullptr;
}

DLLEXPORT void Parentable::OnParentInvalidate(){

    GUARD_LOCK();

    if(AttachedParent)
        AttachedParent->RemoveChildNoNotify(this);

    AttachedParent = nullptr;
}
// ------------------ Sendable ------------------ //
DLLEXPORT void Sendable::ActiveConnection::CheckReceivedPackets(){

    if(SentPackets.empty())
        return;

    // Looped in reverse to hopefully remove only last elements //
    for(int i = SentPackets.size()-1; i >= 0; ){

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

DLLEXPORT void Sendable::ActiveConnection::AddSentPacket(int tick,
    std::shared_ptr<ObjectDeltaStateData> state, std::shared_ptr<SentNetworkThing> packet)
{
    SentPackets.push_back(make_tuple(tick, state, packet));
}
Sendable::ActiveConnection::ActiveConnection(ConnectionInfo* connection) :
    CorrespondingConnection(connection), LastConfirmedTickNumber(-1)
{

}

DLLEXPORT Sendable::Sendable(SENDABLE_TYPE type) : SendableHandleType(type)
{

}

DLLEXPORT void Sendable::AddConnectionToReceivers(Lock &guard, ConnectionInfo* connection){

    UpdateReceivers.push_back(make_shared<ActiveConnection>(connection));
}
// ------------------ BoxGeometry ------------------ //
DLLEXPORT BoxGeometry::BoxGeometry(const Float3 &size, const std::string &material) :
    Sizes(size), Material(material), GraphicalObject(nullptr)
{

}
// ------------------ PositionMarkerOwner ------------------ //
DLLEXPORT PositionMarkerOwner::PositionMarkerOwner(){

}

DLLEXPORT PositionMarkerOwner::PositionMarkerOwner(const Data &positions, GameWorld* world,
    Lock &worldlock)
{

    Markers.reserve(positions.EntityPositions.size());

    for(auto&& tuple : positions.EntityPositions){

        ObjectID marker = get<0>(tuple);

        auto& pos = world->CreatePosition(marker, get<1>(tuple), get<2>(tuple));

        Markers.push_back(make_tuple(marker, &pos));
    }
}
// ------------------------------------ //
DLLEXPORT void PositionMarkerOwner::Release(GameWorld* world, Lock &worldlock){

    for(auto&& tuple : Markers){

        auto id = get<0>(tuple);

        world->QueueDestroyObject(id);
    }

    Markers.clear();
}
// ------------------------------------ //
DLLEXPORT void PositionMarkerOwner::Add(Lock &guard, ObjectID entity, Position& pos){

    Markers.push_back(make_tuple(entity, &pos));
}
// ------------------------------------ //
DLLEXPORT void PositionMarkerOwner::AddDataToPacket(Lock &guard, sf::Packet &packet) const{

    int32_t size = Markers.size();

    packet << size;
    
    for(int32_t i = 0; i < size; i++){

        packet << std::get<0>(Markers[i]) << std::get<1>(Markers[i])->_Position <<
            std::get<1>(Markers[i])->_Orientation;
    }
}
// ------------------------------------ //
DLLEXPORT PositionMarkerOwner::Data PositionMarkerOwner::LoadDataFromPacket(sf::Packet &packet){

    Data data;
    
    int32_t size;

    packet >> size;

    if(!packet)
        return data;

    data.EntityPositions.reserve(size);

    for(int32_t i = 0; i < size; i++){

        ObjectID id;
        Float3 pos;
        Float4 rot;

        packet >> id >> pos >> rot;
        
        data.EntityPositions.push_back(move(make_tuple(id, pos, rot)));
    }

    return data;
}
// ------------------ Model ------------------ //
DLLEXPORT Model::Model(const std::string &file) : ModelFile(file), GraphicalObject(nullptr){

}
// ------------------------------------ //
DLLEXPORT void Model::Release(Ogre::SceneManager* scene){

    if(GraphicalObject){

        scene->destroyEntity(GraphicalObject);
        GraphicalObject = nullptr;
    }
}
// ------------------ Constraintable ------------------ //
DLLEXPORT Constraintable::Constraintable(ObjectID id, GameWorld* world) :
    World(world), PartOfEntity(id)
{

}

DLLEXPORT Constraintable::~Constraintable(){

    GUARD_LOCK();

    for(auto&& constraint : PartInConstraints){

        constraint->Destroy(this);
    }

    PartInConstraints.clear();
}

DLLEXPORT void Constraintable::RemoveConstraint(BaseConstraint* removed){

    GUARD_LOCK();

    for(auto iter = PartInConstraints.begin(); iter != PartInConstraints.end(); ++iter){

        if((*iter).get() == removed){

            PartInConstraints.erase(iter);
            return;
        }
    }
}

DLLEXPORT void Constraintable::AddConstraint(std::shared_ptr<BaseConstraint> added){

    GUARD_LOCK();

    PartInConstraints.push_back(added);
}
// ------------------------------------ //
DLLEXPORT void Constraintable::_NotifyCreate(std::shared_ptr<BaseConstraint> newconstraint,
    Constraintable &other)
{

    this->AddConstraint(newconstraint);
    other.AddConstraint(newconstraint);
    

    // Notify world //
    World->NotifyNewConstraint(newconstraint);
}
// ------------------ ManualObject ------------------ //
DLLEXPORT ManualObject::ManualObject() : Object(nullptr){
    
}

DLLEXPORT void ManualObject::Release(Ogre::SceneManager* scene){

    if(Object){

        scene->destroyManualObject(Object);
        Object = nullptr;
    }

    CreatedMesh.clear();
}


