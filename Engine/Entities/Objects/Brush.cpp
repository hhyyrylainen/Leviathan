#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ENTITY_BRUSH
#include "Brush.h"
#endif
#include "OgreManualObject.h"
#include "OgreMeshManager.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
DLLEXPORT Leviathan::Entity::Brush::Brush(bool hidden) : BaseRenderable(hidden), BaseObject(IDFactory::GetID()), LinkedToWorld(NULL), 
	GraphicalObject(NULL), MeshName(), ObjectsNode(NULL), Collision(NULL), Body(NULL), Immovable(true), Sizes(0)
{

}

DLLEXPORT Leviathan::Entity::Brush::~Brush(){
	// release all //

	// release Ogre entity //
	ObjectsNode->removeAndDestroyAllChildren();
	LinkedToWorld->GetScene()->destroySceneNode(ObjectsNode);
	// the model won't be used anymore //
	if(MeshName.size()){
		LinkedToWorld->GetScene()->destroyManualObject(MeshName+"_manual");
		// this might not work as intended //
		Ogre::MeshManager::getSingleton().destroyResourcePool(MeshName);
	}

	// physical entity //
	_DestroyPhysics();

	GraphicalObject = NULL;
	LinkedToWorld = NULL;
}


void Leviathan::Entity::Brush::_DestroyPhysics(){
	if(Collision)
		NewtonDestroyCollision(Collision);
	if(Body)
		NewtonDestroyBody(Body);
	Body = NULL;
	Collision = NULL;
}

// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Brush::Init(GameWorld* world, const Float3 &dimensions, BRUSHCREATESTYLE style, const string &material, 
	bool createphysics /*= true*/)
{
	QUICKTIME_THISSCOPE;

	// copy world link //
	LinkedToWorld = world;

	Sizes = dimensions;

	// create unique name for mesh //
	MeshName = "Brush_"+Convert::ToString(ID);

	// create the graphical box //
	Ogre::ManualObject* TestModel = LinkedToWorld->GetScene()->createManualObject(MeshName+"_manual");

	// we do not want to update this later //
	TestModel->setDynamic(false);
	TestModel->estimateVertexCount(8);
	TestModel->estimateIndexCount(8);

	TestModel->begin(material, Ogre::RenderOperation::OT_TRIANGLE_LIST);

	switch(style){
	case BRUSHCREATESTYLE_CENTER:
		{
			// bottom left //
			TestModel->position(dimensions.X/-2.f, dimensions.Y/-2.f, dimensions.Z/-2.f);
			TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));

			// right point //
			TestModel->position(dimensions.X/2.f, dimensions.Y/-2.f, dimensions.Z/-2.f);
			TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));

			// right up //
			TestModel->position(dimensions.X/2.f, dimensions.Y/-2.f, dimensions.Z/2.f);
			TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));

			// left up //
			TestModel->position(dimensions.X/-2.f, dimensions.Y/-2.f, dimensions.Z/2.f);
			TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));

			// second layer //
			// left bottom //
			TestModel->position(dimensions.X/-2.f, dimensions.Y/2.f, dimensions.Z/-2.f);
			TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));

			// right point //
			TestModel->position(dimensions.X/2.f, dimensions.Y/2.f, dimensions.Z/-2.f);
			TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));

			// right up //
			TestModel->position(dimensions.X/2.f, dimensions.Y/2.f, dimensions.Z/2.f);
			TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));

			// left up //
			TestModel->position(dimensions.X/-2.f, dimensions.Y/2.f, dimensions.Z/2.f);
			TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));

		}
		break;
	case BRUSHCREATESTYLE_CORNER:
		{
			// TODO: change physics creation code //
			DEBUG_BREAK;
			// origin point //
			TestModel->position(0.f, 0.f, 0.f);
			TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));

			// right point //
			TestModel->position(dimensions.X, 0.f, 0.f);
			TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));

			// right up //
			TestModel->position(dimensions.X, 0.f, dimensions.Z);
			TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));

			// left up //
			TestModel->position(0.f, 0.f, dimensions.Z);
			TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));

			// second layer //
			// left bottom //
			TestModel->position(0.f, dimensions.Y, 0.f);
			TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));

			// right point //
			TestModel->position(dimensions.X, dimensions.Y, 0.f);
			TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));

			// right up //
			TestModel->position(dimensions.X, dimensions.Y, dimensions.Z);
			TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));

			// left up //
			TestModel->position(0.f, dimensions.Y, dimensions.Z);
			TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));
		}
		break;
	}
	// quads are both same //
	// base //
	TestModel->quad(0, 1, 2, 3);
	//TestModel->quad(3, 2, 1, 0);

	// front side //
	//TestModel->quad(0, 1, 5, 4);
	TestModel->quad(4, 5, 1, 0);

	// right side //
	//TestModel->quad(1, 2, 6, 5);
	TestModel->quad(5, 6, 2, 1);

	// left side //
	TestModel->quad(0, 3, 7, 4);
	//TestModel->quad(4, 7, 3, 0);

	// back side //
	//TestModel->quad(2, 3, 7, 6);
	TestModel->quad(6, 7, 3, 2);

	// top //
	//TestModel->quad(4, 5, 6, 7);
	TestModel->quad(7, 6, 5, 4);

	TestModel->end();

	// convert to mesh for instancing //
	TestModel->convertToMesh(MeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	// create instance //
	// load the Ogre entity //
	GraphicalObject = LinkedToWorld->GetScene()->createEntity(MeshName);

	// create scene node for positioning //
	ObjectsNode = LinkedToWorld->GetScene()->getRootSceneNode()->createChildSceneNode(MeshName+"_basenode");

	// attach for deletion and valid display //
	ObjectsNode->attachObject(GraphicalObject);

	// create physical box if wanted //
	if(createphysics)
		AddPhysicalObject(0.f);

	return true;
}

