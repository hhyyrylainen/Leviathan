#ifndef LEVIATHAN_OBJECTLOADER
#define LEVIATHAN_OBJECTLOADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Rendering\TextureDefinition.h"
#include "Entities\GameWorld.h"



namespace Leviathan{

	// forward declaration //
	class Engine;
	class BaseNotifiable;

	namespace Entity{
		class Prop;
		class Brush;
		struct TrackControllerPosition;
		class TrackEntityController;
		struct TrailProperties;
		class TrailEmitter;
	}


	class ObjectLoader : public Object{
	public:
		DLLEXPORT ObjectLoader::ObjectLoader(Engine* engine);


		// Note: the object pointer the load methods set is NOT safe to use after a while (it *should* be safe to call some method on the ptr after
		// the function returns 

		DLLEXPORT int LoadPropToWorld(GameWorld* world, const wstring &name, Entity::Prop** createdinstance);
		// creates a brush with physical component and sets mass (use 0.f for static object) //
		DLLEXPORT int LoadBrushToWorld(GameWorld* world, const string &material, const Float3 &size, const float &mass, Entity::Brush** createdinstance);
		// same as above but no physics initialization (you must do your own if you want this brush to interact with objects) //
		DLLEXPORT int LoadBrushToWorld(GameWorld* world, const string &material, const Float3 &size, Entity::Brush** createdinstance);

		// ------------------ Complex entity loading ------------------ //
		DLLEXPORT int LoadTrackEntityControllerToWorld(GameWorld* world, vector<Entity::TrackControllerPosition> &initialtrack, BaseNotifiable* controllable, 
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