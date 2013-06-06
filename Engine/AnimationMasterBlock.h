#ifndef LEVIATHAN_ANIMATIONMASTERBLOCK
#define LEVIATHAN_ANIMATIONMASTERBLOCK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SkeletonLoadingBone.h"
#include "EventHandler.h"
#include "LoadedAnimation.h"

#include "ozz/animation/animation.h"
#include "ozz/animation/offline/animation_builder.h"
#include "ozz/base/maths/quaternion.h"
#include "ozz/animation/sampling_job.h"
#include "ozz/base/maths/soa_transform.h"


#define ANIMATIONMASTERBLOCK_UPDATE_RETURN_FROZEN	2500

namespace Leviathan{

	class AnimationMasterBlock : public CallableObject{
		friend class GameObject::SkeletonRig;
	public:
		DLLEXPORT AnimationMasterBlock::~AnimationMasterBlock();
			DLLEXPORT AnimationMasterBlock::AnimationMasterBlock(shared_ptr<LoadedAnimation> MainAnimation, bool isfrozen = false);
		DLLEXPORT int UpdateAnimations(int mspassed, bool force = false);
		DLLEXPORT void AddAnimation(shared_ptr<LoadedAnimation> anim);

		DLLEXPORT void OnEvent(Event** pEvent);
	private:
		//// private constructors, can only be constructed by animation manager //

		
		// --------------------------------- //
		//vector<shared_ptr<AnimationBlock>> Animations;
		vector<shared_ptr<LoadedAnimation>> Animations;

		//ozz::animation::SamplingCache* SampledCache;

		// used for converting to specific models //
		vector<ozz::math::SoaTransform*> TempMatrices;
		vector<ozz::animation::SamplingCache*> Cache;

		float AnimationSecondsPassed;

		bool IsFrozen;

		int AnimationBoneCount;

		float PlayingSpeed;

		bool AreAnimationsUpdated;
	};

}
#endif