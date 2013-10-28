#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GAMEWORLD
#include "GameWorld.h"
#endif
#include "Common\GraphicalInputEntity.h"
#include "Newton\NewtonManager.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GameWorld::GameWorld(Ogre::Root* ogre) : WorldSceneCamera(NULL), CameraLocationNode(NULL), WorldsScene(NULL),
	Sunlight(NULL), SunLightNode(NULL)
{
	// these are always required for worlds //
	_CreateOgreResources(ogre);

	// acquire physics engine world //
	_PhysicalWorld = NewtonManager::Get()->CreateWorld();

}


DLLEXPORT Leviathan::GameWorld::~GameWorld(){

}


DLLEXPORT void Leviathan::GameWorld::Release(){
	// release objects //
	for(size_t i = 0; i < Objects.size(); i++){

		Objects[i]->Release();
	}

	Objects.clear();


	// release Ogre resources //
	Ogre::Root::getSingleton().destroySceneManager(WorldsScene);
	WorldsScene = NULL;



	// some smart ptrs need releasing //
	_PhysicalWorld.reset();

}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::UpdateCameraAspect(GraphicalInputEntity* rendertarget){
	// set aspect ratio to the same as the view port (this makes it look realistic) //
	WorldSceneCamera->setAspectRatio(rendertarget->GetViewportAspectRatio());
	WorldSceneCamera->setAutoAspectRatio(true);

	// link camera //
	rendertarget->GetMainViewport()->setCamera(WorldSceneCamera);
}
// ------------------------------------ //
void Leviathan::GameWorld::_CreateOgreResources(Ogre::Root* ogre){
	// create scene manager //
	WorldsScene = ogre->createSceneManager(Ogre::ST_EXTERIOR_FAR, "MainSceneManager");

	// set scene shadow types //
	WorldsScene->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

	// create camera //
	WorldSceneCamera = WorldsScene->createCamera("Camera01");

	// create node for camera and attach it //
	CameraLocationNode = WorldsScene->getRootSceneNode()->createChildSceneNode("MainCameraNode");
	CameraLocationNode->attachObject(WorldSceneCamera);

	// near and far clipping planes //
	WorldSceneCamera->setFOVy(Ogre::Radian(60.f*DEGREES_TO_RADIANS));
	WorldSceneCamera->setNearClipDistance(0.1f);
	WorldSceneCamera->setFarClipDistance(50000.f);

	// enable infinite far clip distance if supported //
	if(ogre->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE)){
		
		WorldSceneCamera->setFarClipDistance(0);
	}

	// set scene ambient colour //
	WorldsScene->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

	// default sun //
	SetSunlight();

}

DLLEXPORT void Leviathan::GameWorld::SetSkyBox(const string &materialname){
	try{
		WorldsScene->setSkyBox(true, "NiceDaySky");
	}
	catch(const Ogre::InvalidParametersException &e){

		Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
	}
}

DLLEXPORT void Leviathan::GameWorld::SetFog(){
	WorldsScene->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue(0.7f, 0.7f, 0.8f), 0, 4000, 10000);
	WorldsScene->setFog(Ogre::FOG_NONE);
}

DLLEXPORT void Leviathan::GameWorld::SetSunlight(){
	// create/update things if they are NULL //
	if(!Sunlight){
		Sunlight = WorldsScene->createLight("sunlight");
	}

	Sunlight->setType(Ogre::Light::LT_DIRECTIONAL);
	Sunlight->setDiffuseColour(0.98f, 1.f, 0.95f);
	Sunlight->setSpecularColour(1.f, 1.f, 1.f);

	if(!SunLightNode){

		SunLightNode = WorldsScene->getRootSceneNode()->createChildSceneNode("sunlight node");

		SunLightNode->attachObject(Sunlight);
	}

	Ogre::Quaternion quat;
	quat.FromAngleAxis(Ogre::Radian(1.f), Float3(0.55f, -0.3f, 0.75f));
	SunLightNode->setOrientation(quat);
}

DLLEXPORT void Leviathan::GameWorld::RemoveSunlight(){
	if(SunLightNode){
		SunLightNode->detachAllObjects();
		// might be safe to delete
		OGRE_DELETE SunLightNode;
		SunLightNode = NULL;
	}
}

DLLEXPORT void Leviathan::GameWorld::UpdateCameraLocation(int mspassed, ViewerCameraPos* camerapos){
	// update camera //
	camerapos->UpdatePos(mspassed);

	// set camera position //
	CameraLocationNode->setPosition(camerapos->GetPosition());

	// convert rotation into a quaternion //
	const Float3& angles = camerapos->GetRotation();

	// create quaternion from quaternion rotations around each axis //
	Ogre::Quaternion rotq(Ogre::Degree(angles.Y), Ogre::Vector3::UNIT_X);
	Ogre::Quaternion rotyaw(Ogre::Degree(angles.X), Ogre::Vector3::UNIT_Y);
	Ogre::Quaternion rotroll(Ogre::Degree(angles.Z), Ogre::Vector3::UNIT_Z);

	rotq = rotyaw*rotq*rotroll;

	WorldSceneCamera->setOrientation(rotq);
}
// ------------------ Object managing ------------------ //
DLLEXPORT void Leviathan::GameWorld::AddObject(BaseObject* obj){
	AddObject(shared_ptr<BaseObject>(obj));
}

DLLEXPORT void Leviathan::GameWorld::AddObject(shared_ptr<BaseObject> obj){
	Objects.push_back(obj);
}

DLLEXPORT shared_ptr<BaseObject> Leviathan::GameWorld::GetWorldObject(int ID){
	for(std::vector<shared_ptr<BaseObject>>::iterator iter = Objects.begin(); iter != Objects.end(); ++iter){
		if((*iter)->GetID() == ID){
			return *iter;
		}
	}
	return nullptr;
}


DLLEXPORT void Leviathan::GameWorld::ClearObjects(){
	for(std::vector<shared_ptr<BaseObject>>::iterator iter = Objects.begin(); iter != Objects.end(); ++iter){
		// TODO: add world unlink function to BaseObject
	}
	Objects.clear();
}


DLLEXPORT Float3 Leviathan::GameWorld::GetGravityAtPosition(const Float3 &pos){
	// TODO: take position into account //
	// create force without mass applied //
	return Float3(0.f, PHYSICS_BASE_GRAVITY, 0.f);
}




DLLEXPORT void Leviathan::GameWorld::SimulateWorld(){
	
	_PhysicalWorld->SimulateWorld();
}

DLLEXPORT void Leviathan::GameWorld::ClearSimulatePassedTime(){
	_PhysicalWorld->ClearTimers();
}

DLLEXPORT void Leviathan::GameWorld::DestroyObject(int ID){
	for(std::vector<shared_ptr<BaseObject>>::iterator iter = Objects.begin(); iter != Objects.end(); ++iter){
		if((*iter)->GetID() == ID){
			// release the object and then erase our reference //
			(*iter)->Release();
			Objects.erase(iter);
			return;
		}
	}
}


