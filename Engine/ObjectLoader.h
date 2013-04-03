#ifndef LEVIATHAN_OBJECTLOADER
#define LEVIATHAN_OBJECTLOADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GeometryAdvancedLoader.h"
#include "ModelObject.h"
#include "ObjectFileProcessor.h"
#include "FileSystem.h"
#include "TextureDefinition.h"

namespace Leviathan{

	// forward declaration //
	class Engine;

	class ObjectLoader : public Object{
	public:
		DLLEXPORT ObjectLoader::ObjectLoader(Engine* engine);
		DLLEXPORT ObjectLoader::~ObjectLoader();
		DLLEXPORT vector<GameObject::Model*> LoadModelFile(const wstring &file, bool finishnow = true);
		DLLEXPORT TextureDefinition* LoadTextureDefinitionFile(const wstring &file);

		//DLLEXPORT vector<GameObject::Model*> LoadModelAssIScene(const aiScene* scene);
	private:
		Engine* m_Engine;

	};

}
#endif