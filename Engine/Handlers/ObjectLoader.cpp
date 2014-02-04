#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTLOADER
#include "ObjectLoader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Statistics/TimingMonitor.h"
#include "OgreManualObject.h"
#include "Entities/Objects/Brush.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/TrackEntityController.h"
#include "Entities/Objects/TrailEmitter.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"

Leviathan::ObjectLoader::ObjectLoader(Engine* engine){
	m_Engine = engine;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ObjectLoader::CreateTestCubeToScene(Ogre::SceneManager* scene, string meshname){

	// create object to scene manager //
	Ogre::ManualObject* TestModel = scene->createManualObject(meshname+"_manual");

	// we do not want to update this later //
	TestModel->setDynamic(false);


	TestModel->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	{
		float qsize = 0.4f;
		float cp = 1.0f * qsize ;
		float cm = -1.0f * qsize;

		TestModel->position(cm, cp, cm);// a vertex
		TestModel->colour(Ogre::ColourValue(0.0f,1.0f,0.0f,1.0f));
		TestModel->position(cp, cp, cm);// a vertex
		TestModel->colour(Ogre::ColourValue(1.0f,1.0f,0.0f,1.0f));
		TestModel->position(cp, cm, cm);// a vertex
		TestModel->colour(Ogre::ColourValue(1.0f,0.0f,0.0f,1.0f));
		TestModel->position(cm, cm, cm);// a vertex
		TestModel->colour(Ogre::ColourValue(0.0f,0.0f,0.0f,1.0f));

		TestModel->position(cm, cp, cp);// a vertex
		TestModel->colour(Ogre::ColourValue(0.0f,1.0f,1.0f,1.0f));
		TestModel->position(cp, cp, cp);// a vertex
		TestModel->colour(Ogre::ColourValue(1.0f,1.0f,1.0f,1.0f));
		TestModel->position(cp, cm, cp);// a vertex
		TestModel->colour(Ogre::ColourValue(1.0f,0.0f,1.0f,1.0f));
		TestModel->position(cm, cm, cp);// a vertex
		TestModel->colour(Ogre::ColourValue(0.0f,0.0f,1.0f,1.0f));

		TestModel->triangle(0,1,2);
		TestModel->triangle(2,3,0);
		TestModel->triangle(4,6,5);
		TestModel->triangle(6,4,7);

		TestModel->triangle(0,4,5);
		TestModel->triangle(5,1,0);
		TestModel->triangle(2,6,7);
		TestModel->triangle(7,3,2);

		TestModel->triangle(0,7,4);
		TestModel->triangle(7,0,3);
		TestModel->triangle(1,5,6);
		TestModel->triangle(6,2,1);	
	}
	TestModel->end();

	// axes //
	TestModel->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_LINE_LIST);
	{
		float qsize = 0.4f;
		float lAxeSize = 2.0f * qsize;
		TestModel->position(0.0f, 0.0f, 0.0f);
		TestModel->colour(Ogre::ColourValue::Red);
		TestModel->position(lAxeSize, 0.0f, 0.0f);
		TestModel->colour(Ogre::ColourValue::Red);
		TestModel->position(0.0f, 0.0f, 0.0f);
		TestModel->colour(Ogre::ColourValue::Green);
		TestModel->position(0.0, lAxeSize, 0.0);
		TestModel->colour(Ogre::ColourValue::Green);
		TestModel->position(0.0f, 0.0f, 0.0f);
		TestModel->colour(Ogre::ColourValue::Blue);
		TestModel->position(0.0, 0.0, lAxeSize);
		TestModel->colour(Ogre::ColourValue::Blue);

		TestModel->index(0);
		TestModel->index(1);
		TestModel->index(2);
		TestModel->index(3);
		TestModel->index(4);
		TestModel->index(5);
	}
	TestModel->end();

	TestModel->convertToMesh(meshname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);


	// create manual quad //


	Ogre::ManualObject* TestQuad = scene->createManualObject("RttQuad_manual");
	TestQuad->setDynamic(false);
	TestQuad->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	// size calculation //
	{
		float qsize = 1.f;
		float cp = 1.0f*qsize;
		float cm = -1.0f*qsize;
		float tilecount = 1.0f;

		TestQuad->position(cm, cp, 0.0f);
		TestQuad->textureCoord(0.0f, 0.0f);

		TestQuad->position(cp, cp, 0.0f);
		TestQuad->textureCoord(tilecount, 0.0f);

		TestQuad->position(cp, cm, 0.0f);
		TestQuad->textureCoord(tilecount, tilecount);

		TestQuad->position(cm, cm, 0.0f);
		TestQuad->textureCoord(0.0f, tilecount);

		TestQuad->triangle(2, 1, 0);
		TestQuad->triangle(0, 3, 2);
	}
	TestQuad->end();
	Ogre::String QuadName = "RttQuad";
	TestQuad->convertToMesh(QuadName);
	// manual object no longer required //
	scene->destroyManualObject(QuadName);

}

