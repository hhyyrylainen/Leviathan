#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Entities/GameWorld.h"



namespace Leviathan{

	//! \brief Class to collect all entity creations to one class
	//! \todo Do a page for entity
	//! \warning The object pointer the load methods return/set is NOT safe to use after a while
    //! (it *should* be safe to call some method on the ptr after
	//! the function returns) But for storing it you need to use GameWorld::GetWorldObject with the returned ID
    //! and store the shared_ptr with the pointer
    //! \note All created objects are added to the world so that they are broadcast on the network (if this is a server)
    //! \todo Make the above statement true for all the functions
    //! \todo Allow objects to be created that will be sent to clients only after the caller has had the chance to set
    //! the position etc.
	class ObjectLoader{
	public:
		DLLEXPORT ObjectLoader(Engine* engine);

		//! \brief Loads prop to a GameWorld
		DLLEXPORT int LoadPropToWorld(GameWorld* world, const std::string &name, int materialid,
            Entity::Prop** createdinstance);
        
		//! \brief Creates a brush with physical component and sets mass
		//! \param mass The mass of the brush, use 0.f for static object
		DLLEXPORT int LoadBrushToWorld(GameWorld* world, const std::string &material,
            const Float3 &size, const float &mass, int materialid, Entity::Brush** createdinstance);
        
		//! \brief Creates a brush, but no physics are initialized
		//! \note To initialize physics later call Entity::Brush::AddPhysicalObject
		DLLEXPORT int LoadBrushToWorld(GameWorld* world, const std::string &material,
            const Float3 &size, Entity::Brush** createdinstance);

		// ------------------ Complex entity loading ------------------ //
		DLLEXPORT int LoadTrackEntityControllerToWorld(GameWorld* world,
            std::vector<Entity::TrackControllerPosition> &initialtrack,
            BaseConstraintable* controllable, Entity::TrackEntityController** createdinstance);

		// Creates a trail entity to a world. Set the dynamic property if you want to update the
        // properties later
		DLLEXPORT int LoadTrailToWorld(GameWorld* world, const std::string &material,
            const Entity::TrailProperties &properties, bool allowupdatelater,
            Entity::TrailEmitter** createdinstance);


		// ------------------ Test object adding ------------------ //
		DLLEXPORT void CreateTestCubeToScene(Ogre::SceneManager* scene, std::string meshname);
		DLLEXPORT void AddTestCubeToScenePositions(Ogre::SceneManager* scene,
            std::vector<Float3> &positions, const std::string &meshname);

	private:
		Engine* m_Engine;

	};

}

