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

	namespace Entity{
		class Prop;
		class Brush;
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

		DLLEXPORT void CreateTestCubeToScene(Ogre::SceneManager* scene, string meshname);
		DLLEXPORT void AddTestCubeToScenePositions(Ogre::SceneManager* scene, vector<Float3> &positions, const string &meshname);

	private:
		Engine* m_Engine;

	};

}
#endif