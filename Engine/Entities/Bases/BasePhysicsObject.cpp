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
void Leviathan::BasePhysicsObject::_DestroyPhysicalBody(Lock &guard){
    
	if(Collision)
		NewtonDestroyCollision(Collision);
	if(Body)
		NewtonDestroyBody(Body);
    
	Body = NULL;
	Collision = NULL;
}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::PosUpdated(Lock &guard){

	_UpdatePhysicsObjectLocation(guard);
}

void Leviathan::BasePhysicsObject::OrientationUpdated(Lock &guard){

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
        Force += tmp->_GatherApplyForces(guard, mass);
    

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

DLLEXPORT void Leviathan::BasePhysicsObject::SetBodyVelocity(Lock &guard,
    const Float3 &velocities)
{
	if(Body){

		NewtonBodySetVelocity(Body, &velocities.X);
	}
}

DLLEXPORT Float3 Leviathan::BasePhysicsObject::GetBodyVelocity(Lock &guard){

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    Float3 vel(0);
    NewtonBodyGetVelocity(Body, &vel.X);
    return vel;
}

DLLEXPORT Float3 Leviathan::BasePhysicsObject::GetBodyTorque(Lock &guard){

    if(!Body)
        throw InvalidState("Physics object doesn't have a body");
    
    Float3 torq(0);
    NewtonBodyGetTorque(Body, &torq.X);
    return torq;
}

DLLEXPORT void Leviathan::BasePhysicsObject::SetBodyTorque(Lock &guard, const Float3 &torque){
    
    if(Body){

        NewtonBodySetTorque(Body, &torque.X);
    }
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

DLLEXPORT void Leviathan::BasePhysicsObject::ApplyForce(ApplyForceInfo* pointertohandle){
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

DLLEXPORT void Leviathan::BasePhysicsObject::SetPhysicalMaterialID(Lock &guard, int ID){

    if(!Body)
        throw InvalidState("Calling set material ID without having physical Body");

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
DLLEXPORT bool Leviathan::BasePhysicsObject::AddPhysicalStateToPacket(Lock &guard,
    sf::Packet &packet)
{
    if(!Body)
        return false;

    packet << GetBodyVelocity(guard);
    packet << GetBodyTorque(guard);

    return true;
}

DLLEXPORT bool Leviathan::BasePhysicsObject::ApplyPhysicalStateFromPacket(Lock &guard,
    sf::Packet &packet)
{
    if(!Body)
        return false;

    Float3 vel;
    Float3 torq;

    packet >> vel;
    packet >> torq;

    if(!packet)
        return false;

    SetBodyTorque(guard, torq);
    SetBodyVelocity(guard, vel);
    
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

DLLEXPORT void Leviathan::BasePhysicsObject::ApplyPhysicalState(Lock &guard,
    BasePhysicsData &data)
{
    if(!Body)
        return;
    
    SetBodyTorque(guard, data.Torque);
    SetBodyVelocity(guard, data.Velocity);
}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::OnBeforeResimulateStateChanged(){

}

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
// ------------------ ApplyForceInfo ------------------ //
DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(bool addmass,
    std::function<Float3(ApplyForceInfo* instance, BasePhysicsObject* object, Lock &objectlock)>
    getforce, std::string* name /*= NULL*/) : 
	Callback(getforce), MultiplyByMass(addmass), OptionalName(name)
{
	
    
}

DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(ApplyForceInfo &other) :
    Callback(other.Callback), MultiplyByMass(other.MultiplyByMass)
{
    if(other.OptionalName)
        OptionalName = move(make_unique<std::string>(*other.OptionalName.get()));
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








