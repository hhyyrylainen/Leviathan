// ------------------------------------ //
#include "TextureGenerator.h"

#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgreMaterialManager.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT bool Leviathan::TextureGenerator::LoadSolidColourLightMaterialToMemory(const string &name, const Float4 &diffusecolour/*= Float4(1)*/){
	// Create it with ogre material manager //
	Ogre::MaterialManager& manager = Ogre::MaterialManager::getSingleton();

	Ogre::MaterialPtr mat = manager.create(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	if(mat.isNull()){
		// Probably failed to create it //
		return false;
	}
	// Set settings //
	Ogre::Pass* pass = mat->getTechnique(0)->getPass(0);

	pass->setDiffuse(diffusecolour);

	mat->compile();

	return true;
}
// ------------------------------------ //

// ------------------------------------ //
