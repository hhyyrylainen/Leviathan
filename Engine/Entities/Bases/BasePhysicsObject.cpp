// ------------------------------------ //
#include "BasePhysicsObject.h"

#include "../GameWorld.h"
#include "Newton/PhysicsMaterialManager.h"
#include "../CommonStateObjects.h"
#include "BaseSendableEntity.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::BasePhysicsObject::BasePhysicsObject() :
    BaseObject(-1, NULL), Body(NULL), Collision(NULL), Immovable(false), ApplyGravity(false),
    AppliedPhysicalMaterial(-1)
{

}

DLLEXPORT Leviathan::BasePhysicsObject::~BasePhysicsObject(){

}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::_DestroyPhysicalBody(){
    GUARD_LOCK();
    
	if(Collision)
		NewtonDestroyCollision(Collision);
	if(Body)
		NewtonDestroyBody(Body);
    
	Body = NULL;
	Collision = NULL;
}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::PosUpdated(){
    GUARD_LOCK();
	_UpdatePhysicsObjectLocation(guard);
}

void Leviathan::BasePhysicsObject::OrientationUpdated(){
    GUARD_LOCK();
	_UpdatePhysicsObjectLocation(guard);
}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat
    timestep, int threadIndex)
{
	// get object from body //
	BasePhysicsObject* tmp = reinterpret_cast<BasePhysicsObject*>(NewtonBodyGetUserData(body));
    
	// check if physics can't apply //
	if(tmp->Immovable)
		return;

    GUARD_LOCK_OTHER(tmp);

	Float3 Torque(0, 0, 0);

	// get properties from newton //
	float mass; 
	float Ixx; 
	float Iyy; 
	float Izz; 

	NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);

	// get gravity force and apply mass to it //
	Float3 Force = tmp->OwnedByWorld->GetGravityAtPosition(tmp->Position)*mass;
    
	// add other forces //
    if(!tmp->ApplyForceList.empty())
        Force += tmp->_GatherApplyForces(mass);
    

	NewtonBodyAddForce(body, &Force.X);
	NewtonBodyAddTorque(body, &Torque.X);
}

void Leviathan::BasePhysicsObject::DestroyBodyCallback(const NewtonBody* body){
	// no user data to destroy //

}
// ------------------ Physical interaction functions ------------------ //
DLLEXPORT void Leviathan::BasePhysicsObject::GiveImpulse(const Float3 &deltaspeed, const Float3 &point /*= Float3(0)*/){
    GUARD_LOCK();
    
	if(Body){

		NewtonBodyAddImpulse(Body, &deltaspeed.X, &point.X);
	}
}

DLLEXPORT void Leviathan::BasePhysicsObject::SetBodyVelocity(const Float3 &velocities){
    GUARD_LOCK();
    
	if(Body){

		NewtonBodySetVelocity(Body, &velocities.X);
	}
}

DLLEXPORT Float3 Leviathan::BasePhysicsObject::GetBodyVelocity(){
    GUARD_LOCK();
    
	if(Body){
		Float3 vel(0);
		NewtonBodyGetVelocity(Body, &vel.X);
		return vel;
	}
    
	return (Float3)0;
}

DLLEXPORT Float3 Leviathan::BasePhysicsObject::GetBodyTorque(){
    GUARD_LOCK();
    
	if(Body){
		Float3 torq(0);
		NewtonBodyGetTorque(Body, &torq.X);
		return torq;
	}
    
	return (Float3)0;
}

DLLEXPORT void Leviathan::BasePhysicsObject::SetBodyTorque(const Float3 &torque){
    GUARD_LOCK();
    
    if(Body){

        NewtonBodySetTorque(Body, &torque.X);
    }
}
// ------------------------------------ //
Float3 Leviathan::BasePhysicsObject::_GatherApplyForces(const float &mass){
	// Return if just an empty list //
	if(ApplyForceList.empty())
		return Float3(0);

	Float3 total(0);

	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter){
		// Add to total, and multiply by mass if wanted //
        const Float3 force = (*iter)->Callback((*iter).get(), this);
        
		total += (*iter)->MultiplyByMass ? force*mass: force;

        // We might assert/crash here if the force was removed //
	}

	return total;
}

