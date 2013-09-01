#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ANIMATIONMASTERBLOCK
#include "AnimationMasterBlock.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::AnimationMasterBlock::AnimationMasterBlock(){
	AreAnimationsUpdated = false;
	AnimationMSPassed = 0;
	IsFrozen = false;

	BonesChanged = true;

	// start listening for frame end, to be able to flush the current state when frame ends //
	this->RegisterForEvent(EVENT_TYPE_FRAME_END);
}

DLLEXPORT Leviathan::AnimationMasterBlock::~AnimationMasterBlock(){
	// stop listening //
	this->UnRegister(EVENT_TYPE_FRAME_END, true);
	// release cache //
	SAFE_DELETE_VECTOR(CachedIndexes);
	SAFE_DELETE_VECTOR(CachedStreamsForAnimationBlock);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::AnimationMasterBlock::HookBones(vector<shared_ptr<GameObject::SkeletonBone>> &bonestohook){
	// make sure that there is enough space //
	HookedBones.reserve(HookedBones.size()+bonestohook.size());

	// copy shared ptrs over //
	for(unsigned int i = 0; i < bonestohook.size(); i++){
		HookedBones.push_back(bonestohook[i]);
	}

	// bones have been changed //
	BonesChanged = true;

}
// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT int Leviathan::AnimationMasterBlock::UpdateAnimations(int mspassed, bool force /*= false*/){
	if(IsFrozen){
		// skip //
		return ANIMATIONMASTERBLOCK_UPDATE_RETURN_FROZEN;
	}

	if((AreAnimationsUpdated) && (!force)){
		
		// is already updated this frame and isn't forced //
		return 1;
	}

	// check are streams ok //
	if(!VerifyBoneStreamChannelsExist()){
		// no streams, can't animate //
		DEBUG_BREAK;
		return -1;
	}

	AreAnimationsUpdated = true;
	AnimationMSPassed += mspassed;

	if(AnimationMSPassed > 9000)
		AnimationMSPassed -= 9000;

	// update all animations based on current total time passed //
	// and send bones to animations for their positions to be updated //
	for(unsigned int i = 0; i < Animations.size(); i++){
		if(!Animations[i]->SampleToStreams(this)){

			Logger::Get()->Error(L"AnimationMasterBlock: UpdateAnimations: failed to SampleToStreams on animation index: "+Convert::IntToWstring(i));
			return -1;
		}
	}
	// update them by random amount //


	//for(unsigned int i = 0; i < HookedBones.size(); i++){
	//	if(i != 0){
	//		Float3 dir = HookedBones[i]->GetAnimationDirection();

	//		// set rotation //
	//		HookedBones[i]->SetAnimationDirection(dir+Float3(0, 0, 0.01f*mspassed));

	//		continue;
	//	}
	//	Float3 dir = HookedBones[i]->GetAnimationDirection();

	//	// set rotation //
	//	HookedBones[i]->SetAnimationDirection(dir+Float3(0, 0, 0.01f*mspassed));
	//}


	// animations have updated bone data streams, mix them and update bones //
	return RunMixing();
}


// ------------------------------------ //

DLLEXPORT void Leviathan::AnimationMasterBlock::OnEvent(Event** pEvent){
	// only act on event of type frame end //
	if((*pEvent)->GetType() == EVENT_TYPE_FRAME_END){
		// no longer up to date with bone data //
		AreAnimationsUpdated = false;
	}
}

DLLEXPORT bool Leviathan::AnimationMasterBlock::VerifyBoneStreamChannelsExist(){
	// cache must be good to use this //
	if(!VerifyCache()){

		DEBUG_BREAK;
		return false;
	}

	// check does all channels exist //
	if(DoAllChannelsUpToMaxBonesExist()){
		return true;
	}

	// loop from 0 to max vertex group and add (if doesn't exist) //

	// screw it, clear channels and add all //
	BoneDataStreams.clear();

	// reserve data //
	BoneDataStreams.reserve(MaxVertexGroup);

	// loop through all vertex groups and construct index data //
	for(int i = 0; i <= MaxVertexGroup; i++){

		BoneDataStreams.push_back(shared_ptr<AnimationStream>(new AnimationStream(i)));
	}

	return true;
}

DLLEXPORT bool Leviathan::AnimationMasterBlock::DoAllChannelsUpToMaxBonesExist(){
	// cache must be good to use this //
	if(!VerifyCache()){

		DEBUG_BREAK;
		return false;
	}

	// check does all channels exist //
	int FoundIndex = 0;
	bool Correct = true;

	while(FoundIndex <= MaxVertexGroup){
		bool Found = false;
		for(int i = FoundIndex-1; i < FoundIndex+2; i++){
			if(i < 0)
				i = 0;
			if(FoundIndex >= (int)BoneDataStreams.size())
				break;

			if(BoneDataStreams[i]->GetVertexGroup() == FoundIndex){
				// found //
				Found = true;
				break;
			}
		}
		if(Found){
			// move to next //
			FoundIndex++;
			continue;
		}
		// search all //
		for(size_t i = 0; i < BoneDataStreams.size(); i++){
			if(BoneDataStreams[i]->GetVertexGroup() == FoundIndex){
				// found //
				Found = true;
				break;
			}
		}

		if(Found){
			// move to next //
			FoundIndex++;
			continue;
		}
		// if got here, needs to reconstruct streams //
		Correct = false;
		break;
	}
	// correct is true when no gaps found and false otherwise //
	return Correct;
}

