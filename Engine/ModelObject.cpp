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

	Flags = new MultiFlag(DefFlags);

	// set starting values //

	TotallyErrored = false;
	LastError = 0;

	Inited = false;

	NeededShader = MODEL_NEEDED_SHADER_ERROR;

	MType = MODELOBJECT_MODEL_TYPE_NONLOADED;

	ModelDataContainer = NULL;

	XScale = 1.0f;
	YScale = 1.0f;
	ZScale = 1.0f;

	Hidden = false;
}
Leviathan::GameObject::Model::~Model(){
	Release();
}

vector<shared_ptr<Flag>> Leviathan::GameObject::Model::DefFlags = boost::assign::list_of(new Flag(FLAG_GOBJECT_CONTAINS_INIT))
	(new Flag(FLAG_GOBJECT_CONTAINS_RENDER)) (new Flag(FLAG_GOBJECT_TYPE_MODEL));
// ------------------------------------ //
bool Leviathan::GameObject::Model::Init(){
	// don't actually load anything //

	return true;
}
void Leviathan::GameObject::Model::Release(){
	// release vertices
	ReleaseModel();
	// destroy model object //
	SAFE_DELETE(ModelDataContainer);
	SAFE_DELETE(Flags);
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

	CheckTextures();
	//	vector<int> ;
}
void Leviathan::GameObject::Model::SetModelToLoad(wstring &file, MultiFlag flags){
	ModelPath = file;
	// do something with flags //
	ProcessModelFlags(&flags);
}


bool Leviathan::GameObject::Model::ReloadModel(wstring &file, MultiFlag flags, Graphics* loadnow){
	ModelPath = file;

	ProcessModelFlags(&flags);

	if(loadnow != NULL){
		if(!LoadRenderModel(&ModelPath/*, loadnow*/)){
			// failed //
			Logger::Get()->Error(L"GameObject::Model: Render: Failed to load model: model file "+ModelPath, false);
			MType = MODELOBJECT_MODEL_TYPE_ERROR;
			return false;
		}
		if(!InitBuffers(loadnow->GetRenderer()->GetDevice())){
			// failed //
			Logger::Get()->Error(L"GameObject::Model: Render: Failed to init buffers model: model file "+ModelPath, false);
			MType = MODELOBJECT_MODEL_TYPE_ERROR;
			return false;
		}
		Inited = true;
	}
	return false;
}

void Leviathan::GameObject::Model::ProcessModelFlags(MultiFlag* flags){

	ModelDataContainer = BaseModelDataObject::CreateRequiredModelObject(flags);
	if(ModelDataContainer == NULL){
		// not valid flags //
		Logger::Get()->Error(L"GameObject::Model::ProcessModelFlags: invalid flags, not a bump or normal model");
		TotallyErrored = true;
	}
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

	// check for total error //
	if(TotallyErrored || MType == MODELOBJECT_MODEL_TYPE_ERROR){
		return false;
	}
	if((MType == MODELOBJECT_MODEL_TYPE_NONLOADED) || (!Inited)){
		if(LastError == MODEL_ERROR_LOADDATAFAIL){
			TotallyErrored = true;
			return false;
		}

		// try to load //
		if(!LoadRenderModel(&ModelPath/*, renderer*/)){
			// failed //
			Logger::Get()->Error(L"GameObject::Model::Render: Failed to load model: model file "+ModelPath, false);
			MType = MODELOBJECT_MODEL_TYPE_ERROR;
			LastError = MODEL_ERROR_LOADDATAFAIL;
			return false;
		}
		if(!InitBuffers(renderer->GetRenderer()->GetDevice())){
			// failed //
			Logger::Get()->Error(L"GameObject::Model::Render: Failed to init buffers model: model file "+ModelPath, false);
			MType = MODELOBJECT_MODEL_TYPE_ERROR;
			LastError = MODEL_ERROR_LOADDATAFAIL;
			return false;
		}
		Inited = true;
		MType = ModelDataContainer->GetType();
	}

	// update frame count //
	Frames++;

	SmoothValues();
	// only update things when they are updated //
	if(SmoothUpdated || IsScaleUpdated()){
		SmoothUpdated = false;
		ScaleUpdated = false;

		// move values towards real ones //
		OwnWorld = info.GetWorldMatrix();

		// Y and Z are switched in application, this needs to be taken into account here //
		D3DXMATRIX ScaleMatrix = OwnWorld;
		D3DXMatrixScaling(&ScaleMatrix, XScale, ZScale, YScale);

		// generate rotation from orientation data //
		D3DXMatrixRotationYawPitchRoll(&OwnWorld, s_Yaw/3600.f, s_Pitch/3600.f, s_Roll/3600.f);

		// apply scale //
		D3DXMatrixMultiply(&OwnWorld, &OwnWorld, &ScaleMatrix);
		// translate location //
		D3DXMATRIX TranslateMatrix;
		D3DXMatrixTranslation(&TranslateMatrix, (float)s_X/1000, (float)s_Z/1000, (float)s_Y/1000);
		// apply translation //
		D3DXMatrixMultiply(&OwnWorld, &OwnWorld, &TranslateMatrix); 
	}
	// make sure that textures are loaded //
	CheckTextures();

	// render //
	RenderBuffers(renderer->GetRenderer()->GetDeviceContext());

	// check what shader is correct //
	SkeletonRig* skeleton = GetSkeleton();
	if(skeleton){
		// update skeleton pose //
		skeleton->UpdatePose(mspassed);

		// needs a shader that can render the skeleton //


	} else {


	}

	return false;
}
// ------------------------------------ //
void Leviathan::GameObject::Model::SetPos(int x, int y, int z){

	X = x;
	Y = y;
	Z = z;
	PosUpdated();
}
void Leviathan::GameObject::Model::SetOrientation(int pitch, int yaw, int roll){

	Pitch = pitch;
	Yaw = yaw;
	Roll = roll;
	OrientationUpdated();
}
// ------------------------------------ //
void Leviathan::GameObject::Model::PosUpdated(){

}
void Leviathan::GameObject::Model::OrientationUpdated(){

}

