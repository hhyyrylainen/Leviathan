#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ENTITY_PROP
#include "Prop.h"
#endif
#include "ObjectFiles\ObjectFileProcessor.h"
#include "OgreMatrix4.h"
#include <Newton.h>
using namespace Leviathan;
using namespace Leviathan::Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::Prop::Prop(bool hidden) : BaseRenderable(hidden), BaseObject(IDFactory::GetID()), LinkedToWorld(NULL), 
	GraphicalObject(NULL)
{

}

DLLEXPORT Leviathan::Entity::Prop::~Prop(){

}

DLLEXPORT void Leviathan::Entity::Prop::Release(){
	// release Ogre entity //
	ObjectsNode->removeAndDestroyAllChildren();
	LinkedToWorld->GetScene()->destroySceneNode(ObjectsNode);

	// physical entity //
	NewtonDestroyCollision(Collision);
	NewtonDestroyBody(Body);
	Body = NULL;
	Collision = NULL;

	GraphicalObject = NULL;
	LinkedToWorld = NULL;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Prop::Init(const wstring &modelfile, GameWorld* world){
	QUICKTIME_THISSCOPE;
	// store world //
	LinkedToWorld = world;

	// parse file //
	vector<shared_ptr<NamedVariableList>> headervar;
	vector<shared_ptr<ObjectFileObject>>  objects = ObjectFileProcessor::ProcessObjectFile(modelfile,  headervar);

	NamedVars varlist(headervar);

	wstring ogrefile;

	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist, L"Model-Graphical", ogrefile, L"error", true, L"Prop: Init: no model file!:");

	// load the Ogre entity //
	GraphicalObject = world->GetScene()->createEntity(Convert::WstringToString(ogrefile));

	// create scene node for positioning //
	ObjectsNode = world->GetScene()->getRootSceneNode()->createChildSceneNode(Convert::WstringToString(modelfile)+"_basenode");

	// attach for deletion and valid display //
	ObjectsNode->attachObject(GraphicalObject);

	// TODO: load physics model here //

	// create a test box //
	Ogre::AxisAlignedBox bbox = GraphicalObject->getBoundingBox();

	NewtonWorld* tmpworld = world->GetPhysicalWorld()->GetWorld();

	Ogre::Vector3 sizes = bbox.getMaximum()-bbox.getMinimum();

	// we need to offset the bounding box because the node is at the bottom of the box and newton uses the center of the box //
	Ogre::Matrix4 offset = Ogre::Matrix4::IDENTITY;
	offset.setTrans(bbox.getCenter());
	
	Ogre::Matrix4 toffset = offset.transpose();

	Collision = NewtonCreateBox(tmpworld, sizes.x, sizes.y, sizes.z, NULL, &toffset[0][0]);

	Ogre::Matrix4 locrot = Ogre::Matrix4::IDENTITY;
	locrot.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tlocrot = locrot.transpose();

	Body = NewtonCreateDynamicBody(tmpworld, Collision, &tlocrot[0][0]);
	
	// add this as user data //
	NewtonBodySetUserData(Body, this);

	// first calculate inertia and center of mass points //
	Float3 inertia;
	Float3 centerofmass;

	NewtonConvexCollisionCalculateInertialMatrix(Collision, &inertia.X, &centerofmass.X);
	// set mass //
	float Mass = 120.f;
	// apply mass to inertia 
	inertia *= Mass;

	NewtonBodySetMassMatrix(Body, Mass, inertia.X, inertia.Y, inertia.Z);
	NewtonBodySetCentreOfMass(Body, &centerofmass.X);


	// callbacks //
	NewtonBodySetTransformCallback(Body, Prop::PropPhysicsMovedEvent);
	NewtonBodySetForceAndTorqueCallback(Body, Prop::ApplyForceAndTorgueEvent);
	NewtonBodySetDestructorCallback(Body, Prop::DestroyBodyCallback);

	// tweaking parameters //
	


	// should be fine //
	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Prop::CheckRender(GraphicalInputEntity* graphics, int mspassed){


	return true;
}
// ------------------------------------ //
void Leviathan::Entity::Prop::PosUpdated(){
	_UpdatePhysicsObjectLocation();
}

void Leviathan::Entity::Prop::OrientationUpdated(){
	_UpdatePhysicsObjectLocation();
}

void Leviathan::Entity::Prop::_UpdatePhysicsObjectLocation(){
	// update physics object location which will in turn change graphical object location //

	Ogre::Matrix4 matrix;
	matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tmatrix = matrix.transpose();

	// update body //
	NewtonBodySetMatrix(Body, &tmatrix[0][0]);

	//// update graphical object location to have it always match up (only for static objects) //
	//if(Immovable){
	//	ObjectsNode->setOrientation(quat);
	//	ObjectsNode->setPosition(position);
	//}
}

// ------------------------------------ //
void Leviathan::Entity::Prop::PropPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix, int threadIndex){

	// first create Ogre 4x4 matrix from the matrix //
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10],
		matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);

	// this might be "cleaner" way to do this //
	//memcpy(mat.m, matrix, sizeof(float)*16);

	// needs to convert from d3d style matrix to OpenGL style matrix //
	Ogre::Matrix4 tmat = mat.transpose();

	Ogre::Vector3 vec = tmat.getTrans();

	Float3 position = vec;

	// rotation //
	Float4 quat(tmat.extractQuaternion());

	// apply to graphical object //
	Prop* tmp = static_cast<Prop*>(NewtonBodyGetUserData(body));

	tmp->ObjectsNode->setOrientation(quat);
	tmp->ObjectsNode->setPosition(position);
	// also update these so if only one is updated it doesn't force last value to rotation or location //
	tmp->Position = position;
	tmp->QuatRotation = quat;
}

void Leviathan::Entity::Prop::ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat timestep, int threadIndex){
	// apply gravity //
	Float3 Torque(0, 0, 0);

	// get properties from newton //
	float mass; 
	float Ixx; 
	float Iyy; 
	float Izz; 

	NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);



	Prop* tmp = static_cast<Prop*>(NewtonBodyGetUserData(body));

	// get gravity force and apply mass to it //
	Float3 Force = tmp->LinkedToWorld->GetGravityAtPosition(tmp->Position)*mass;
	

	NewtonBodyAddForce(body, &Force.X);
	NewtonBodyAddTorque(body, &Torque.X);
}

void Leviathan::Entity::Prop::DestroyBodyCallback(const NewtonBody* body){
	// no user data to destroy //

}



