#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASEPHYSICSOBJECT
#include "BasePhysicsObject.h"
#endif
#include "../GameWorld.h"
#include "Newton/PhysicsMaterialManager.h"
#include "../CommonStateObjects.h"
using namespace Leviathan;
// ------------------------------------ //
// I hope that this virtual constructor isn't actually called //
DLLEXPORT Leviathan::BasePhysicsObject::BasePhysicsObject() :
    BaseObject(-1, NULL), Body(NULL), Collision(NULL), Immovable(false), ApplyGravity(false),
    AppliedPhysicalMaterial(-1)
{

}

DLLEXPORT Leviathan::BasePhysicsObject::~BasePhysicsObject(){

}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::_DestroyPhysicalBody(){
	if(Collision)
		NewtonDestroyCollision(Collision);
	if(Body)
		NewtonDestroyBody(Body);
	Body = NULL;
	Collision = NULL;
}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::PosUpdated(){
    GUARD_LOCK_THIS_OBJECT();
	_UpdatePhysicsObjectLocation(guard);
}

void Leviathan::BasePhysicsObject::OrientationUpdated(){
    GUARD_LOCK_THIS_OBJECT();
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

    GUARD_LOCK_OTHER_OBJECT(tmp);

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

	if(Body){

		NewtonBodyAddImpulse(Body, &deltaspeed.X, &point.X);
	}
}

DLLEXPORT void Leviathan::BasePhysicsObject::SetBodyVelocity(const Float3 &velocities){
	if(Body){

		NewtonBodySetVelocity(Body, &velocities.X);
	}
}

DLLEXPORT Float3 Leviathan::BasePhysicsObject::GetBodyVelocity(){
	if(Body){
		Float3 vel(0);
		NewtonBodyGetVelocity(Body, &vel.X);
		return vel;
	}
    
	return (Float3)0;
}

DLLEXPORT Float3 Leviathan::BasePhysicsObject::GetBodyTorque(){
	if(Body){
		Float3 torq(0);
		NewtonBodyGetTorque(Body, &torq.X);
		return torq;
	}
    
	return (Float3)0;
}

DLLEXPORT void Leviathan::BasePhysicsObject::SetBodyTorque(const Float3 &torque){
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

	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ){
		// Add to total, and multiply by mass if wanted //
		total += (*iter)->MultiplyByMass ? (*iter)->ForcesToApply*mass: (*iter)->ForcesToApply;

		// Remove if it isn't persistent force //
		if(!(*iter)->Persist){
            
			iter = ApplyForceList.erase(iter);
            
		} else {
            
			++iter;
		}
	}

	return total;
}

DLLEXPORT void Leviathan::BasePhysicsObject::ApplyForce(ApplyForceInfo* pointertohandle){
    GUARD_LOCK_THIS_OBJECT();
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

DLLEXPORT bool Leviathan::BasePhysicsObject::RemoveApplyForce(const wstring &name){
    GUARD_LOCK_THIS_OBJECT();
    
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
DLLEXPORT bool Leviathan::BasePhysicsObject::SetPhysicalMaterial(const wstring &materialname){
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
	assert(Body != NULL && "calling set material ID without having physical model loaded");

	NewtonBodySetMaterialGroupID(Body, ID);
}

DLLEXPORT int Leviathan::BasePhysicsObject::GetDefaultPhysicalMaterialID() const{

    return NewtonMaterialGetDefaultGroupID(OwnedByWorld->GetPhysicalWorld()->GetNewtonWorld());
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
			wstring* tmpstring = reinterpret_cast<wstring*>(data);

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

    if(!Body)
        return false;

    packet << GetBodyVelocity();
    packet << GetBodyTorque();

    return true;
}

DLLEXPORT bool Leviathan::BasePhysicsObject::ApplyPhysicalStateFromPacket(sf::Packet &packet){

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

    if(!Body)
        return;
    
    SetBodyTorque(data.Torque);
    SetBodyVelocity(data.Velocity);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BasePhysicsObject::CheckOldPhysicalState(PositionablePhysicalDeltaState* servercasted,
    PositionablePhysicalDeltaState* ourcasted, int tick)
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

    
    // TODO: Check does this lock help with something
    UNIQUE_LOCK_THIS_OBJECT();
    
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


    Float3 finalvelocity;
    Float3 curvelocity = GetBodyVelocity();

    if(servercasted->ValidFields & PPDELTAUPDATED_VEL_X){

        finalvelocity.X = servercasted->Velocity.X;
        
    } else if(ourcasted){
        
        finalvelocity.X = ourcasted->Position.X;
        
    } else {

        finalvelocity.X = curvelocity.X;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_VEL_Y){

        finalvelocity.Y = servercasted->Velocity.Y;
        
    } else if(ourcasted){
        
        finalvelocity.Y = ourcasted->Position.Y;
        
    } else {

        finalvelocity.Y = curvelocity.Y;
    }

    if(servercasted->ValidFields & PPDELTAUPDATED_VEL_Z){

        finalvelocity.Z = servercasted->Velocity.Z;
        
    } else if(ourcasted){
        
        finalvelocity.Z = ourcasted->Position.Z;
        
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

    int tosimulate = OwnedByWorld->GetTickNumber()-tick;
    
    if(tosimulate < 1)
        return;

    lockit.unlock();
    
    //Logger::Get()->Write("Resimulating body for "+Convert::ToString(abs(OwnedByWorld->GetTickNumber()-tick))+
    //    " ticks");

    // TODO: make sure that the entity doesn't get simulated before this resimulate call locks the world
    OwnedByWorld->GetPhysicalWorld()->ResimulateBody(Body, tosimulate*TICKSPEED);
}
// ------------------ ApplyForceInfo ------------------ //
DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(const Float3 &forces, bool addmass, bool persist /*= true*/,
    wstring* name /*= NULL*/) : 
	ForcesToApply(forces), Persist(persist), MultiplyByMass(addmass), OptionalName(name)
{
	

}

DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(ApplyForceInfo &other) :
    ForcesToApply(other.ForcesToApply), Persist(other.Persist), 
    MultiplyByMass(other.MultiplyByMass)
{
	// Swap the pointers //
	OptionalName.swap(other.OptionalName);
}

DLLEXPORT Leviathan::ApplyForceInfo::~ApplyForceInfo(){

}

DLLEXPORT ApplyForceInfo& Leviathan::ApplyForceInfo::operator=(ApplyForceInfo &other){
	// assign data with normal operators and swap the unique ptr //
	ForcesToApply = other.ForcesToApply;
	Persist = other.Persist;
	MultiplyByMass = other.MultiplyByMass;

	OptionalName.swap(other.OptionalName);

	return *this;
}