DLLEXPORT void Leviathan::Entity::Brush::AddPhysicalObject(const float &mass /*= 0.f*/){
	QUICKTIME_THISSCOPE;
	// destroy old first //
	_DestroyPhysics();

	// create a newton object which is always a box //

	NewtonWorld* tmpworld = LinkedToWorld->GetPhysicalWorld()->GetWorld();

	// possible offset //
	Ogre::Matrix4 offset = Ogre::Matrix4::IDENTITY;
	Ogre::Matrix4 toffset = offset.transpose();

	Collision = NewtonCreateBox(tmpworld, Sizes.X, Sizes.Y, Sizes.Z, NULL, &toffset[0][0]);

	Ogre::Matrix4 matrix;
	matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tmatrix = matrix.transpose();

	Body = NewtonCreateDynamicBody(tmpworld, Collision, &tmatrix[0][0]);
	// set location //
	_UpdatePhysicsObjectLocation();

	// add this as user data //
	NewtonBodySetUserData(Body, this);

	// set as movable if has mass //
	if(mass != 0){

		// first calculate inertia and center of mass points //
		Float3 inertia;
		Float3 centerofmass;

		NewtonConvexCollisionCalculateInertialMatrix(Collision, &inertia.X, &centerofmass.X);
		// set mass //
		// apply mass to inertia 
		inertia *= mass;


		Immovable = false;

		NewtonBodySetMassMatrix(Body, mass, inertia.X, inertia.Y, inertia.Z);
		NewtonBodySetCentreOfMass(Body, &centerofmass.X);

		// gravity callback //
		NewtonBodySetForceAndTorqueCallback(Body, Brush::ApplyForceAndTorgueEvent);
	}

	// callbacks //
	NewtonBodySetTransformCallback(Body, Brush::PropPhysicsMovedEvent);
	NewtonBodySetDestructorCallback(Body, Brush::DestroyBodyCallback);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Brush::CheckRender(GraphicalInputEntity* graphics, int mspassed){

	return true;
}
// ------------------------------------ //
void Leviathan::Entity::Brush::PosUpdated(){
	_UpdatePhysicsObjectLocation();
}

void Leviathan::Entity::Brush::OrientationUpdated(){
	_UpdatePhysicsObjectLocation();
}

void Leviathan::Entity::Brush::_UpdatePhysicsObjectLocation(){
	// update physics object location which will in turn change graphical object location //

	Ogre::Matrix4 matrix;
	matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tmatrix = matrix.transpose();

	// update body //
	NewtonBodySetMatrix(Body, &tmatrix[0][0]);

	// update graphical object location to have it always match up (only for static objects) //
	if(Immovable){
		ObjectsNode->setOrientation(QuatRotation);
		ObjectsNode->setPosition(Position);
	}
}
// ------------------------------------ //
void Leviathan::Entity::Brush::PropPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix, int threadIndex){
	// first create Ogre 4x4 matrix from the matrix //
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10],
		matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);

	// this might be "cleaner" way to do this //
	//memcpy(&mat[0][0], matrix, sizeof(float)*16);

	// needs to convert from d3d style matrix to OpenGL style matrix //
	Ogre::Matrix4 tmat = mat.transpose();
	Ogre::Vector3 vec = tmat.getTrans();

	Float3 position = vec;

	// rotation //
	Float4 quat(tmat.extractQuaternion());

	// apply to graphical object //
	Brush* tmp = static_cast<Brush*>(NewtonBodyGetUserData(body));

	tmp->ObjectsNode->setOrientation(quat);
	tmp->ObjectsNode->setPosition(position);
	// also update these so if only one is updated it doesn't force last value to rotation or location //
	tmp->Position = position;
	tmp->QuatRotation = quat;
}

void Leviathan::Entity::Brush::ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat timestep, int threadIndex){
	// get object from body //
	Brush* tmp = static_cast<Brush*>(NewtonBodyGetUserData(body));
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

void Leviathan::Entity::Brush::DestroyBodyCallback(const NewtonBody* body){
	// no user data to destroy //

}
