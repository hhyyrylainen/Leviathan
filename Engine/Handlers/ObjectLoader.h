#ifndef LEVIATHAN_OBJECTLOADER
#define LEVIATHAN_OBJECTLOADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/GameWorld.h"



namespace Leviathan{

	//! \brief Class to collect all entity creations to one class
	//! \todo Do a page for entity
	//! \warning The object pointer the load methods return/set is NOT safe to use after a while (it *should* be safe to call some method on the ptr after
	//! the function returns) But for storing it you need to use GameWorld::GetWorldObject with the returned ID and store the shared_ptr with the pointer
	class ObjectLoader : public Object{
	public:
		DLLEXPORT ObjectLoader(Engine* engine);

		//! \brief Loads prop to a GameWorld
		DLLEXPORT int LoadPropToWorld(GameWorld* world, const wstring &name, Entity::Prop** createdinstance);
		//! \brief Creates a brush with physical component and sets mass
		//! \param mass The mass of the brush, use 0.f for static object
		DLLEXPORT int LoadBrushToWorld(GameWorld* world, const string &material, const Float3 &size, const float &mass, Entity::Brush** createdinstance);
		//! \brief Creates a brush, but no physics are initialized
		//! \note To initialize physics later call FINDTHIS
		//! \todo Finish this description
		DLLEXPORT int LoadBrushToWorld(GameWorld* world, const string &material, const Float3 &size, Entity::Brush** createdinstance);

		// ------------------ Complex entity loading ------------------ //
		DLLEXPORT int LoadTrackEntityControllerToWorld(GameWorld* world, vector<Entity::TrackControllerPosition> &initialtrack, BaseNotifiableEntity* controllable,
			Entity::TrackEntityController** createdinstance);

		// Creates a trail entity to a world. Set the dynamic property if you want to update the properties later //
		DLLEXPORT int LoadTrailToWorld(GameWorld* world, const string &material, const Entity::TrailProperties &properties, bool allowupdatelater,
			Entity::TrailEmitter** createdinstance);


		// ------------------ Test object adding ------------------ //
		DLLEXPORT void CreateTestCubeToScene(Ogre::SceneManager* scene, string meshname);
		DLLEXPORT void AddTestCubeToScenePositions(Ogre::SceneManager* scene, vector<Float3> &positions, const string &meshname);

	private:
		Engine* m_Engine;

	};

}
#endif
