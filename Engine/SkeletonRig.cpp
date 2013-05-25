#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SKELETONRIG
#include "SkeletonRig.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
DLLEXPORT Leviathan::GameObject::SkeletonRig::SkeletonRig() : PlayingAnimation(NULL), VerticeTranslationMatrices(), RigsBones(), BoneGroups(){

	StopOnNextFrame = false;
	UseTranslatedPositions = false;
}



DLLEXPORT Leviathan::GameObject::SkeletonRig::~SkeletonRig(){

}
DLLEXPORT void Leviathan::GameObject::SkeletonRig::Release(){
	// stop playing animation //
	KillAnimation();
}
//void Leviathan::GameObject::SkeletonRig::ReleaseBuffers(){
//
//}
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

	if(PlayingAnimation.get() != NULL){

		// using animation //
		UseTranslatedPositions = true;

		// update current positions //
		PlayingAnimation->UpdateAnimations(mspassed);
	}

	// resize matrices if apropriate //
	if(VerticeTranslationMatrices.size() != RigsBones.size()+1){
		// resize to proper value //
		ResizeMatriceCount(RigsBones.size()+1);
	}

	for(unsigned int i = 0; i < VerticeTranslationMatrices.size(); i++){
		if(VerticeTranslationMatrices[i].get() == NULL){
			// needs a new matrice //
			VerticeTranslationMatrices[i] = shared_ptr<D3DXMATRIX>(new D3DXMATRIX());
		}
		// clear matrix //
		D3DXMatrixIdentity(VerticeTranslationMatrices[i].get());

		// fetch new translation //
		SkeletonBone* curbone = NULL;

		// find bone with this index //
		for(unsigned int ind = 0; ind < RigsBones.size(); ind++){
			if(RigsBones[ind].get()->BoneGroup == (int)i){
				// group matches translation index //
				curbone = RigsBones[ind].get();
			}
		}

		if(curbone != NULL){
			// transform //
			Float3* pos = NULL;
			if(UseTranslatedPositions){
				pos = &curbone->AnimationPosition;
			} else {
				// display in rest pose //
				pos = &curbone->RestPosition;
			}
			// translate matrix //
			D3DXMatrixTranslation(VerticeTranslationMatrices[i].get(), pos->X(), pos->Y(), pos->Z());
		}

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
	// used to release memory even in case of exceptions //
	unique_ptr<SkeletonRig> CreatedRig = unique_ptr<SkeletonRig>(new SkeletonRig());

	// go through each line and load the bone on that line //
	for(unsigned int i = 0; i < structure->Lines.size(); i++){
		// split the line //
		vector<wstring*> LineParts;

		LineTokeNizer::TokeNizeLine(*structure->Lines[i], LineParts);

		shared_ptr<SkeletonBone> CurrentBone(new SkeletonBone());

		// process line parts //
		for(unsigned int ind = 0; ind < LineParts.size(); ind++){
			// check what part of definition this is (they can be in any order) //
			if(Misc::WstringStartsWith(*LineParts[ind], L"name")){
				// get text between quotation marks //
				unique_ptr<WstringIterator> Iterator(new WstringIterator(LineParts[ind], false));

				// get wstring that is in quotes //
				unique_ptr<wstring> bonename = Iterator->GetStringInQuotes(QUOTETYPE_BOTH);

				// set name //
				CurrentBone->SetName(*bonename);

			} else if (Misc::WstringStartsWith(*LineParts[ind], L"pos")){
				// split to numbers //
				vector<wstring> Values;
				LineTokeNizer::SplitTokenToValues(*LineParts[ind], Values);

				if(Values.size() != 3){
					// somethings wrong //
					Logger::Get()->Error(L"SkeletonRig: LoadFromFileStructure: invalid number of position elements (3) expected ", Values.size());
					continue;
				}

				// Generate Float3 from elements //
				Float3 Coordinates(Convert::WstringToFloat(Values[0]), Convert::WstringToFloat(Values[1]), Convert::WstringToFloat(Values[2]));

				if(NeedToChangeCoordinateSystem){
					// swap y and z to convert from blender coordinates to work with  //

					swap(Coordinates.Val[1], Coordinates.Val[2]);
				}

				CurrentBone->SetRestPosition(Coordinates);
			} else if (Misc::WstringStartsWith(*LineParts[ind], L"group")){

				// create iterator for string //
				unique_ptr<WstringIterator> Iterator(new WstringIterator(LineParts[ind], false));

				// get name //
				unique_ptr<wstring> name = Iterator->GetStringInQuotes(QUOTETYPE_DOUBLEQUOTES);
				unique_ptr<wstring> idstr = Iterator->GetNextNumber(DECIMALSEPARATORTYPE_DOT);

				int id = Convert::WstringToInt(*idstr.get());

				// set group id //
				CurrentBone->SetBoneGroup(id);
				// verify that the group exists in the rig //
				CreatedRig->VerifyBoneGroupExist(id, *name);

			} else if (Misc::WstringStartsWith(*LineParts[ind], L"dir")){
				// split to numbers //
				vector<wstring> Values;
				LineTokeNizer::SplitTokenToValues(*LineParts[ind], Values);

				if(Values.size() != 3){
					// somethings wrong //
					Logger::Get()->Error(L"SkeletonRig: LoadFromFileStructure: invalid number of direction elements (3) expected ", Values.size());
					continue;
				}

				// Generate Float3 from elements //
				Float3 Direction(Convert::WstringToFloat(Values[0]), Convert::WstringToFloat(Values[1]), Convert::WstringToFloat(Values[2]));

				if(NeedToChangeCoordinateSystem){
					// swap y and z to convert from blender coordinates to work with  //

					swap(Direction.Val[1], Direction.Val[2]);
				}

				//CurrentBone->SetRestPosition(Direction);
			} else if (Misc::WstringStartsWith(*LineParts[ind], L"parent")){
				// create iterator for string //
				unique_ptr<WstringIterator> Iterator(new WstringIterator(LineParts[ind], false));

				// get name //
				unique_ptr<wstring> parent = Iterator->GetStringInQuotes(QUOTETYPE_DOUBLEQUOTES);

				// find the parent bone //
				weak_ptr<SkeletonBone> ParentBone;

				for(unsigned int bind = 0; bind < CreatedRig->RigsBones.size(); bind++){
					if(CreatedRig->RigsBones[bind]->Name == *parent){
						ParentBone = weak_ptr<SkeletonBone>(CreatedRig->RigsBones[bind]);
					}
				}
				
				CurrentBone->ParentName = *parent;
				CurrentBone->Parent = ParentBone;

			} else {
				// unknown definition //
				DEBUG_BREAK;
				Logger::Get()->Warning(L"SkeletonRig: LoadFromFileStructure: unknown definition on line :"+*LineParts[ind], false);
			}
		}
		// add bone to list //
		CreatedRig->RigsBones.push_back(CurrentBone);

		// release memory //
		SAFE_DELETE_VECTOR(LineParts);
	}

	// verify bone's parents //
	for(unsigned int bind = 0; bind < CreatedRig->RigsBones.size(); bind++){
		if((CreatedRig->RigsBones[bind]->Parent.lock().get() == NULL) && (CreatedRig->RigsBones[bind]->ParentName.size() > 0)){
			// find parent //
			for(unsigned int aind = 0; aind < CreatedRig->RigsBones.size(); aind++){
				if(CreatedRig->RigsBones[aind]->Name == CreatedRig->RigsBones[bind]->ParentName){
					CreatedRig->RigsBones[bind]->Parent = weak_ptr<SkeletonBone>(CreatedRig->RigsBones[aind]);
				}
			}
		}
	}

	SkeletonRig* tmp = CreatedRig.get();
	// hopefully not delete it by accident here... (release should work) //
	CreatedRig.release();
	return tmp;
}