DLLEXPORT void Leviathan::BasePhysicsObject::ApplyForce(ApplyForceInfo* pointertohandle){
    GUARD_LOCK();
	// Overwrite old if found //
	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter){
        
		// Check do the names match //
		if((bool)(*iter)->OptionalName == (bool)pointertohandle->OptionalName){

			if(!pointertohandle->OptionalName || *pointertohandle->OptionalName == *(*iter)->OptionalName){
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

DLLEXPORT bool Leviathan::BasePhysicsObject::RemoveApplyForce(const std::string &name){
    GUARD_LOCK();
    
	// Search for a matching name //
    auto end = ApplyForceList.end();
	for(auto iter = ApplyForceList.begin(); iter != end; ++iter){
        
		// Check do the names match //
		if((!((*iter)->OptionalName) && name.size() == 0) || ((*iter)->OptionalName && *(*iter)->OptionalName == name)){
            
			ApplyForceList.erase(iter);
			return true;
		}
	}

	return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BasePhysicsObject::SetPhysicalMaterial(const std::string &materialname){
    // Empty string sets the default material //
    if(materialname.empty()){

        SetPhysicalMaterialID(GetDefaultPhysicalMaterialID());
        return true;
    }
    
	// Fetches the ID and calls the direct material ID setting function //
	int id = PhysicsMaterialManager::Get()->GetMaterialIDForWorld(materialname,
        OwnedByWorld->GetPhysicalWorld()->GetNewtonWorld());

	if(id == -1){
		// Invalid name //
		return false;
	}
    
	// Apply it //
	SetPhysicalMaterialID(id);
	return true;
}

DLLEXPORT void Leviathan::BasePhysicsObject::SetPhysicalMaterialID(int ID){
    GUARD_LOCK();
    
	assert(Body != NULL && "calling set material ID without having physical model loaded");

	NewtonBodySetMaterialGroupID(Body, ID);
}

DLLEXPORT int Leviathan::BasePhysicsObject::GetDefaultPhysicalMaterialID() const{

    return NewtonMaterialGetDefaultGroupID(OwnedByWorld->GetPhysicalWorld()->GetNewtonWorld());
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BasePhysicsObject::SetLinearDampening(float factor /*= 0.1f*/){
    GUARD_LOCK();
    
    if(Body)
        NewtonBodySetLinearDamping(Body, factor);
}
// ------------------------------------ //
bool Leviathan::BasePhysicsObject::BasePhysicsCustomMessage(int message, void* data){
	switch(message){
	case ENTITYCUSTOMMESSAGETYPE_ADDAPPLYFORCE:
		{
			// Add force by copying into a new apply force object //
			ApplyForceInfo* todestroyinfo = reinterpret_cast<ApplyForceInfo*>(data);
			
			// Copy to new and add the apply force //
			ApplyForce(new ApplyForceInfo(*todestroyinfo));

			return true;
		}
	case ENTITYCUSTOMMESSAGETYPE_REMOVEAPPLYFORCE:
		{
			// Remove force by name //
			std::string* tmpstring = reinterpret_cast<std::string*>(data);

			RemoveApplyForce(*tmpstring);

			return true;
		}
	case  ENTITYCUSTOMMESSAGETYPE_SETVELOCITY:
		{
			// Set the velocity from the data ptr //
			SetBodyVelocity(*reinterpret_cast<Float3*>(data));
			return true;
		}
	}
	return false;
}

bool Leviathan::BasePhysicsObject::BasePhysicsCustomGetData(ObjectDataRequest* data){
	switch(data->RequestObjectPart){
		case ENTITYDATA_REQUESTTYPE_NEWTONBODY: data->RequestResult = Body; return true;
	}

	return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BasePhysicsObject::AddPhysicalStateToPacket(sf::Packet &packet){
    GUARD_LOCK();
    
    if(!Body)
        return false;

    packet << GetBodyVelocity();
    packet << GetBodyTorque();

    return true;
}

DLLEXPORT bool Leviathan::BasePhysicsObject::ApplyPhysicalStateFromPacket(sf::Packet &packet){
    GUARD_LOCK();
    
    if(!Body)
        return false;

    Float3 vel;
    Float3 torq;

    packet >> vel;
    packet >> torq;

    if(!packet)
        return false;

    SetBodyTorque(torq);
    SetBodyVelocity(vel);
    
    return true;
}

DLLEXPORT bool Leviathan::BasePhysicsObject::LoadPhysicalStateFromPacket(sf::Packet &packet,
    BasePhysicsData &fill)
{
    packet >> fill.Velocity;
    packet >> fill.Torque;
    
    if(!packet)
        return false;
}

DLLEXPORT void Leviathan::BasePhysicsObject::ApplyPhysicalState(BasePhysicsData &data){
    GUARD_LOCK();
    
    if(!Body)
        return;
    
    SetBodyTorque(data.Torque);
    SetBodyVelocity(data.Velocity);
}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::OnBeforeResimulateStateChanged(){

}

#ifndef NETWORK_USE_SNAPSHOTS
DLLEXPORT void Leviathan::BasePhysicsObject::CheckOldPhysicalState(PositionablePhysicalDeltaState* servercasted,
    PositionablePhysicalDeltaState* ourcasted, int tick, BaseSendableEntity* assendable)
{
    // Check first do we need to resimulate //
    bool requireupdate = false;

    if(!ourcasted){

        requireupdate = true;
        
    } else {
        
        float totaldifference = 0.f;

        if(servercasted->ValidFields & PPDELTAUPDATED_POS_X)
            totaldifference += fabs((ourcasted->Position.X-servercasted->Position.X));
        if(servercasted->ValidFields & PPDELTAUPDATED_POS_Y)
            totaldifference += fabs((ourcasted->Position.Y-servercasted->Position.Y));
        if(servercasted->ValidFields & PPDELTAUPDATED_POS_Z)
            totaldifference += fabs((ourcasted->Position.Z-servercasted->Position.Z));

        if(servercasted->ValidFields & PPDELTAUPDATED_ROT_X)
            totaldifference += fabs((ourcasted->Rotation.X-servercasted->Rotation.X));
        if(servercasted->ValidFields & PPDELTAUPDATED_ROT_Y)
            totaldifference += fabs((ourcasted->Rotation.Y-servercasted->Rotation.Y));
        if(servercasted->ValidFields & PPDELTAUPDATED_ROT_Z)
            totaldifference += fabs((ourcasted->Rotation.Z-servercasted->Rotation.Z));
        if(servercasted->ValidFields & PPDELTAUPDATED_ROT_W)
            totaldifference += fabs((ourcasted->Rotation.W-servercasted->Rotation.W));
        
        if(servercasted->ValidFields & PPDELTAUPDATED_VEL_X)
            totaldifference += fabs((ourcasted->Velocity.X-servercasted->Velocity.X));
        if(servercasted->ValidFields & PPDELTAUPDATED_VEL_Y)
            totaldifference += fabs((ourcasted->Velocity.Y-servercasted->Velocity.Y));
        if(servercasted->ValidFields & PPDELTAUPDATED_VEL_Z)
            totaldifference += fabs((ourcasted->Velocity.Z-servercasted->Velocity.Z));

        if(servercasted->ValidFields & PPDELTAUPDATED_TOR_X)
            totaldifference += fabs((ourcasted->Torque.X-servercasted->Torque.X));
        if(servercasted->ValidFields & PPDELTAUPDATED_TOR_Y)
            totaldifference += fabs((ourcasted->Torque.Y-servercasted->Torque.Y));
        if(servercasted->ValidFields & PPDELTAUPDATED_TOR_Z)
            totaldifference += fabs((ourcasted->Torque.Z-servercasted->Torque.Z));


        if(totaldifference >= SENDABLE_RESIMULATE_THRESSHOLD){

            // We are too far of the right values //
            requireupdate = true;
        }
    }

    // All good if our old state matched //
    if(!requireupdate)
        return;

    // This should hold on to the world update lock once that is required //
    auto nworld = OwnedByWorld->GetPhysicalWorld()->GetNewtonWorld();


    OnBeforeResimulateStateChanged();
    
    // TODO: Check does this lock help with something
    GUARD_LOCK_NAME(lockit);
    
    // Go back to the verified position and resimulate from there //
    if(servercasted->ValidFields & PPDELTAUPDATED_POS_X){
        SetPosX(servercasted->Position.X);
        
    } else if(ourcasted){
        
        SetPosX(ourcasted->Position.X);
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_POS_Y){
        SetPosY(servercasted->Position.Y);
        
    } else if(ourcasted){
        
        SetPosY(ourcasted->Position.Y);
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_POS_Z){
        SetPosZ(servercasted->Position.Z);
        
    } else if(ourcasted){
        
        SetPosZ(ourcasted->Position.Z);
    }

    Float4 finalrotation;
    Float4 currotation = GetOrientation();

    if(servercasted->ValidFields & PPDELTAUPDATED_ROT_X){

        finalrotation.X = servercasted->Rotation.X;
        
    } else if(ourcasted){
        
        finalrotation.X = ourcasted->Rotation.X;
        
    } else {

        finalrotation.X = currotation.X;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_ROT_Y){

        finalrotation.Y = servercasted->Rotation.Y;
        
    } else if(ourcasted){
        
        finalrotation.Y = ourcasted->Rotation.Y;
        
    } else {

        finalrotation.Y = currotation.Y;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_ROT_Z){

        finalrotation.Z = servercasted->Rotation.Z;
        
    } else if(ourcasted){
        
        finalrotation.Z = ourcasted->Rotation.Z;
        
    } else {

        finalrotation.Z = currotation.Z;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_ROT_W){

        finalrotation.W = servercasted->Rotation.W;
        
    } else if(ourcasted){
        
        finalrotation.W = ourcasted->Rotation.W;
        
    } else {

        finalrotation.W = currotation.W;
    }
    
    SetOrientation(finalrotation);

    Float3 finalvelocity;
    Float3 curvelocity = GetBodyVelocity();

    if(servercasted->ValidFields & PPDELTAUPDATED_VEL_X){

        finalvelocity.X = servercasted->Velocity.X;
        
    } else if(ourcasted){
        
        finalvelocity.X = ourcasted->Velocity.X;
        
    } else {

        finalvelocity.X = curvelocity.X;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_VEL_Y){

        finalvelocity.Y = servercasted->Velocity.Y;
        
    } else if(ourcasted){
        
        finalvelocity.Y = ourcasted->Velocity.Y;
        
    } else {

        finalvelocity.Y = curvelocity.Y;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_VEL_Z){

        finalvelocity.Z = servercasted->Velocity.Z;
        
    } else if(ourcasted){
        
        finalvelocity.Z = ourcasted->Velocity.Z;
        
    } else {

        finalvelocity.Z = curvelocity.Z;
    }


    Float3 finaltorque;
    Float3 curtorque = GetBodyTorque();

    if(servercasted->ValidFields & PPDELTAUPDATED_TOR_X){

        finaltorque.X = servercasted->Torque.X;
        
    } else if(ourcasted){
        
        finaltorque.X = ourcasted->Torque.X;
        
    } else {

        finaltorque.X = curtorque.X;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_TOR_Y){

        finaltorque.Y = servercasted->Torque.Y;
        
    } else if(ourcasted){
        
        finaltorque.Y = ourcasted->Torque.Y;
        
    } else {

        finaltorque.Y = curtorque.Y;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_TOR_Z){

        finaltorque.Z = servercasted->Torque.Z;
        
    } else if(ourcasted){
        
        finaltorque.Z = ourcasted->Torque.Z;
        
    } else {

        finaltorque.Z = curtorque.Z;
    }    
    
    
    SetBodyVelocity(finalvelocity);
    SetBodyTorque(finaltorque);

    const int worldtick = OwnedByWorld->GetTickNumber();

    int tosimulate = worldtick-tick;

    // All the old states need to be replaced with new ones or we are going to resimulate a lot
    // for the next few ticks
    int advancedtick = tick;
    assendable->ReplaceOldClientState(advancedtick, assendable->CaptureState());

    int simulatedtime = 0;
    

    
    
    if(tosimulate < 1)
        return;

    lockit.unlock();

    
    
    // TODO: make sure that the entity doesn't get simulated before this resimulate call locks the world
    OwnedByWorld->GetPhysicalWorld()->ResimulateBody(Body, tosimulate*TICKSPEED,
        std::bind<void>([](int &simulatedtime, int &advancedtick, BaseSendableEntity* obj, int worldtick) -> void
            {
        
                // Keep track of current tick while resimulating //
                simulatedtime += NEWTON_FPS_IN_MICROSECONDS;

                if(simulatedtime >= TICKSPEED*1000){

                    advancedtick++;
                    simulatedtime -= TICKSPEED*1000;

                    assert(advancedtick <= worldtick && "BasePhysicsObject resimulate assert");
            
                    if(!obj->ReplaceOldClientState(advancedtick, obj->CaptureState())){

                        // The world tick might not have been stored yet... //
                        if(advancedtick != worldtick){

                            Logger::Get()->Warning("BasePhysicsObject("+Convert::ToString(obj->GetID())+
                                "): resimulate: didn't find old state for tick "+
                                Convert::ToString(advancedtick));
                        }
                    }
                }
            }, simulatedtime, advancedtick, assendable, worldtick),
        BasePhysicsGetConstraintable());
}
#else
DLLEXPORT void BasePhysicsObject::InterpolatePhysicalState(PositionablePhysicalDeltaState &first,
    PositionablePhysicalDeltaState &second, float progress)
{

    // TODO: fix using partial first states
    DEBUG_BREAK;
    
    if(progress < 0.f)
        progress = 0.f;

    if(progress > 1.f)
        progress = 1.f;
    
    GUARD_LOCK();
    Float3 pos = GetPosition();
    Float4 rot = GetOrientation();
    Float3 vel = GetBodyVelocity();
    Float3 tor = GetBodyTorque();

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
    
    SetPos(pos);
    SetOrientation(rot);
    SetBodyVelocity(vel);
    SetBodyTorque(tor);
}

#endif //NETWORK_USE_SNAPSHOTS
// ------------------ ApplyForceInfo ------------------ //
DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(bool addmass,
    std::function<Float3(ApplyForceInfo* instance, BasePhysicsObject* object)> getforce,
    std::string* name /*= NULL*/) : 
	Callback(getforce), MultiplyByMass(addmass), OptionalName(name)
{
	
    
}

DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(ApplyForceInfo &other) :
    Callback(other.Callback), MultiplyByMass(other.MultiplyByMass)
{
    if(other.OptionalName)
        OptionalName = move(unique_ptr<std::string>(new std::string(*other.OptionalName.get())));
}

DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(ApplyForceInfo &&other) :
    MultiplyByMass(move(other.MultiplyByMass)), Callback(std::move(other.Callback))
{
    OptionalName = move(other.OptionalName);
}

DLLEXPORT Leviathan::ApplyForceInfo::~ApplyForceInfo(){

}

DLLEXPORT ApplyForceInfo& Leviathan::ApplyForceInfo::operator=(const ApplyForceInfo &other){

	MultiplyByMass = other.MultiplyByMass;

    if(other.OptionalName){
        
        OptionalName = move(unique_ptr<std::string>(new std::string(*other.OptionalName.get())));
        
    } else {
        
        OptionalName.reset();
    }

	return *this;
}








