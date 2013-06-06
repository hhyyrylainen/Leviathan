#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SKELETONRIG
#include "SkeletonRig.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
using namespace ozz::animation;
using namespace ozz;
// ------------------------------------ //
DLLEXPORT Leviathan::GameObject::SkeletonRig::SkeletonRig() : PlayingAnimation(NULL), VerticeTranslationMatrices(), RigsBones(), BoneGroups(){

	StopOnNextFrame = false;
	//UseTranslatedPositions = false;
}

DLLEXPORT Leviathan::GameObject::SkeletonRig::~SkeletonRig(){
	// release //
	SAFE_DELETE_VECTOR(VerticeTranslationMatrices);
	// skeleton is allocated with ozz's allocator //
	memory::default_allocator().Delete(ModelSkeleton);
}
DLLEXPORT void Leviathan::GameObject::SkeletonRig::Release(){
	// stop playing animation //
	KillAnimation();
}
// ------------------------------------ //
void Leviathan::GameObject::SkeletonRig::ResizeMatriceCount(size_t newsize){
	if(newsize < VerticeTranslationMatrices.size()){
		// delete old //
		SAFE_DELETE_VECTOR(VerticeTranslationMatrices);
	}

	// resize //
	VerticeTranslationMatrices.resize(newsize);

}

// ------------------------------------ //
DLLEXPORT bool Leviathan::GameObject::SkeletonRig::UpdatePose(int mspassed){
	// update currently playing animations //
	if(PlayingAnimation.get() == NULL)
		// this should be handled differently //
		return false;

	// using animation //

	// update current positions //
	PlayingAnimation->UpdateAnimations(mspassed);
	
	// convert precalculated matrices to model space matrices //
	ozz::animation::LocalToModelJob ltmjob;
	ltmjob.skeleton = ModelSkeleton;
	ltmjob.input_begin = PlayingAnimation->TempMatrices.front();
	ltmjob.input_end = PlayingAnimation->TempMatrices.back();
	ltmjob.output_begin = VerticeTranslationMatrices.front();
	ltmjob.output_end = VerticeTranslationMatrices.back();

	if(!ltmjob.Run()){
		DEBUG_BREAK;
		return false;
	}
	// succeeded //
	return true;
}

// ------------------------------------ //
DLLEXPORT SkeletonRig* Leviathan::GameObject::SkeletonRig::LoadRigFromFileStructure(ObjectFileTextBlock* structure, bool NeedToChangeCoordinateSystem){
	// used to release memory even in case of exceptions //
	unique_ptr<SkeletonRig> CreatedRig = unique_ptr<SkeletonRig>(new SkeletonRig());

	// create temporary //
	offline::RawSkeleton rawskeleton;
	// root bone //

	// because ozz uses funky custom allocator, don't directly copy to raw skeleton, but create bone objects and then copy //
	rawskeleton.roots.resize(1);
	offline::RawSkeleton::Joint* root = &rawskeleton.roots[0];

	// go through each line and load the bone on that line //
	for(unsigned int i = 0; i < structure->Lines.size(); i++){
		// split the line //
		vector<wstring*> LineParts;

		LineTokeNizer::TokeNizeLine(*structure->Lines[i], LineParts);

		shared_ptr<SkeletonLoadingBone> CurrentBone(new SkeletonLoadingBone());

		// process line parts //
		for(unsigned int ind = 0; ind < LineParts.size(); ind++){
			// check what part of definition this is (they can be in any order) //
			if(Misc::WstringStartsWith(*LineParts[ind], L"name")){
				// get text between quotation marks //
				WstringIterator itr(LineParts[ind], false);

				// get wstring that is in quotes //
				unique_ptr<wstring> bonename = itr.GetStringInQuotes(QUOTETYPE_BOTH);

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

					swap(Coordinates.Y, Coordinates.Z);
				}

				CurrentBone->SetRestPosition(Coordinates);
			} else if (Misc::WstringStartsWith(*LineParts[ind], L"group")){

				// create iterator for string //
				WstringIterator itr(LineParts[ind], false);

				// get name //
				unique_ptr<wstring> name = itr.GetStringInQuotes(QUOTETYPE_DOUBLEQUOTES);
				unique_ptr<wstring> idstr = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);

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
					DEBUG_BREAK;
					Logger::Get()->Error(L"SkeletonRig: LoadFromFileStructure: invalid number of direction elements (3) expected ", Values.size());
					continue;
				}

				// Generate Float3 from elements //
				Float3 Direction(Convert::WstringToFloat(Values[0]), Convert::WstringToFloat(Values[1]), Convert::WstringToFloat(Values[2]));

				if(NeedToChangeCoordinateSystem){
					// swap y and z to convert from blender coordinates to work with  //

					swap(Direction.Y, Direction.Z);
				}

				CurrentBone->SetRestDirection(Direction);

			} else if (Misc::WstringStartsWith(*LineParts[ind], L"parent")){
				// create iterator for string //
				WstringIterator itr(LineParts[ind], false);

				// get name //
				unique_ptr<wstring> parent = itr.GetStringInQuotes(QUOTETYPE_DOUBLEQUOTES);

				// find the parent bone //
				// not done here anymore //
				
				CurrentBone->ParentName = *parent;
				CurrentBone->Parent.reset();

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
					// so that children are correctly set //
					CreatedRig->RigsBones[bind]->SetParent(CreatedRig->RigsBones[aind], CreatedRig->RigsBones[bind]);
				}
			}
		}
	}


	// build the actual animating skeleton //

	// find root bone //
	for(size_t i = 0; i < CreatedRig->RigsBones.size(); i++){
		// root bone doesn't have a parent, find bone with empty parent name //
		if(CreatedRig->RigsBones[i]->ParentName.size() == 0){
			// root bone found //
			// call the function to copy stuff, it will recurse until all bones are added //
			SetSkeletonLoadingBoneToOzzJoint(CreatedRig->RigsBones[i].get(), root);
			break;
		}
	}

	// verify bone counts //
	if((size_t)rawskeleton.num_joints() != CreatedRig->RigsBones.size()){
		// loading failed //
		DEBUG_BREAK;
	}

	// compile the skeleton //
	offline::SkeletonBuilder skeletonbuilder;

	CreatedRig->ModelSkeleton = skeletonbuilder(rawskeleton);
	// check for fail //
	if(CreatedRig->ModelSkeleton == NULL){
		DEBUG_BREAK;
	}

	// allocate memory for runtime data //

	// no need for helper structs //

	// if not working steal loan code from ozz utils //
	
	CreatedRig->VerticeTranslationMatrices.resize(rawskeleton.num_joints());

	// potentially release all data here (check is it useful later //

	// return the finished rig object //
	// hopefully not delete it by accident here... (release should work) //
	SkeletonRig* tmp = CreatedRig.release();

	return tmp;
}