// ------------------------------------ //
DLLEXPORT bool Leviathan::GameObject::SkeletonRig::SaveOnTopOfTextBlock(ObjectFileTextBlock* block){
	// clear existing data //
	SAFE_DELETE_VECTOR(block->Lines);


	// create new //
	for(unsigned int i = 0; i < RigsBones.size(); i++){
		wstring* curstr = new wstring();
		block->Lines.push_back(curstr);

		SkeletonBone* bone = RigsBones[i].get();

		// data //
		(*curstr) += L"name(\""+bone->Name+L"\") ";

		// check for parent //
		shared_ptr<SkeletonBone> parentbone(bone->Parent.lock());

		if(parentbone.get() != NULL){
			// has parent, save it //
			(*curstr) += L"parent(\""+parentbone->Name+L"\") ";
		}
		parentbone.reset();

		(*curstr) += L"pos("+Convert::ToWstring(bone->RestPosition[0])+L","+Convert::ToWstring(bone->RestPosition[1])+L","
			+Convert::ToWstring(bone->RestPosition[2])+L") ";
		//(*curstr) += L"post("+Convert::ToWstring(bone->RestDir[0])+L","+Convert::ToWstring(bone->RestDir[1])+L","
		//	+Convert::ToWstring(bone->RestDir[2])+L") ";

		// look for bone group name //
		wstring BoneGroupName = L"INVALID";

		for(unsigned int ind = 0; ind < BoneGroups.size(); ind++){
			if(BoneGroups[ind]->Value == bone->BoneGroup){
				BoneGroupName = *BoneGroups[i]->Wstr;
				break;
			}
		}

		(*curstr) += L"group(\""+BoneGroupName+L"\", "+Convert::IntToWstring(bone->BoneGroup)+L")";
	}


	// done //
	return true;
}
// ------------------------------------ //
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
		DEBUG_BREAK;
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
		*(Buffersdata+i) = *VerticeTranslationMatrices[i];
	}

	return true;
}

DLLEXPORT int Leviathan::GameObject::SkeletonRig::GetBoneCount(){
	return VerticeTranslationMatrices.size();
}

DLLEXPORT bool Leviathan::GameObject::SkeletonRig::VerifyBoneGroupExist(int ID, const wstring &name){
	for(unsigned int i = 0; i < BoneGroups.size(); i++){
		if(BoneGroups[i]->GetValue() == ID){
			return true;
		}
	}
	// needs to create new //
	BoneGroups.push_back(shared_ptr<IntWstring>(new IntWstring(name, ID)));
	return true;
}
// --------------- Animation ----------------- //
DLLEXPORT bool Leviathan::GameObject::SkeletonRig::StartPlayingAnimation(shared_ptr<AnimationMasterBlock> Animation){
	if(PlayingAnimation.get() != NULL){
		// needs to kill old animation //
		KillAnimation();
	}

	// verify if can hook into animation //


	//if(false){
	//	// cannot get into animation //
	//	return false;
	//}




	// store animation //
	PlayingAnimation = Animation;
	// register values //

	// hook bones //
	PlayingAnimation->HookBones(RigsBones);
	return true;
}

DLLEXPORT void Leviathan::GameObject::SkeletonRig::KillAnimation(){
	// unregister variables //

	// unreference animation //
	PlayingAnimation = NULL;

	// unhook bones from animation //
}

DLLEXPORT shared_ptr<AnimationMasterBlock> Leviathan::GameObject::SkeletonRig::GetAnimation(){
	return PlayingAnimation;
}