int Leviathan::AnimationMasterBlock::RunMixing(){
	// go through all bone data streams and calculate final positions based on stream data that animation block objects have updated //

	for(size_t i = 0; i < BoneDataStreams.size(); i++){
		// get matching bone(s) //
		BoneGroupFoundIndexes* receivingbones = GetIndexesOfBonesMatchingGroup(i);

		if(receivingbones->IndexesInHookedBones.size() < 1){
			// no bones to receive data, no point in calculating //
			continue;
		}

		// calculate first from the stream //
		Float3 ChangedPosition(0);
		Float4 ChangedDirection(0);
		// use sample function to get total changed position and direction for bone //
		BoneDataStreams[i]->SampleData(ChangedPosition, ChangedDirection);
		
		// set final positions to first index //
		GameObject::SkeletonBone* bone = GetBoneOnIndex(receivingbones->IndexesInHookedBones[0]);

		// add base pos and direction to get final position //
		bone->SetAnimationPosition(bone->GetRestPosition()+ChangedPosition);
		bone->SetAnimationDirection(ChangedDirection.QuaternionMultiply(bone->GetRestDirection()));

		// copy result to other ones //
		for(size_t a = 1; a < receivingbones->IndexesInHookedBones.size(); a++){
			GetBoneOnIndex(receivingbones->IndexesInHookedBones[a])->CopyAnimationDataFromOther(
				*GetBoneOnIndex(receivingbones->IndexesInHookedBones[a-1]));
		}
	}
	// done (2: no error) //
	return 2;
}

DLLEXPORT shared_ptr<AnimationStream> Leviathan::AnimationMasterBlock::GetStreamForVertexGroup(const int &group){
	// loop through all streams and return right one //
	for(size_t i = 0; i < BoneDataStreams.size(); i++){
		if(BoneDataStreams[i]->GetVertexGroup() == group){
			// right one found, return raw pointer to avoid copying smart pointers //
			return BoneDataStreams[i];
		}
	}
	// none found //
	return NULL;
}

DLLEXPORT BoneGroupFoundIndexes* Leviathan::AnimationMasterBlock::GetIndexesOfBonesMatchingGroup(const int &group){
	// check cache //
	if(!VerifyCache()){

		DEBUG_BREAK;
	}

	// cache index should be group-1 //
	size_t startind = (size_t)group;
	if(startind != 0)
		startind -= 1;

	for(size_t i = startind; i < (size_t)group+3; i++){

		if(i >= CachedIndexes.size()){
			// not found //
			return NULL;
		}

		// return if correct //
		if(CachedIndexes[i]->Group == group)
			return CachedIndexes[i];

	}

	// not found, start search index could be improved //
	DEBUG_BREAK;
	return NULL;
}

DLLEXPORT GameObject::SkeletonBone* Leviathan::AnimationMasterBlock::GetBoneOnIndex(const size_t &index){
	ARR_INDEX_CHECKINV(index, HookedBones.size()){
		// out of bound //
		DEBUG_BREAK;

		return NULL;
	}
	return HookedBones[index].get();
}

DLLEXPORT bool Leviathan::AnimationMasterBlock::VerifyCache(){
	// make sure that cache is up to date //
	if(BonesChanged){
		BonesChanged = false;

		// rebuild cache //

		SAFE_DELETE_VECTOR(CachedIndexes);

		MaxVertexGroup = 0;

		for(size_t i = 0; i < HookedBones.size(); i++){
			// increase if over currently max found //
			if(HookedBones[i]->BoneGroup > MaxVertexGroup){

				MaxVertexGroup = HookedBones[i]->BoneGroup;
			}
		}

		// reserve data //
		CachedIndexes.reserve(MaxVertexGroup);

		// loop through all vertex groups and construct index data //
		for(int i = 0; i <= MaxVertexGroup; i++){

			CachedIndexes.push_back(new BoneGroupFoundIndexes(i, ConstructCacheForGroup(i)));
		}


	}
	// cache good to go //
	return true;
}

vector<size_t> Leviathan::AnimationMasterBlock::ConstructCacheForGroup(const int &group){
	// vector for collecting the groups //
	vector<size_t> Result;

	for(size_t i = 0; i < HookedBones.size(); i++){

		if(HookedBones[i]->BoneGroup == group){
			// add //
			Result.push_back(i);
		}
	}

	return Result;
}

DLLEXPORT int Leviathan::AnimationMasterBlock::GetBlockMSPassed(){
	return AnimationMSPassed;
}

DLLEXPORT vector<shared_ptr<AnimationStream>>& Leviathan::AnimationMasterBlock::GetStoredStreamsForVertexGroupList(const int &AnimID, 
	const vector<int> &vgroups)
{
	// find based on id //
	for(size_t i = 0; i < CachedStreamsForAnimationBlock.size(); i++){
		if(CachedStreamsForAnimationBlock[i]->AnimationBlockID == AnimID){

			return CachedStreamsForAnimationBlock[i]->Streams;
		}
	}


	// needs to construct new //
	StreamsForAnimationBlock* tmp = new StreamsForAnimationBlock(AnimID);

	tmp->Streams.reserve(vgroups.size());

	// add streams //
	for(size_t i = 0; i < vgroups.size(); i++){
		// add corresponding stream //
		tmp->Streams.push_back(GetStreamForVertexGroup(vgroups[i]));
	}

	// store to vector //
	CachedStreamsForAnimationBlock.push_back(tmp);
	// return the vectors stored in the object //
	return tmp->Streams;
}

DLLEXPORT bool Leviathan::AnimationMasterBlock::AddAnimationBlock(shared_ptr<AnimationBlock> newblock){
	// add to vector of playing animations //
	Animations.push_back(newblock);
	// can't fail right now //
	return true;
}
