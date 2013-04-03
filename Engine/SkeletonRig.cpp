#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SKELETONRIG
#include "SkeletonRig.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
DLLEXPORT Leviathan::GameObject::SkeletonRig::SkeletonRig(){
	VerticeTranslationMatrices.resize(10);

	// populate with empty matrices //
	for(unsigned int i = 0; i < VerticeTranslationMatrices.size(); i++){
		VerticeTranslationMatrices[i] = shared_ptr<D3DXMATRIX>(new D3DXMATRIX());
	}

	Translated = 0.f;
}

DLLEXPORT Leviathan::GameObject::SkeletonRig::~SkeletonRig(){

}
DLLEXPORT void Leviathan::GameObject::SkeletonRig::Release(){

}
void Leviathan::GameObject::SkeletonRig::ReleaseBuffers(){

}
// ------------------------------------ //


DLLEXPORT bool Leviathan::GameObject::SkeletonRig::CreateBuffersForRendering(ID3D11Device* device){
	return false;
}

DLLEXPORT bool Leviathan::GameObject::SkeletonRig::UpdateBuffersForRendering(ID3D11DeviceContext* devcont){


	return false;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::GameObject::SkeletonRig::UpdatePose(int mspassed, D3DXMATRIX* WorldMatrix){
	// test update //
	Translated += mspassed*0.00001f;
	
	for(unsigned int i = 0; i < VerticeTranslationMatrices.size(); i++){
		//D3DXMatrixTranslation((VerticeTranslationMatrices[i].get()), Translated, Translated, 0);

		//Float3 parentPos(0.f, 0.f, 0.f);
		//D3DXMATRIX mTranslate;
		//D3DXMatrixTranslation(&mTranslate, parentPos.X(), parentPos.Y(), parentPos.Z());

		//D3DXMATRIX mQuat;
		//D3DXMatrixRotationYawPitchRoll(&mQuat, 0.f, 0.f, 0.f);

		////D3DXMatrixRotationQuaternion( &mQuat, &quat );
		//D3DXMATRIX LocalTransform = ( mQuat * mTranslate );
	
		//// Transform ourselves
		//D3DXMATRIX LocalWorld;
		//D3DXMatrixMultiply(&LocalWorld, &LocalTransform, WorldMatrix);
		//(*VerticeTranslationMatrices[i]) = LocalWorld;
		D3DXMatrixIdentity(VerticeTranslationMatrices[i].get());

		// try multiplying with world matrix //
		//D3DXMatrixMultiply(VerticeTranslationMatrices[i].get(), VerticeTranslationMatrices[i].get(), WorldMatrix);

		//D3DXMatrixTranslation((VerticeTranslationMatrices[i].get()), Translated, Translated, Translated);

		// translate it a bit //
		if(i == 1){
			//D3DXMatrixTranslation((VerticeTranslationMatrices[i].get()), Translated/-1.f, Translated/-1.f, Translated/-1.f);
			//D3DXMatrixTranslation((VerticeTranslationMatrices[i].get()), mspassed*0.01f, mspassed*0.01f,mspassed*0.01f);
		}
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

DLLEXPORT bool Leviathan::GameObject::SkeletonRig::CopyValuesToBuffer(BoneTransformsBufferType* buffer){
	// copy matrices to the buffer //
	for(unsigned int i = 0; i < VerticeTranslationMatrices.size(); i++){
		buffer->BoneMatrices[i] = *VerticeTranslationMatrices[i];
	}

	return true;
}