DLLEXPORT void Leviathan::ObjectLoader::AddTestCubeToScenePositions(Ogre::SceneManager* scene, vector<Float3> &positions, const string &meshname){

	for(size_t i = 0; i < positions.size(); i++){

		Ogre::Entity* Entity = scene->createEntity(meshname);
		Ogre::SceneNode* Node = scene->getRootSceneNode()->createChildSceneNode();
		Node->attachObject(Entity);
		Node->translate(positions[i].X, positions[i].Y, positions[i].Z);
	}
}


// ------------------------------------ //
DLLEXPORT int Leviathan::ObjectLoader::LoadPropToWorld(GameWorld* world, const wstring &name, Entity::Prop** createdinstance){

	unique_ptr<Entity::Prop> prop(new Entity::Prop(false, world));

	(*createdinstance) = prop.get();

	// get relative path //
	wstring path = FileSystem::Get()->SearchForFile(FILEGROUP_MODEL, name, L"levmd", false);

	// initialize the model //
	prop->Init(path);

	int id = prop->GetID();

	// add to world //
	world->AddObject(prop.release());

	return id;
}

DLLEXPORT int Leviathan::ObjectLoader::LoadBrushToWorld(GameWorld* world, const string &material, const Float3 &size, const float &mass, 
	Entity::Brush** createdinstance)
{
	unique_ptr<Entity::Brush> brush(new Entity::Brush(false, world));

	(*createdinstance) = brush.get();

	// initialize the brush //
	brush->Init(size, material, mass == 0.f ? true: false);

	int id = brush->GetID();

	// optional physics init //
	if(mass != 0.f){
		brush->AddPhysicalObject(mass);
	}

	// add to world //
	world->AddObject(brush.release());

	return id;
}

DLLEXPORT int Leviathan::ObjectLoader::LoadBrushToWorld(GameWorld* world, const string &material, const Float3 &size, Entity::Brush** createdinstance){
	unique_ptr<Entity::Brush> brush(new Entity::Brush(false, world));

	(*createdinstance) = brush.get();

	// initialize the brush //
	brush->Init(size, material, false);

	int id = brush->GetID();

	// add to world //
	world->AddObject(brush.release());

	return id;
}
// ------------------ Complex entity loading ------------------ //
DLLEXPORT int Leviathan::ObjectLoader::LoadTrackEntityControllerToWorld(GameWorld* world, std::vector<Entity::TrackControllerPosition> &initialtrack, 
	BaseNotifiableEntity* controllable, Entity::TrackEntityController** createdinstance)
{
	// Construct the object //
	unique_ptr<Entity::TrackEntityController> tmpptr(new Entity::TrackEntityController(world));

	// Set nodes //
	for(auto iter = initialtrack.begin(); iter != initialtrack.end(); ++iter){
		// Add position //
		tmpptr->AddLocationToTrack(iter->Pos, iter->Orientation);
	}

	// Link object //
	if(controllable)
		tmpptr->ConnectToNotifiable(controllable);

	// Initialize //
	if(!tmpptr->Init()){

		DEBUG_BREAK;
		return -1;
	}

	// Copy ID for returning //
	int retid = tmpptr->GetID();

	// Copy instance ptr //
	*createdinstance = tmpptr.get();

	// Add to world //
	world->AddObject(tmpptr.release());

	return retid;
}

DLLEXPORT int Leviathan::ObjectLoader::LoadTrailToWorld(GameWorld* world, const string &material, const Entity::TrailProperties &properties, 
	bool allowupdatelater, Entity::TrailEmitter** createdinstance)
{
	// Construct the object //
	unique_ptr<Entity::TrailEmitter> tmpptr(new Entity::TrailEmitter(world));

	// Initialize //
	if(!tmpptr->Init(material, properties, allowupdatelater)){

		return -1;
	}

	// Copy ID for returning //
	int retid = tmpptr->GetID();

	// Copy instance ptr //
	*createdinstance = tmpptr.get();

	// Add to world //
	world->AddObject(tmpptr.release());

	return retid;
}
// ------------------------------------ //

// ------------------------------------ //



