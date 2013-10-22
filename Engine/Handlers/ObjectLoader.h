#ifndef LEVIATHAN_OBJECTLOADER
#define LEVIATHAN_OBJECTLOADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ObjectFiles\ObjectFileProcessor.h"
#include "FileSystem.h"
#include "Rendering\TextureDefinition.h"
#include "Entities\GameWorld.h"
#include "Entities\Objects\Brush.h"


namespace Leviathan{

	// forward declaration //
	class Engine;

	class ObjectLoader : public Object{
	public:
		DLLEXPORT ObjectLoader::ObjectLoader(Engine* engine);


		DLLEXPORT int LoadPropToWorld(GameWorld* world, const wstring &name);
		// creates a brush with physical component and sets mass (use 0.f for static object) //
		DLLEXPORT int LoadBrushToWorld(GameWorld* world, const string &material, const Float3 &size, const float &mass);
		// same as above but no physics initialization (you must do your own if you want this brush to interact with objects) //
		DLLEXPORT int LoadBrushToWorld(GameWorld* world, const string &material, const Float3 &size);

		DLLEXPORT void CreateTestCubeToScene(Ogre::SceneManager* scene, string meshname);
		DLLEXPORT void AddTestCubeToScenePositions(Ogre::SceneManager* scene, vector<Float3> &positions, const string &meshname);

	private:
		Engine* m_Engine;

	};

}
#endif