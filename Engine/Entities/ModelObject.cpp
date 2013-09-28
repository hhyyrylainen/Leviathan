#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECT_MODEL
#include "ModelObject.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
#include "Engine.h"
#include "Rendering\Graphics.h"
#include <boost\assign\list_of.hpp>

Leviathan::GameObject::Model::Model(){
	ID = IDFactory::GetID();

	Type = OBJECT_TYPE_MODEL;

	// set starting values //

	Inited = false;

	Hidden = false;
}
Leviathan::GameObject::Model::~Model(){
	Release();
}
// ------------------------------------ //
bool Leviathan::GameObject::Model::Init(){
	// don't actually load anything //

	return true;
}
void Leviathan::GameObject::Model::Release(){

}
// ------------------------------------ //
void Leviathan::GameObject::Model::SetModelToLoad(const wstring &file){
	ModelPath = file;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameObject::Model::Render(Graphics* renderer, int mspassed){

	// update frame count //
	Frames++;

	SmoothValues();
	// only update things when they are updated //
	if(SmoothUpdated || IsScaleUpdated()){
		SmoothUpdated = false;
		ScaleUpdated = false;


	}

	// render //


	return false;
}
// ------------------------------------ //

