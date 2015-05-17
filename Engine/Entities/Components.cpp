// ------------------------------------ //
#include "Components.h"

#include "OgreSceneManager.h"
#include "OgreBillboardChain.h"
#include "OgreRibbonTrail.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

// ------------------ Position ------------------ //
DLLEXPORT Position::Position(){

}

DLLEXPORT bool Position::Init(const Float3 pos, const Float4 rot){

    _Position = pos;
    _Orientation = rot;

    Marked = true;
}

//! \brief Initializes at 0, 0, 0
DLLEXPORT bool Position::Init(){

    _Position = Float3(0, 0, 0);
    _Orientation = Float4::IdentityQuaternion();
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
DLLEXPORT void Position::Interpolate(PositionDeltaState &from, PositionDeltaState &to,
    float progress)
{
    Float3 pos = _Position;
    Float4 rot = _Orientation;

    // Position
    if(second.ValidFields & PRDELTAUPDATED_POS_X){

        if(first.ValidFields & PRDELTAUPDATED_POS_X){
            
            pos.X = first.Position.X + (second.Position.X-first.Position.X)*progress;
        } else {

            pos.X = second.Position.X;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_POS_X){
        
        pos.X = first.Position.X;
    }

    if(second.ValidFields & PRDELTAUPDATED_POS_Y){

        if(first.ValidFields & PRDELTAUPDATED_POS_Y){
            
            pos.Y = first.Position.Y + (second.Position.Y-first.Position.Y)*progress;
        } else {

            pos.Y = second.Position.Y;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_POS_Y){
        
        pos.Y = first.Position.Y;
    }
    
    if(second.ValidFields & PRDELTAUPDATED_POS_Z){

        if(first.ValidFields & PRDELTAUPDATED_POS_Z){
            
            pos.Z = first.Position.Z + (second.Position.Z-first.Position.Z)*progress;
        } else {

            pos.Z = second.Position.Z;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_POS_Z){
        
        pos.Z = first.Position.Z;
    }
    
    // Rotation
    // TODO: spherical interpolation for rotation
    if(second.ValidFields & PRDELTAUPDATED_ROT_X){

        if(first.ValidFields & PRDELTAUPDATED_ROT_X){
            
            rot.X = first.Rotation.X + (second.Rotation.X-first.Rotation.X)*progress;
        } else {

            rot.X = second.Rotation.X;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_ROT_X){

        rot.X = first.Rotation.X;
    }

    if(second.ValidFields & PRDELTAUPDATED_ROT_Y){

        if(first.ValidFields & PRDELTAUPDATED_ROT_Y){
            
            rot.Y = first.Rotation.Y + (second.Rotation.Y-first.Rotation.Y)*progress;
        } else {

            rot.Y = second.Rotation.Y;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_ROT_Y){

        rot.Y = first.Rotation.Y;
    }
    
    if(second.ValidFields & PRDELTAUPDATED_ROT_Z){

        if(first.ValidFields & PRDELTAUPDATED_ROT_Z){
            
            rot.Z = first.Rotation.Z + (second.Rotation.Z-first.Rotation.Z)*progress;
        } else {

            rot.Z = second.Rotation.Z;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_ROT_Z){

        rot.Z = first.Rotation.Z;
    }
    
    if(second.ValidFields & PRDELTAUPDATED_ROT_W){

        if(first.ValidFields & PRDELTAUPDATED_ROT_W){
            
            rot.W = first.Rotation.W + (second.Rotation.W-first.Rotation.W)*progress;
        } else {

            rot.W = second.Rotation.W;
        }
        
    } else if(first.ValidFields & PRDELTAUPDATED_ROT_W){

        rot.W = first.Rotation.W;
    }
    
    _Position = pos;
    _Orientation = rot;

    Marked = true;
}
// ------------------ RenderNode ------------------ //
DLLEXPORT RenderNode::RenderNode(){

}

DLLEXPORT bool RenderNode::Init(){

    Node = nullptr;
}

DLLEXPORT void RenderNode::Release(Ogre::Scene* worldsscene){

    worldsscene->destroySceneNode(Node);
    Node = nullptr;
}


// ------------------ Physics ------------------ //
DLLEXPORT void Physics::JumpTo(Position &target){

    GUARD_LOCK();
    SetPosition(guard, target._Position, target._Orientation);
}

DLLEXPORT bool Physics::SetPosition(Lock &guard, const Float3 &pos, const Float4 &orientation){

    if(!Body)
        return false;
    
    Ogre::Matrix4 matrix;
    matrix.makeTransform(pos, Float3(1, 1, 1), orientation);

    Ogre::Matrix4 tmatrix = matrix.transpose();

    // Update body //
    NewtonBodySetMatrix(Body, &tmatrix[0][0]);

    return true;
}


void Leviathan::Entity::Prop::PhysicsMovedEvent(const NewtonBody* const body,
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
    
    if(tmp->UpdatePosition){

        GUARD_LOCK_OTHER_NAME(tmp->UpdatePosition, guard2);

        tmp->UpdatePosition->_Position = tmat.getTrans();
        tmp->UpdatePosition->_Orientation = tmat.extractQuaternion();
        tmp->UpdatePosition->Marked = true;
    }
    
    if(tmp->UpdateSendable){
        
        tmp->UpdateSendable = true;
    }
    
    tmp->Marked = true;
}

void Physics::ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat
    timestep, int threadIndex)
{
	// Get object from body //
	Physics* tmp = reinterpret_cast<Physics*>(NewtonBodyGetUserData(body));
    
	// Check if physics can't apply //
	if(tmp->Immovable)
		return;

	Float3 Torque(0, 0, 0);

	// Get properties from newton //
	float mass; 
	float Ixx; 
	float Iyy; 
	float Izz; 

	NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);

    GUARD_LOCK_OTHER(tmp);

	// get gravity force and apply mass to it //
	Float3 Force = tmp->OwnedByWorld->GetGravityAtPosition(tmp->Position)*mass;
    
	// add other forces //
    if(!tmp->ApplyForceList.empty())
        Force += tmp->_GatherApplyForces(guard, mass);
    

	NewtonBodyAddForce(body, &Force.X);
	NewtonBodyAddTorque(body, &Torque.X);
}

void Physics::DestroyBodyCallback(const NewtonBody* body){
    
    Body = nullptr;
    Collision = nullptr;
}
// ------------------------------------ //
DLLEXPORT void Physics::GiveImpulse(const Float3 &deltaspeed, const Float3 &point /*= Float3(0)*/){
    GUARD_LOCK();
    
	if(Body){

		NewtonBodyAddImpulse(Body, &deltaspeed.X, &point.X);
	}
}

DLLEXPORT void Physics::SetBodyVelocity(Lock &guard,
    const Float3 &velocities)
{
	if(Body){

		NewtonBodySetVelocity(Body, &velocities.X);
	}
}

DLLEXPORT Float3 Physics::GetBodyVelocity(Lock &guard){

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    Float3 vel(0);
    NewtonBodyGetVelocity(Body, &vel.X);
    return vel;
}

DLLEXPORT Float3 Physics::GetBodyTorque(Lock &guard){

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    Float3 torq(0);
    NewtonBodyGetTorque(Body, &torq.X);
    return torq;
}

DLLEXPORT void Physics::SetBodyTorque(Lock &guard, const Float3 &torque){
    
    if(Body){

        NewtonBodySetTorque(Body, &torque.X);
    }
}

DLLEXPORT void Physics::SetLinearDampening(float factor /*= 0.1f*/){
    GUARD_LOCK();
    
    if(Body)
        NewtonBodySetLinearDamping(Body, factor);
}
// ------------------------------------ //
Float3 Leviathan::BasePhysicsObject::_GatherApplyForces(Lock &guard, const float &mass){
	// Return if just an empty list //
	if(ApplyForceList.empty())
		return Float3(0);

	Float3 total(0);

	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter){
		// Add to total, and multiply by mass if wanted //
        const Float3 force = (*iter)->Callback((*iter).get(), this, guard);
        
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
DLLEXPORT void Physics::InterpolatePhysicalState(PositionablePhysicalDeltaState &first,
    PositionablePhysicalDeltaState &second, float progress)
{

    // TODO: fix using partial first states
    DEBUG_BREAK;
    
    if(progress < 0.f)
        progress = 0.f;

    if(progress > 1.f)
        progress = 1.f;
    
    GUARD_LOCK();
    Float3 pos = GetPos(guard);
    Float4 rot = GetOrientation(guard);
    Float3 vel = GetBodyVelocity(guard);
    Float3 tor = GetBodyTorque(guard);

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
    
    SetPos(guard, pos);
    SetOrientation(guard, rot);
    SetBodyVelocity(guard, vel);
    SetBodyTorque(guard, tor);
}
// ------------------ Received ------------------ //
DLLEXPORT bool Received::LoadUpdateFromPacket(sf::Packet &packet, int ticknumber,
    int referencetick)
{

    auto receivedstate = CreateStateFromPacket(ticknumber, packet);

    if(!receivedstate){

        return false;
    }

    {
        GUARD_LOCK();

        // Skip if not newer than any //
        if(ClientStateBuffer.size() != 0){

            bool newer = false;
            bool filled = false;
            
            for(auto& obj : ClientStateBuffer){

                // Fill data from the reference tick to make this update packet as complete as possible //
                if(obj.Tick == referencetick){

                    // Add missing values //
                    receivedstate->FillMissingData(*obj.DeltaData);
                
                    filled = true;
                
                    if(newer)
                        break;
                }

                if(obj.Tick < ticknumber){
                
                    newer = true;

                    if(filled)
                        break;
                }
            }
            
            if(!newer)
                return false;

            // If it isn't filled that tells that our buffer is too small //
            // referencetick is invalid when it is -1 and is ignored in that case //
            if(!filled && referencetick != -1){

                bool olderexist = false;

                // TODO: make sure that this doesn't mess with interpolation too badly
                // under reasonable network stress, also this probably should never be missing
                // under normal conditions
                
                // Or that we have missed a single packet //
                for(auto& obj : ClientStateBuffer){

                    if(obj.Tick < referencetick){

                        olderexist = true;
                        break;
                    }
                }

                cout << "Entity old state " << referencetick << " is no longer in memory" << endl;
                
                //if(!olderexist)
                //    throw Exception("ReferenceTick is no longer in memory ClientStateBuffer "
                //        "is too small");
            }
            
        } else {

            // No stored states, must be newer //
            // Also no need to fill missing data as only the updated values should be in the packet //
            
        }

        
        // Store the new state in the buffer so that it can be found when interpolating //
        ClientStateBuffer.push_back(StoredState(receivedstate));
    }

    // Interpolations can only happen if more than one state is received
    if(ClientStateBuffer.size() > 1)
        _OnNewStateReceived();

    return true;
}

DLLEXPORT void Received::GetServerSentStates(shared_ptr<ObjectDeltaStateData> &first,
    std::shared_ptr<ObjectDeltaStateData> &second, int tick, float &progress) const
{
    bool firstfound = false;
    int secondfound = 0;

    {
        GUARD_LOCK();
    
        for(auto& obj : ClientStateBuffer){

            if(obj.Tick == tick){
            
                // This is the first state //
                first = obj.DeltaData;

                firstfound = true;
                continue;
            }

            // For this to be found the client should be around 50-100 milliseconds in the past
            if(obj.Tick > tick && (secondfound == 0 || obj.Tick-tick < secondfound)){

                // The second state //
                second = obj.DeltaData;
            
                secondfound = obj.Tick-tick;
                continue;
            }
        }
    }

    if(!firstfound || secondfound == 0){

        throw InvalidState("No stored server states around tick");
    }

    // Adjust progress //
    if(secondfound > 1){

        const float mspassed = TICKSPEED*progress;
        progress = mspassed / (TICKSPEED*secondfound);
    }
}

// ------------------ TrackController ------------------ //
DLLEXPORT bool TrackEntityController::SetStateToInterpolated(ObjectDeltaStateData &first,
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
DLLEXPORT bool Trail::SetTrailProperties(const Properties &variables, bool force /*= false*/){

    GUARD_LOCK();

    if(!TrailEntity || !_RenderNode)
        return false;

    // Set if we unconnected the node and we should reconnect it afterwards //
	bool ConnectAgain = false;

	// Determine if we need to unconnect the node //
	if(force || variables.MaxChainElements != CurrentSettings.MaxChainElements){

		// This to avoid Ogre bug //
		TrailEntity->removeNode(_RenderNode.Node);
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
		const ElementProperties* tmp = variables.Elements[i];

		if(tmp){
			TrailEntity->setInitialColour(i, tmp->InitialColour);
			TrailEntity->setInitialWidth(i, tmp->InitialSize);
			TrailEntity->setColourChange(i, tmp->ColourChange);
			TrailEntity->setWidthChange(i, tmp->SizeChange);
		}
	}

	// More bug avoiding //
	if(ConnectAgain)	
		TrailEntity->addNode(RenderNode.Node);

	return true;
}
// ------------------ Sendable ------------------ //
DLLEXPORT void Sendable::SendUpdatesToAllClients(int ticknumber){

    GUARD_LOCK();

    // Return if none could want any updates //
    if(!IsAnyDataUpdated)
        return;

    // Create current state here as one or more conections should require it //
    auto curstate = CaptureState(guard, ticknumber);
    
    auto end = UpdateReceivers.end();
    for(auto iter = UpdateReceivers.begin(); iter != end; ){

        // Currently all active connections will receive all updates //

        std::shared_ptr<sf::Packet> packet = make_shared<sf::Packet>();

        // Prepare the packet //
        // The first type is used by EntitySerializerManager and the second by the sendable entity serializer
        (*packet) << static_cast<int32_t>(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY) << static_cast<int32_t>(SerializeType);
        
        // Now calculate a delta update from curstate to the last confirmed state //
        curstate->CreateUpdatePacket((*iter)->LastConfirmedData.get(), *packet.get());
        
        // Check is the connection fine //
        std::shared_ptr<ConnectionInfo> safeconnection = NetworkHandler::Get()->GetSafePointerToConnection(
            (*iter)->CorrespondingConnection);
        
        // This will be the only function removing closed connections //
        // TODO: do a more global approach to avoid having to lookup connections here
        if(!safeconnection){

            iter = UpdateReceivers.erase(iter);
            end = UpdateReceivers.end();
            
            // TODO: add a disconnect callback
            continue;
        }

        // Create the final update packet //
        std::shared_ptr<NetworkResponse> updatemesg = make_shared<NetworkResponse>(-1,
            PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 4);

        updatemesg->GenerateEntityUpdateResponse(new NetworkResponseDataForEntityUpdate(OwnedByWorld->GetID(),
                GetID(), ticknumber, (*iter)->LastConfirmedTickNumber, packet));

        auto senthing = safeconnection->SendPacketToConnection(updatemesg, 1);

        // Add a callback for success //
        senthing->SetCallback(std::bind(
                &SendableObjectConnectionUpdate::SucceedOrFailCallback, (*iter), ticknumber, curstate,
                placeholders::_1, placeholders::_2));
        
        ++iter;
    }
    
    // After sending every connection is up to date //
    IsAnyDataUpdated = false;
}
