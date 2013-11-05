#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ENTITY_BRUSH
#include "Brush.h"
#endif
#include "OgreManualObject.h"
#include "OgreMeshManager.h"
#include "FileSystem.h"
using namespace Leviathan;
using namespace Entity;
// ------------------------------------ //
//#define BRUSH_CALCULATENORMALS		1



DLLEXPORT Leviathan::Entity::Brush::Brush(bool hidden, GameWorld* world) : BaseRenderable(hidden), BaseObject(IDFactory::GetID(), world), 
	GraphicalObject(NULL), MeshName(), ObjectsNode(NULL), Sizes(0)
{

}

DLLEXPORT Leviathan::Entity::Brush::~Brush(){
	// delete memory that can wait until this //
}

DLLEXPORT void Leviathan::Entity::Brush::Release(){
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
	AggressiveConstraintUnlink();
	_DestroyPhysicalBody();

	GraphicalObject = NULL;
	LinkedToWorld = NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Brush::Init(const Float3 &dimensions, const string &material, 
	bool createphysics /*= true*/)
{
	Sizes = dimensions;

	// create unique name for mesh //
	MeshName = "Brush_"+Convert::ToString(ID);

	// create the graphical box //
	Ogre::ManualObject* TestModel = LinkedToWorld->GetScene()->createManualObject(MeshName+"_manual");

	// we do not want to update this later //
	TestModel->setDynamic(false);
	TestModel->estimateVertexCount(24);
	TestModel->estimateIndexCount(24);
#ifdef BRUSH_CALCULATENORMALS
	std::vector<Float3> tmpvertices;
	tmpvertices.reserve(24);
#endif // BRUSH_CALCULATENORMALS


	TestModel->begin(material, Ogre::RenderOperation::OT_TRIANGLE_LIST);


	// loops to avoid redundant code //
	float yval = dimensions.Y/-2.f;
	bool up = false;

	for(int i = 0; i < 2; i++){
		// loop three times on all points for each side that has that point //

		// bottom left //
		for(int a = 0; a < 3; a++){

			TestModel->position(dimensions.X/-2.f, yval, dimensions.Z/-2.f);
#ifdef BRUSH_CALCULATENORMALS
			tmpvertices.push_back(Float3(dimensions.X/-2.f, yval, dimensions.Z/-2.f));
#endif // BRUSH_CALCULATENORMALS
					
			// bottom left //
			if(a == 0){

				TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));
				TestModel->normal(Float3(0.f, -1.f*(i != 0 ? -1.f: 1.f), 0.f));
						
			} else if (a == 1){
				// second is the face that is on the right when looking from the corner to the center of the cube //
				if(!up)
					TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));
				else
					TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));

				TestModel->normal(Float3(0.f, 0.f, -1.f));

			} else {
				// and third is on the left when looking to the center //
				if(!up)
					TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));
				else
					TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));

				TestModel->normal(Float3(-1.f, 0.f, 0.f));
			}
		}

		// right point //
		for(int a = 0; a < 3; a++){

			TestModel->position(dimensions.X/2.f, yval, dimensions.Z/-2.f);
#ifdef BRUSH_CALCULATENORMALS
			tmpvertices.push_back(Float3(dimensions.X/2.f, yval, dimensions.Z/-2.f));
#endif // BRUSH_CALCULATENORMALS


			// first is bottom or top face //
			if(a == 0){

				TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));
				TestModel->normal(Float3(0.f, -1.f*(i != 0 ? -1.f: 1.f), 0.f));

			} else if (a == 1){
				// second is the face that is on the right when looking from the corner to the center of the cube //
				if(!up)
					TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));
				else
					TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));

				TestModel->normal(Float3(1.f, 0.f, 0.f));

			} else {
				// and third is on the left when looking to the center //
				if(!up)
					TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));
				else
					TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));

				TestModel->normal(Float3(0.f, 0.f, -1.f));
			}
		}

		// right up //
		for(int a = 0; a < 3; a++){

			TestModel->position(dimensions.X/2.f, yval, dimensions.Z/2.f);
#ifdef BRUSH_CALCULATENORMALS
			tmpvertices.push_back(Float3(dimensions.X/2.f, yval, dimensions.Z/2.f));
#endif // BRUSH_CALCULATENORMALS


			// first is bottom or top face //
			if(a == 0){

				TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));
				TestModel->normal(Float3(0.f, -1.f*(i != 0 ? -1.f: 1.f), 0.f));

			} else if (a == 1){
				// second is the face that is on the right when looking from the corner to the center of the cube //
				if(!up)
					TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));
				else
					TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));

				TestModel->normal(Float3(0.f, 0.f, 1.f));

			} else {
				// and third is on the left when looking to the center //
				if(!up)
					TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));
				else
					TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));

				TestModel->normal(Float3(1.f, 0.f, 0.f));

			}
		}
		// left up //
		for(int a = 0; a < 3; a++){

			TestModel->position(dimensions.X/-2.f, yval, dimensions.Z/2.f);
#ifdef BRUSH_CALCULATENORMALS
			tmpvertices.push_back(Float3(dimensions.X/-2.f, yval, dimensions.Z/2.f));
#endif // BRUSH_CALCULATENORMALS


			// first is bottom or top face //
			if(a == 0){

				TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));
				TestModel->normal(Float3(0.f, -1.f*(i != 0 ? -1.f: 1.f), 0.f));

			} else if (a == 1){
				// second is the face that is on the right when looking from the corner to the center of the cube //
				if(!up)
					TestModel->textureCoord(Ogre::Vector2(0.f, 1.f));
				else
					TestModel->textureCoord(Ogre::Vector2(0.f, 0.f));

				TestModel->normal(Float3(-1.f, 0.f, 0.f));

			} else {
				// and third is on the left when looking to the center //
				if(!up)
					TestModel->textureCoord(Ogre::Vector2(1.f, 1.f));
				else
					TestModel->textureCoord(Ogre::Vector2(1.f, 0.f));

				TestModel->normal(Float3(0.f, 0.f, 1.f));
			}
		}
				
		// move to next layer for next loop //
		yval = dimensions.Y/2.f;
		up = true;
	}

	// quads are both same //
	// base //
	TestModel->quad(0, 3, 6, 9);

	// front side //
	TestModel->quad(13, 17, 5, 1);
	//TestModel->quad(1, 13, 17, 5);

	// right side //
	TestModel->quad(16, 20, 8, 4);
	//TestModel->quad(4, 16, 20, 8);

	// left side //
	TestModel->quad(2, 10, 22, 14);
	//TestModel->quad(10, 22, 14, 2);

	// back side //
	TestModel->quad(19, 23, 11, 7);
	//TestModel->quad(7, 19, 23, 11);

	// top //
	TestModel->quad(21, 18, 15, 12);
	//TestModel->quad(12, 21, 18, 15);


