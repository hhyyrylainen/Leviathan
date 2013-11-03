#ifndef LEVIATHAN_GAMEWORLD
#define LEVIATHAN_GAMEWORLD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Objects\ViewerCameraPos.h"
#include "Newton\PhysicalWorld.h"
#include "Bases\BaseObject.h"


#define PHYSICS_BASE_GRAVITY		-9.81f

namespace Leviathan{

	class GraphicalInputEntity;

	class GameWorld : public Object{
	public:
		DLLEXPORT GameWorld(Ogre::Root* ogre);
		DLLEXPORT ~GameWorld();

		// release to not use Ogre when deleting //
		DLLEXPORT void Release();

		DLLEXPORT void UpdateCameraAspect(GraphicalInputEntity* rendertarget);
		DLLEXPORT void SetFog();
		DLLEXPORT void SetSkyBox(const string &materialname);

		DLLEXPORT void UpdateCameraLocation(int mspassed, ViewerCameraPos* camerapos);

		DLLEXPORT void SetSunlight();
		DLLEXPORT void RemoveSunlight();

		// object managing functions //
		// this takes the object to be deleted by this //
		DLLEXPORT void AddObject(BaseObject* obj);
		DLLEXPORT void AddObject(shared_ptr<BaseObject> obj);
		DLLEXPORT void DestroyObject(int ID);
		DLLEXPORT void QueueDestroyObject(int ID);

		DLLEXPORT shared_ptr<BaseObject> GetWorldObject(int ID);
		// clears all objects from the world //
		DLLEXPORT void ClearObjects();

		// Ogre get functions //
		DLLEXPORT inline Ogre::SceneManager* GetScene(){
			return WorldsScene;
		}
		// physics functions //
		DLLEXPORT inline Float3 GetGravityAtPosition(const Float3 &pos);

		DLLEXPORT inline PhysicalWorld* GetPhysicalWorld(){
			return _PhysicalWorld.get();
		}

		DLLEXPORT void SimulateWorld();
		DLLEXPORT void ClearSimulatePassedTime();
	private:

		void _CreateOgreResources(Ogre::Root* ogre);
		void _HandleDelayedDelete();
		// ------------------------------------ //
		Ogre::Camera* WorldSceneCamera;
		Ogre::SceneNode* CameraLocationNode;
		Ogre::SceneManager* WorldsScene;

		Ogre::Light* Sunlight;
		Ogre::SceneNode* SunLightNode;

		// physics //
		shared_ptr<PhysicalWorld> _PhysicalWorld;

		// objects //
		// TODO: maybe change this to a map //
		std::vector<shared_ptr<BaseObject>> Objects;

		// This vector is used for delayed deletion //
		std::vector<int> DelayedDeleteIDS;
	};

}
#endif