#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASEMODELDATAOBJECT
#include "BaseModelDataObject.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
#include "BumpModelData.h"
#include "NormalModelData.h"

DLLEXPORT Leviathan::GameObject::BaseModelDataObject::BaseModelDataObject(){
	Type = MODELOBJECT_MODEL_TYPE_ERROR;

	IndexBuffer = NULL;
	VertexBuffer = NULL;

	IsLoaded = false;
	IsBufferDone = false;
	ThisName = L"";
}

DLLEXPORT Leviathan::GameObject::BaseModelDataObject::~BaseModelDataObject(){
	SAFE_RELEASE(VertexBuffer);
	SAFE_RELEASE(IndexBuffer);
}
// ------------------------------------ //
DLLEXPORT MODELOBJECT_MODEL_TYPE Leviathan::GameObject::BaseModelDataObject::GetType(){
	return Type;
}
// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT BaseModelDataObject* Leviathan::GameObject::BaseModelDataObject::CreateRequiredModelObject(MultiFlag* modelfileflags){
	if(modelfileflags->IsSet(FLAG_GOBJECT_MODEL_TYPE_BUMP)){
		// bump model //
		return new BumpModelData();
	} else if(modelfileflags->IsSet(FLAG_GOBJECT_MODEL_TYPE_NORMAL)){
		// normal model //
		return new NormalModelData();
	} else {
		Logger::Get()->Error(L"GameObject::BaseModelDataObject::CreateRequiredModelObject: invalid flags, didn't match any type");
		return NULL;
	}
}

DLLEXPORT int Leviathan::GameObject::BaseModelDataObject::GetIndexCount() const{
	return IndexCount;
}

DLLEXPORT  SkeletonRig* Leviathan::GameObject::BaseModelDataObject::GetSkeleton(){
	return NULL;
}


// ------------------------------------ //