#ifdef BRUSH_CALCULATENORMALS
	// calculate normals and save //


	wstring filetext = L"";

	std::vector<Float3> normals(6);


	// calculate normals and store results according to indexes in above quads //
	normals[0] = MMath::CalculateNormal(tmpvertices[0], tmpvertices[3], tmpvertices[6]);
	normals[1] = MMath::CalculateNormal(tmpvertices[13], tmpvertices[17], tmpvertices[5]);
	normals[2] = MMath::CalculateNormal(tmpvertices[16], tmpvertices[20], tmpvertices[8]);
	normals[3] = MMath::CalculateNormal(tmpvertices[2], tmpvertices[10], tmpvertices[22]);
	normals[4] = MMath::CalculateNormal(tmpvertices[19], tmpvertices[23], tmpvertices[11]);
	normals[5] = MMath::CalculateNormal(tmpvertices[21], tmpvertices[18], tmpvertices[15]);


	for(size_t i = 0; i < normals.size(); i++){

		// space after a face //
		filetext += L"\nstarting face number: "+Convert::ToWstring(i)+L"\n";
		

		filetext += L"normal: "+Convert::ToWstring(normals[i].X)+L", "+Convert::ToWstring(normals[i].Y)+L", "
			+Convert::ToWstring(normals[i].Z)+L"\n";

	}


	FileSystem::WriteToFile(filetext, L"Brush_Normals_"+Convert::ToWstring(ID)+L".txt");

#endif // BRUSH_CALCULATENORMALS

	// end and turn into a mesh //
	TestModel->end();
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
	// destroy old first //
	AggressiveConstraintUnlink();
	_DestroyPhysicalBody();


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
	NewtonBodySetUserData(Body, static_cast<BasePhysicsObject*>(this));

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
		NewtonBodySetForceAndTorqueCallback(Body, BasePhysicsObject::ApplyForceAndTorgueEvent);
	} else {
		Immovable = true;
	}

	// callbacks //
	NewtonBodySetTransformCallback(Body, Brush::BrushPhysicsMovedEvent);
	NewtonBodySetDestructorCallback(Body, BasePhysicsObject::DestroyBodyCallback);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Brush::CheckRender(GraphicalInputEntity* graphics, int mspassed){

	return true;
}
// ------------------------------------ //
void Leviathan::Entity::Brush::_UpdatePhysicsObjectLocation(){
	// update physics object location which will in turn change graphical object location //

	Ogre::Matrix4 matrix;
	matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tmatrix = matrix.transpose();

	// update body //
	NewtonBodySetMatrix(Body, &tmatrix[0][0]);

	// update graphical object location to have it always match up //
	ObjectsNode->setOrientation(QuatRotation);
	ObjectsNode->setPosition(Position);
	
}
// ------------------------------------ //
void Leviathan::Entity::Brush::BrushPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix, int threadIndex){
	// first create Ogre 4x4 matrix from the matrix //
	Ogre::Matrix4 mat(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10],
		matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);

	// needs to convert from d3d style matrix to OpenGL style matrix //
	Ogre::Matrix4 tmat = mat.transpose();
	Ogre::Vector3 vec = tmat.getTrans();

	Float3 position = vec;

	// rotation //
	Float4 quat(tmat.extractQuaternion());

	// apply to graphical object //
	Brush* tmp = static_cast<Brush*>(reinterpret_cast<BasePhysicsObject*>(NewtonBodyGetUserData(body)));

	tmp->ObjectsNode->setOrientation(quat);
	tmp->ObjectsNode->setPosition(position);
	// also update these so if only one is updated it doesn't force last value to rotation or location //
	tmp->Position = position;
	tmp->QuatRotation = quat;
}
// ------------------------------------ //
void Leviathan::Entity::Brush::_OnHiddenStateUpdated(){
	// Set scene node visibility //
	ObjectsNode->setVisible(!Hidden);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Brush::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	DEBUG_BREAK;
	return false;
}