DLLEXPORT void Leviathan::GameObject::SkeletonRig::SetSkeletonLoadingBoneToOzzJoint(SkeletonLoadingBone* bone, 
	ozz::animation::offline::RawSkeleton::Joint* joint)
{
	// set name //
	joint->name = Convert::WstringToString(bone->Name).c_str();

	// set rest positions //
	joint->transform.scale = math::Float3::one();

	// set rotation, TODO: change exporter to give axis rotations //
	joint->transform.rotation = math::Quaternion::FromEuler(math::Float3(bone->RestDirection.X, bone->RestDirection.Y, bone->RestDirection.Z));

	// and finally set location //
	joint->transform.translation = math::Float3(bone->RestPosition.X, bone->RestPosition.Y, bone->RestPosition.Z);

	// set root children //
	joint->children.resize(bone->Children.size());

	for(size_t a = 0; a < bone->Children.size(); a++){
		// copy child bone data //
		SetSkeletonLoadingBoneToOzzJoint(bone->Children[a].lock().get(), &joint->children[a]);
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameObject::SkeletonRig::SaveOnTopOfTextBlock(ObjectFileTextBlock* block){
	// clear existing data //
	SAFE_DELETE_VECTOR(block->Lines);


	// create new //
	for(unsigned int i = 0; i < RigsBones.size(); i++){
		wstring* curstr = new wstring();
		block->Lines.push_back(curstr);

		SkeletonLoadingBone* bone = RigsBones[i].get();

		// data //
		(*curstr) += L"name(\""+bone->Name+L"\") ";

		// check for parent //
		shared_ptr<SkeletonLoadingBone> parentbone(bone->Parent.lock());

		if(parentbone.get() != NULL){
			// has parent, save it //
			(*curstr) += L"parent(\""+parentbone->Name+L"\") ";
		}
		parentbone.reset();

		(*curstr) += L"pos("+Convert::ToWstring(bone->RestPosition.X)+L","+Convert::ToWstring(bone->RestPosition.Y)+L","
			+Convert::ToWstring(bone->RestPosition.Z)+L") ";
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
		// converting //
		*(Buffersdata+i) = CreateFromOzzFloatMatrix(VerticeTranslationMatrices[i]);
	}

	return true;
}

DLLEXPORT D3DXMATRIX Leviathan::GameObject::SkeletonRig::CreateFromOzzFloatMatrix(ozz::math::Float4x4* matrice){
	// construct matrix from 4x4 matrix //
	math::SimdFloat4* row1 = &matrice->cols[0];
	math::SimdFloat4* row2 = &matrice->cols[0];
	math::SimdFloat4* row3 = &matrice->cols[0];
	math::SimdFloat4* row4 = &matrice->cols[0];
	// huge blob of constructor parameters //
	return D3DXMATRIX(row1->x, row1->y, row1->z, row1->w, 
		row2->x, row2->y, row2->z, row2->w,
		row3->x, row3->y, row3->z, row3->w,
		row4->x, row4->y, row4->z, row4->w);
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
