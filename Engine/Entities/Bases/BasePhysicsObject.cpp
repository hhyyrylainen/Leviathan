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

	// apply gravity //
	Float3 Torque(0, 0, 0);

	// get properties from newton //
	float mass; 
	float Ixx; 
	float Iyy; 
	float Izz; 

	NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);

	// get gravity force and apply mass to it //
	Float3 Force = tmp->LinkedToWorld->GetGravityAtPosition(tmp->Position)*mass;


	NewtonBodyAddForce(body, &Force.X);
	NewtonBodyAddTorque(body, &Torque.X);
}

void Leviathan::BasePhysicsObject::DestroyBodyCallback(const NewtonBody* body){
	// no user data to destroy //

}



