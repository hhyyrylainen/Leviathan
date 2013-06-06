#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ANIMATIONMASTERBLOCK
#include "AnimationMasterBlock.h"
#endif
using namespace Leviathan;
using namespace ozz::animation;
using namespace ozz;
// ------------------------------------ //
DLLEXPORT Leviathan::AnimationMasterBlock::AnimationMasterBlock(shared_ptr<LoadedAnimation> MainAnimation, bool isfrozen){
	AreAnimationsUpdated = false;
	AnimationSecondsPassed = 0.f;
	IsFrozen = isfrozen;
	// needs the bone count to have large enough buffers for skeletons to fetch bone matrices from this animation //
	AnimationBoneCount = MainAnimation->BaseBoneCount;

	// start listening for frame end, to be able to flush the current state when frame ends //
	this->RegisterForEvent(EVENT_TYPE_FRAME_END);

	//memory::Allocator& allocator = ozz::memory::default_allocator();

	// allocate buffers for sampling animations //
	TempMatrices.resize((AnimationBoneCount + 3) / 4);
	Cache.resize(AnimationBoneCount);
}

DLLEXPORT Leviathan::AnimationMasterBlock::~AnimationMasterBlock(){
	// stop listening //
	this->UnRegister(EVENT_TYPE_FRAME_END, true);

	// release memory //
	SAFE_DELETE_VECTOR(TempMatrices);
	SAFE_DELETE_VECTOR(Cache);
	//// cache is initialized with ozz allocator //
	//ozz::memory::default_allocator().Delete(SampledCache);
}
// ------------------------------------ //

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
	AnimationSecondsPassed += mspassed/1000.f;

	// update all animations based on current total time passed //

	// sample animation at current time //
	SamplingJob sampler;
	// set sampler parameters //
	sampler.animation = Animations[0]->RealAnimation;
	sampler.cache = Cache[0];
	sampler.time = AnimationSecondsPassed;
	sampler.output_begin = TempMatrices.front();
	sampler.output_end = TempMatrices.back();

	// run sampling of animation //
	if(!sampler.Run()){
		DEBUG_BREAK;
		return false;
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

DLLEXPORT void Leviathan::AnimationMasterBlock::AddAnimation(shared_ptr<LoadedAnimation> anim){
	Animations.push_back(anim);
}




