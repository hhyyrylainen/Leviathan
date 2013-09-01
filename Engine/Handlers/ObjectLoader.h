#ifndef LEVIATHAN_OBJECTLOADER
#define LEVIATHAN_OBJECTLOADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities\ModelObject.h"
#include "ObjectFiles\ObjectFileProcessor.h"
#include "FileSystem.h"
#include "Rendering\TextureDefinition.h"

namespace Leviathan{

	// forward declaration //
	class Engine;

	class ObjectLoader : public Object{
	public:
		DLLEXPORT ObjectLoader::ObjectLoader(Engine* engine);

		DLLEXPORT vector<GameObject::Model*> LoadModelFile(const wstring &file, bool finishnow = true);
		DLLEXPORT TextureDefinition* LoadTextureDefinitionFile(const wstring &file);


		DLLEXPORT void CreateTestCubeToScene(Ogre::SceneManager* scene, string meshname);
		DLLEXPORT void AddTestCubeToScenePositions(Ogre::SceneManager* scene, vector<Float3> &positions, const string &meshname);

	private:
		Engine* m_Engine;

	};

}
#endif