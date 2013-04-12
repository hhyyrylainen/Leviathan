#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SKELETONRIG
#include "SkeletonRig.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
DLLEXPORT Leviathan::GameObject::SkeletonRig::SkeletonRig(){

}



DLLEXPORT Leviathan::GameObject::SkeletonRig::~SkeletonRig(){

}
DLLEXPORT void Leviathan::GameObject::SkeletonRig::Release(){

}
void Leviathan::GameObject::SkeletonRig::ReleaseBuffers(){

}
// ------------------------------------ //
void Leviathan::GameObject::SkeletonRig::ResizeMatriceCount(int newsize){
	// resize //
	VerticeTranslationMatrices.resize(newsize);

	// overwrite everything with NULL pointers //
	for(unsigned int i = 0; i < VerticeTranslationMatrices.size(); i++){
		VerticeTranslationMatrices[i] = NULL;
	}
}

// ------------------------------------ //
DLLEXPORT void Leviathan::GameObject::SkeletonRig::UpdatePose(int mspassed, D3DXMATRIX* WorldMatrix){
	// update currently playing animations //

	// resize matrices if apropriate //

	for(unsigned int i = 0; i < VerticeTranslationMatrices.size(); i++){
		// clear matrix //
		D3DXMatrixIdentity(VerticeTranslationMatrices[i].get());

		// fetch new translation //

		// maybe transpose the matrix //
		D3DXMatrixTranspose(VerticeTranslationMatrices[i].get(), VerticeTranslationMatrices[i].get());
	}
	//// Transform our siblings
	//if( m_pFrameArray[iFrame].SiblingFrame != INVALID_FRAME )
	//	TransformFrame( m_pFrameArray[iFrame].SiblingFrame, pParentWorld, fTime );

	//// Transform our children
	//if( m_pFrameArray[iFrame].ChildFrame != INVALID_FRAME )
	//	TransformFrame( m_pFrameArray[iFrame].ChildFrame, &LocalWorld, fTime );

}

// ------------------------------------ //
DLLEXPORT SkeletonRig* Leviathan::GameObject::SkeletonRig::LoadRigFromFileStructure(ObjectFileTextBlock* structure, bool NeedToChangeCoordinateSystem){
	return new SkeletonRig();
}

// ------------------------------------ //
DLLEXPORT bool Leviathan::GameObject::SkeletonRig::SaveOnTopOfTextBlock(ObjectFileTextBlock* block){
	return false;
}

DLLEXPORT bool Leviathan::GameObject::SkeletonRig::CopyValuesToBuffer(BoneTransfromBufferWrapper* buffer){
	// dynamically cast the wrapper's buffer to right type //
	D3DXMATRIX* Buffersdata = NULL;
	unsigned int MaxCount = 0;
	if(buffer->BoneCount <= MAX_BONES_TINY){
		MaxCount = MAX_BONES_TINY;
		Buffersdata =  (dynamic_cast<BoneTransformsPointerTiny*>(buffer->Data))->Data->BoneMatrices;
	} else if(buffer->BoneCount <= MAX_BONES_SMALL){
		MaxCount = MAX_BONES_SMALL;
		Buffersdata =  (dynamic_cast<BoneTransformsPointerSmall*>(buffer->Data))->Data->BoneMatrices;
	} else if(buffer->BoneCount <= MAX_BONES_MEDIUM){
		MaxCount = MAX_BONES_MEDIUM;
		Buffersdata =  (dynamic_cast<BoneTransformsPointerMedium*>(buffer->Data))->Data->BoneMatrices;
	} else if(buffer->BoneCount <= MAX_BONES_LARGE){
		MaxCount = MAX_BONES_LARGE;
		Buffersdata =  (dynamic_cast<BoneTransformsPointerLarge*>(buffer->Data))->Data->BoneMatrices;
	} else if(buffer->BoneCount <= MAX_BONES_HUGE){
		MaxCount = MAX_BONES_HUGE;
		Buffersdata =  (dynamic_cast<BoneTransformsPointerHuge*>(buffer->Data))->Data->BoneMatrices;
	} else if(buffer->BoneCount <= MAX_BONES_MAX){
		MaxCount = MAX_BONES_MAX;
		Buffersdata =  (dynamic_cast<BoneTransformsPointerMax*>(buffer->Data))->Data->BoneMatrices;
	} else {

		ComplainOnce::PrintErrorOnce(L"skeletonRig: CopyValuesToBuffer: too many bones!", L"SkeletonRig: CopyValuesToBuffer: too many bones! max is "+Convert::IntToWstring(MAX_BONES_MAX)
			+L" bone count: "+Convert::IntToWstring(buffer->BoneCount));
		return false;
	}
	try{
		if(MaxCount < VerticeTranslationMatrices.size() || Buffersdata[0] == NULL){
			return false;
		}
	}
	catch(...){
		DEBUG_BREAK;
		return false;
	}
	// copy matrices to the buffer //
	for(unsigned int i = 0; i < VerticeTranslationMatrices.size(); i++){
		*(Buffersdata+1) = *VerticeTranslationMatrices[i];
	}

	return true;
}

DLLEXPORT int Leviathan::GameObject::SkeletonRig::GetBoneCount(){
	return VerticeTranslationMatrices.size();
}




