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

	// start listening for frame end, to be able to flush the current state when frame ends //
	this->RegisterForEvent(EVENT_TYPE_FRAME_END);
}

DLLEXPORT Leviathan::AnimationMasterBlock::~AnimationMasterBlock(){
	// stop listening //
	this->UnRegister(EVENT_TYPE_FRAME_END, true);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::AnimationMasterBlock::HookBones(vector<shared_ptr<GameObject::SkeletonBone>> &bonestohook){
	// make sure that there is enough space //
	HookedBones.reserve(HookedBones.size()+bonestohook.size());

	// copy shared ptrs over //
	for(unsigned int i = 0; i < bonestohook.size(); i++){
		HookedBones.push_back(bonestohook[i]);
	}

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

	AreAnimationsUpdated = true;
	AnimationMSPassed += mspassed;

	// update all animations based on current total time passed //
	// and send bones to animations for their positions to be updated //
	for(unsigned int i = 0; i < Animations.size(); i++){

	}
	// update them by random amount //


	for(unsigned int i = 0; i < HookedBones.size(); i++){
		if(i != 0){
			Float3 dir = HookedBones[i]->GetAnimationDirection();

			// set rotation //
			HookedBones[i]->SetAnimationDirection(dir-Float3(0.01f*mspassed, 0, 0));

			continue;
		}
		Float3 dir = HookedBones[i]->GetAnimationDirection();

		// set rotation //
		HookedBones[i]->SetAnimationDirection(dir+Float3(0.f, 0.f, 0.007f*mspassed));
	}

	return 2;
}


// ------------------------------------ //

DLLEXPORT void Leviathan::AnimationMasterBlock::OnEvent(Event** pEvent){
	// only act on event of type frame end //
	if((*pEvent)->GetType() == EVENT_TYPE_FRAME_END){
		// no longer up to date with bone data //
		AreAnimationsUpdated = false;
	}
}




