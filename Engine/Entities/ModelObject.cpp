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
void Leviathan::GameObject::Model::SetTexturesToLoad(vector<shared_ptr<wstring>> files, MultiFlag flags){
	// if old exist, clear them //
	TexturePath.clear();
	TextureIDS.clear();
	TextureTypes.clear();

	// copy files to texture path vector //
	TexturePath.clear();
	for(unsigned int i = 0; i < files.size(); i++){
		TexturePath.push_back(files[i]);
	}
	// copy flags for later usage //

	vector<shared_ptr<Flag>> ptrflags = flags.GetFlags();
	for(unsigned int i = 0; i < ptrflags.size(); i++){
		TextureTypes.push_back(ptrflags[i]->Value);
	}
}
void Leviathan::GameObject::Model::SetModelToLoad(const wstring &file){
	ModelPath = file;
}
void Leviathan::GameObject::Model::CheckTextures(){
	// check textures //
	if(TextureIDS.size() == 0){

		TextureManager* tempman = Graphics::Get()->GetTextureManager();

		// need to "reload" textures //
		for(unsigned int i = 0; i < TexturePath.size(); i++){
			// add it to texture manager //
			int id = tempman->LoadTexture(*TexturePath[0], TEXTURETYPE_NORMAL, false);
			TextureIDS.push_back(id);
		}
	}

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameObject::Model::Render(Graphics* renderer, int mspassed, const RenderingPassInfo &info){

	// update frame count //
	Frames++;

	SmoothValues();
	// only update things when they are updated //
	if(SmoothUpdated || IsScaleUpdated()){
		SmoothUpdated = false;
		ScaleUpdated = false;


	}
	// make sure that textures are loaded //
	CheckTextures();

	// render //


	return false;
}
// ------------------------------------ //

// ---------------- // static utility // --------------------- //
DLLEXPORT int Leviathan::GameObject::Model::GetFlagFromTextureTypeName(const wstring &name){
	// switch on the name and return corresponding value from MultiFlag.h file "
	if(name == L"CheckRequired"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_UNKOWN;
	}
	if(name == L"NormalMap"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_NORMAL;
	}
	if(name == L"BumpMap"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_BUMP;
	}
	if(name == L"LightMap"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_LIGHT;
	}
	if(name == L"BlendMap"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_BLENDMAP;
	}
	if(name == L"NormalTexture"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_NORMAL;
	}
	if(name == L"BumpTexture"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_BUMP;
	}
	if(name == L"LightTexture"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_LIGHT;
	}
	if(name == L"BlendTexture"){

		return FLAG_GOBJECT_MODEL_TEXTURETYPE_BLENDMAP;
	}

	// -1 signals for error //
	return -1;
}

DLLEXPORT wstring Leviathan::GameObject::Model::TextureFlagToTypeName(int flag){
	// switch on the flag and return corresponding name from GetFlagFromTextureTypeName function "

	if(flag == FLAG_GOBJECT_MODEL_TEXTURETYPE_NORMAL){

		return L"NormalMap";
	}
	if(flag == FLAG_GOBJECT_MODEL_TEXTURETYPE_BUMP){

		return L"BumpMap";
	}
	if(flag == FLAG_GOBJECT_MODEL_TEXTURETYPE_LIGHT){

		return L"LightMap";
	}
	if(flag == FLAG_GOBJECT_MODEL_TEXTURETYPE_BLENDMAP){

		return L"BlendMap";
	}

	// CheckRequired signals for error //
	return L"CheckRequired";
}
