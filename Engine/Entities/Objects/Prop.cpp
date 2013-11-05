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
DLLEXPORT Leviathan::Entity::Prop::Prop(bool hidden, GameWorld* world) : BaseRenderable(hidden), BaseObject(IDFactory::GetID(), world), 
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
	AggressiveConstraintUnlink();
	_DestroyPhysicalBody();

	GraphicalObject = NULL;
	LinkedToWorld = NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Prop::Init(const wstring &modelfile){

	// parse file //
	std::vector<shared_ptr<NamedVariableList>> headervar;
	std::vector<shared_ptr<ObjectFileObject>>  objects = ObjectFileProcessor::ProcessObjectFile(modelfile,  headervar);

	NamedVars varlist(headervar);

	wstring ogrefile;

	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist, L"Model-Graphical", ogrefile, L"error", true, L"Prop: Init: no model file!:");

	// load the Ogre entity //
	GraphicalObject = LinkedToWorld->GetScene()->createEntity(Convert::WstringToString(ogrefile));

	// create scene node for positioning //
	ObjectsNode = LinkedToWorld->GetScene()->getRootSceneNode()->createChildSceneNode(Convert::WstringToString(modelfile)+"_basenode_"+Convert::ToString(ID));

	// attach for deletion and valid display //
	ObjectsNode->attachObject(GraphicalObject);

	
	// find the physics object //
	ObjectFileList* physicspropertieslist = NULL;

	// loop through the file and try to find it //
	for(size_t i = 0; i < objects.size(); i++){

		if(objects[i]->TName == L"PhysicalModel"){
			// loop it's contents and find "properties" //
			for(size_t a = 0; a < objects[i]->Contents.size(); a++){
				if(objects[i]->Contents[a]->Name == L"properties"){

					physicspropertieslist = objects[i]->Contents[a];
					break;
				}
			}
		}
	}

	if(!physicspropertieslist){
		// no physics model associated with this model, no further processing required //
		return true;
	}

	NewtonWorld* tmpworld = LinkedToWorld->GetPhysicalWorld()->GetWorld();

	// first get the type //
	wstring ptype;

	// get offset //
	wstring offsettype;
	Ogre::Matrix4 offset = Ogre::Matrix4::IDENTITY;
	// this is useful when the origin is at the bottom of the model and you don't take this into account in newton primitive (using Convex hull
	// should not require this) //
	if(ObjectFileProcessor::LoadValueFromNamedVars<wstring>(physicspropertieslist->Variables, L"Offset", offsettype, L"", true, 
		L"Prop: Init: CreatePhysicsModel:"))
	{

		if(offsettype == L"None"){
			// nothing needs to set //


		} else if(offsettype == L"BoundingBoxCenter"){

			Ogre::AxisAlignedBox bbox = GraphicalObject->getBoundingBox();
			offset.setTrans(bbox.getCenter());

		} else {
			Logger::Get()->Error(L"Prop: Init: invalid offset type, use None or BoundingBoxCenter for most common cases, file: "+modelfile);
			return false;
		}
	}
	// Newton uses different handed matrices
	Ogre::Matrix4 toffset = offset.transpose();


	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(physicspropertieslist->Variables, L"PrimitiveType", ptype, L"Convex");

	// process first the most complicated one, Convex hull which is basically a prop that we need to load //
	if(ptype == L"Convex"){

		Logger::Get()->Error(L"Prop: Init: physical object Convex hull loading is not implemented!");
		return false;
	}
	if(ptype == L"Sphere" || ptype == L"Ball"){
		// sphere primitive type, now we just need to load the size parameter or count it from the object bounding box //

		float radius = 0.f;
		if(!ObjectFileProcessor::LoadValueFromNamedVars<float>(physicspropertieslist->Variables, L"Size", radius, 0.f)){
			// it should be string type //
			wstring sizesourcename;
			
			if(!ObjectFileProcessor::LoadValueFromNamedVars<wstring>(physicspropertieslist->Variables, L"Size", sizesourcename, L"", true, 
				L"Prop: Init: CreatePhysicsModel:"))
			{
				Logger::Get()->Error(L"Prop: Init: physical model has no size! at least specify \"Size = GraphicalModel;\", file: "+modelfile);
				return false;
			}

			// process based on source name //

			if(sizesourcename == L"GraphicalModel"){
				// calculate radius from bounding box size //
				Ogre::AxisAlignedBox bbox = GraphicalObject->getBoundingBox();
				Ogre::Vector3 sizes = bbox.getMaximum()-bbox.getMinimum();

				// little sanity check //
				if(sizes.x != sizes.y || sizes.x != sizes.z){
					// it's not cube //
					Logger::Get()->Warning(L"Prop: Init: physical model sphere, the bounding box of graphical model is not cube, continuing anyways");
				}


			} else {
				Logger::Get()->Error(L"Prop: Init: physical model has no size! unknown source: "+sizesourcename+L" (\"Size = GraphicalModel;\"), file: "
					+modelfile);
				return false;
			}
		}
		// radius should be fine now //

		// create the sphere now //
		Collision = NewtonCreateSphere(tmpworld, radius, 0, &toffset[0][0]);

	} else if(ptype == L"Box"){

		DEBUG_BREAK;
		//Collision = NewtonCreateBox(tmpworld, sizes.x, sizes.y, sizes.z, NULL, &toffset[0][0]);
	} else {
		Logger::Get()->Error(L"Prop: Init: physical model has no type! unknown typename: "+ptype+L", \"PrimitiveType = Convex;\" for mesh collisions, file: "
			+modelfile);
		return false;

	}


	// create the body //
	Ogre::Matrix4 locrot = Ogre::Matrix4::IDENTITY;
	locrot.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

	Ogre::Matrix4 tlocrot = locrot.transpose();

	Body = NewtonCreateDynamicBody(tmpworld, Collision, &tlocrot[0][0]);
	
	// add this as user data //
	NewtonBodySetUserData(Body, static_cast<BasePhysicsObject*>(this));

	// Get mass //
	float Mass = 0;
	ObjectFileProcessor::LoadValueFromNamedVars<float>(physicspropertieslist->Variables, L"Mass", Mass, 0.f, true,	L"Prop: Init: CreatePhysicsModel:");

	// first calculate inertia and center of mass points //
	Float3 inertia;
	Float3 centerofmass;

	NewtonConvexCollisionCalculateInertialMatrix(Collision, &inertia.X, &centerofmass.X);

	// apply mass to inertia 
	inertia *= Mass;

	NewtonBodySetMassMatrix(Body, Mass, inertia.X, inertia.Y, inertia.Z);
	NewtonBodySetCentreOfMass(Body, &centerofmass.X);


	// callbacks //
	NewtonBodySetTransformCallback(Body, Prop::PropPhysicsMovedEvent);
	NewtonBodySetForceAndTorqueCallback(Body, BasePhysicsObject::ApplyForceAndTorgueEvent);
	NewtonBodySetDestructorCallback(Body, BasePhysicsObject::DestroyBodyCallback);

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

	if(Body){
		Ogre::Matrix4 matrix;
		matrix.makeTransform(Position, Float3(1, 1, 1), QuatRotation);

		Ogre::Matrix4 tmatrix = matrix.transpose();

		// update body //
		NewtonBodySetMatrix(Body, &tmatrix[0][0]);
	}

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
	Prop* tmp = static_cast<Prop*>(reinterpret_cast<BasePhysicsObject*>(NewtonBodyGetUserData(body)));

	tmp->ObjectsNode->setOrientation(quat);
	tmp->ObjectsNode->setPosition(position);
	// also update these so if only one is updated it doesn't force last value to rotation or location //
	tmp->Position = position;
	tmp->QuatRotation = quat;
}
// ------------------------------------ //
void Leviathan::Entity::Prop::_OnHiddenStateUpdated(){
	// Set scene node visibility //
	ObjectsNode->setVisible(!Hidden);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Entity::Prop::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	DEBUG_BREAK;
	return false;
}