// ---------------- // model functions // --------------------- //
void Leviathan::GameObject::Model::ReleaseModel(){
	// release //
	if(ModelDataContainer){
		ModelDataContainer->ReleaseModel();
	} else {
		TotallyErrored = true;
	}
	return;
}
bool Leviathan::GameObject::Model::InitBuffers(ID3D11Device* device, bool allowrecurse /*= true*/){
	if(ModelDataContainer){
		return ModelDataContainer->InitBuffers(device);
	} else {
		TotallyErrored = true;
	}
	return false;
}
void Leviathan::GameObject::Model::RenderBuffers(ID3D11DeviceContext* devcont){
	if(ModelDataContainer){
		ModelDataContainer->RenderBuffers(devcont);
	} else {
		TotallyErrored = true;
	}
}

bool Leviathan::GameObject::Model::LoadRenderModel(wstring* file /* ,Graphics* graph*/){
	if(ModelDataContainer){
		return ModelDataContainer->LoadRenderModel(file);
	} else {
		TotallyErrored = true;
	}
	return false;
}
// ----------------------------- //
int Leviathan::GameObject::Model::GetIndexCount(){
	return ModelDataContainer->GetIndexCount();
}

DLLEXPORT wstring Leviathan::GameObject::Model::GetModelTypeName(){
	return ModelDataContainer->GetModelTypeName();
}

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

DLLEXPORT  wstring Leviathan::GameObject::Model::TextureFlagToTypeName(int flag){
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

DLLEXPORT SkeletonRig* Leviathan::GameObject::Model::GetSkeleton(){
	if(ModelDataContainer){
		return ModelDataContainer->GetSkeleton();
	}
	return NULL;
}

DLLEXPORT bool Leviathan::GameObject::Model::StartPlayingAnimation(shared_ptr<AnimationBlock> Block, bool Smoothtonew /*= false*/){
	// bind the animation to a (new) master block //
	if(CurrentlyPlaying.get() == NULL){
		// new block is required //
		CurrentlyPlaying = shared_ptr<AnimationMasterBlock>(new AnimationMasterBlock());
	}
	// check can new child animation be added //
	bool CanAdd = true;

	if(!CanAdd)
		return false;

	// add animation //
	CurrentlyPlaying->AddAnimationBlock(Block);

	// make sure that skeleton is playing the animation //
	VerifySkeletonPlayingAnimations();

	// now playing the animation //
	return true;
}

DLLEXPORT void Leviathan::GameObject::Model::StopPlayingAnimations(bool KeepCurrentPose /*= false*/){
	if(KeepCurrentPose){
		// set current animation to be saved and then destroyed //


	} else {
		// just reset all animations //
		// set the master block to die //

	}
	// clear pointer //
	CurrentlyPlaying.reset();

	SmoothOutTo.reset();
}

DLLEXPORT void Leviathan::GameObject::Model::FreezeAnimations(){
	// set animation as paused //

}

DLLEXPORT bool Leviathan::GameObject::Model::UnFreezeAnimations(){
	// un pause animation //

	return false;
}

DLLEXPORT bool Leviathan::GameObject::Model::VerifySkeletonPlayingAnimations(){
	// get a skeleton //
	SkeletonRig* rig = this->GetSkeleton();
	if(rig == NULL){

		return false;
	}

	// check is skeleton playing this' objects animation //
	if(rig->GetAnimation().get() != CurrentlyPlaying.get()){
		rig->StartPlayingAnimation(CurrentlyPlaying);
	}

	return false;
}

DLLEXPORT bool Leviathan::GameObject::Model::VerifyResourcesLoaded(Graphics* renderer){
	// try to load //
	if(!LoadRenderModel(&ModelPath/*, renderer*/)){
		// failed //
		Logger::Get()->Error(L"GameObject::Model::Render: Failed to load model: model file "+ModelPath, false);
		MType = MODELOBJECT_MODEL_TYPE_ERROR;
		LastError = MODEL_ERROR_LOADDATAFAIL;
		return false;
	}
	if(!InitBuffers(renderer->GetRenderer()->GetDevice())){
		// failed //
		Logger::Get()->Error(L"GameObject::Model::Render: Failed to init buffers model: model file "+ModelPath, false);
		MType = MODELOBJECT_MODEL_TYPE_ERROR;
		LastError = MODEL_ERROR_LOADDATAFAIL;
		return false;
	}
	Inited = true;
	MType = ModelDataContainer->GetType();
	return true;
}
