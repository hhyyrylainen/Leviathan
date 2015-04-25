// ------------------------------------ //
#include "BaseRenderable.h"

#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreSceneNode.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseRenderable::BaseRenderable(bool hidden) :
    Hidden(hidden), GraphicalObject(NULL), ObjectsNode(NULL)
{

}

DLLEXPORT Leviathan::BaseRenderable::~BaseRenderable(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseRenderable::SetHiddenState(bool hidden){
	Hidden = hidden;
	_OnHiddenStateUpdated();
}

void Leviathan::BaseRenderable::_OnHiddenStateUpdated(){
	// Set scene node visibility //
	if(ObjectsNode)
		ObjectsNode->setVisible(!Hidden);
}
// ------------------------------------ //
DLLEXPORT Ogre::Entity* Leviathan::BaseRenderable::GetOgreEntity(){
	return GraphicalObject;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseRenderable::SetDefaultSubDefaultPassDiffuse(const Float4 &newdiffuse) THROWS{
	GraphicalObject->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->setDiffuse(newdiffuse);
}

DLLEXPORT void Leviathan::BaseRenderable::SetOgreMaterialName(const string &name){
	GraphicalObject->getSubEntity(0)->setMaterialName(name);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseRenderable::SetScale(const Float3 &scale){

    if(ObjectsNode)
        ObjectsNode->setScale(scale.X, scale.Y, scale.Z);
}
