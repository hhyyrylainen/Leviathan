#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASEPHYSICSOBJECT
#include "BasePhysicsObject.h"
#endif
#include "../GameWorld.h"
using namespace Leviathan;
// ------------------------------------ //
// I hope that this virtual constructor isn't actually called //
DLLEXPORT Leviathan::BasePhysicsObject::BasePhysicsObject() : BaseObject(-1, NULL), Body(NULL), Collision(NULL), Immovable(false){

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
	_UpdatePhysicsObjectLocation();
}

void Leviathan::BasePhysicsObject::OrientationUpdated(){
	_UpdatePhysicsObjectLocation();
}
// ------------------------------------ //
void Leviathan::BasePhysicsObject::ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat timestep, int threadIndex){
	// get object from body //
	BasePhysicsObject* tmp = reinterpret_cast<BasePhysicsObject*>(NewtonBodyGetUserData(body));
	// check if physics can't apply //
	if(tmp->Immovable)
		return;

	Float3 Torque(0, 0, 0);

	// get properties from newton //
	float mass; 
	float Ixx; 
	float Iyy; 
	float Izz; 

	NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);

	// get gravity force and apply mass to it //
	Float3 Force = tmp->LinkedToWorld->GetGravityAtPosition(tmp->Position)*mass;
	// add other forces //
	Force += tmp->_GatherApplyForces();


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
// ------------------------------------ //
Leviathan::Float3 Leviathan::BasePhysicsObject::_GatherApplyForces(){
	// return if just an empty list //
	if(ApplyForceList.size() == 0)
		return Float3(0);

	// we probably need the mass from the body, because some forces want to multiply by mass //
	float mass; 
	float Ixx, Iyy, Izz;
	NewtonBodyGetMassMatrix(Body, &mass, &Ixx, &Iyy, &Izz);

	Float3 total(0);

	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ){
		// add to total, and multiply by mass if wanted //
		total += (*iter)->MultiplyByMass ? (*iter)->ForcesToApply*mass: (*iter)->ForcesToApply;

		// remove if it isn't persistent force //
		if(!(*iter)->Persist){
			iter = ApplyForceList.erase(iter);
		} else {
			++iter;
		}
	}

	return total;
}

DLLEXPORT void Leviathan::BasePhysicsObject::ApplyForce(ApplyForceInfo* pointertohandle){
	// overwrite old if found //
	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter){
		// check do names match //
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
	// search for matching name //
	for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter){
		// check do names match //
		if(!(*iter)->OptionalName && name.size() == 0 || ((*iter)->OptionalName && *(*iter)->OptionalName == name)){
			ApplyForceList.erase(iter);
			return true;
		}
	}

	return false;
}
// ------------------ ApplyForceInfo ------------------ //
DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(const Float3 &forces, bool addmass, bool persist /*= true*/, wstring* name /*= NULL*/) : 
	ForcesToApply(forces), Persist(persist), MultiplyByMass(addmass), OptionalName(name)
{
	

}

DLLEXPORT Leviathan::ApplyForceInfo::ApplyForceInfo(const ApplyForceInfo& other){
	assert(false && "don't call this constructor!");
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
