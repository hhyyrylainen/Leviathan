#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_RENDERABLE
#include "BaseRenderable.h"
#endif
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseRenderable::BaseRenderable(bool hidden) : Hidden(hidden), GraphicalObject(NULL), ObjectsNode(NULL){

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
DLLEXPORT void Leviathan::BaseRenderable::SetDefaultSubDefaultPassDiffuse(const Float4 &newdiffuse) throw(...){
	GraphicalObject->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->setDiffuse(newdiffuse);
}

DLLEXPORT void Leviathan::BaseRenderable::SetOgreMaterialName(const string &name){
	GraphicalObject->getSubEntity(0)->setMaterialName(name);
}


