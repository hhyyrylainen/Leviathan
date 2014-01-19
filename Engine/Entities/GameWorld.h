#ifndef LEVIATHAN_GAMEWORLD
#define LEVIATHAN_GAMEWORLD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Objects/ViewerCameraPos.h"
#include "Newton/PhysicalWorld.h"
#include "Bases/BaseObject.h"
#include "Common/ReferenceCounted.h"


#define PHYSICS_BASE_GRAVITY		-9.81f

namespace Leviathan{



	// Holds the returned object that was hit during ray casting //
	class RayCastHitEntity : public ReferenceCounted{
	public:
		DLLEXPORT RayCastHitEntity(const NewtonBody* ptr = NULL, const float &tvar = 0.f, RayCastData* ownerptr = NULL);

		DLLEXPORT RayCastHitEntity* operator =(const RayCastHitEntity& other);

		// Compares the hit entity with NULL //
		DLLEXPORT bool HasHit();

		DLLEXPORT Float3 GetPosition();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(RayCastHitEntity);

		DLLEXPORT bool DoesBodyMatchThisHit(NewtonBody* other);

		// Stores the entity, typed as NewtonBody to make sure that user knows what should be compared with this //
		const NewtonBody* HitEntity;
		float HitVariable;
		Float3 HitLocation;
	};

	// Internal object in ray casts //
	struct RayCastData{
		DLLEXPORT RayCastData(int maxcount, const Float3 &from, const Float3 &to);
		DLLEXPORT ~RayCastData();

		// All hit entities that pass checks //
		std::vector<RayCastHitEntity*> HitEntities;
		// Used to stop after certain amount of entities found //
		int MaxCount;
		// Used to efficiently calculate many hit locations //
		Float3 BaseHitLocationCalcVar;
	};


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

		// Casts a ray from point along a vector and returns the first physical object it hits //
		// Warning: you need to call Release on the returned object once done //
		DLLEXPORT RayCastHitEntity* CastRayGetFirstHit(const Float3 &from, const Float3 &to);


		// object managing functions //
		// this takes the object to be deleted by this //
		DLLEXPORT void AddObject(BaseObject* obj);
		// The smart pointer should have custom deleter to use Release //
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
		DLLEXPORT Float3 GetGravityAtPosition(const Float3 &pos);

		DLLEXPORT inline PhysicalWorld* GetPhysicalWorld(){
			return _PhysicalWorld.get();
		}

		DLLEXPORT void SetWorldPhysicsFrozenState(bool frozen);

		DLLEXPORT void SimulateWorld();
		DLLEXPORT void ClearSimulatePassedTime();

		// Ray callbacks //
		static dFloat RayCallbackDataCallbackClosest(const NewtonBody* const body, const NewtonCollision* const shapeHit, const dFloat* const hitContact, const dFloat* const hitNormal, dLong collisionID, void* const userData, dFloat intersectParam);

		// Script proxies //
		DLLEXPORT RayCastHitEntity* CastRayGetFirstHitProxy(Float3 from, Float3 to);

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

		// The world can be frozen to stop physics //
		bool WorldFrozen;
		bool GraphicalMode;

		// objects //
		// \todo maybe change this to a map //
		std::vector<shared_ptr<BaseObject>> Objects;

		// This vector is used for delayed deletion //
		std::vector<int> DelayedDeleteIDS;
	};

}
#endif
