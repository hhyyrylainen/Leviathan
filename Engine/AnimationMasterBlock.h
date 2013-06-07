#ifndef LEVIATHAN_ANIMATIONMASTERBLOCK
#define LEVIATHAN_ANIMATIONMASTERBLOCK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "AnimationBlock.h"
#include "SkeletonBone.h"
#include "EventHandler.h"

#define ANIMATIONMASTERBLOCK_UPDATE_RETURN_FROZEN	2500

namespace Leviathan{

	class AnimationMasterBlock : public CallableObject{
	public:
		DLLEXPORT AnimationMasterBlock::AnimationMasterBlock();
		DLLEXPORT AnimationMasterBlock::~AnimationMasterBlock();

		DLLEXPORT int UpdateAnimations(int mspassed, bool force = false);
		DLLEXPORT void HookBones(vector<shared_ptr<GameObject::SkeletonBone>> &bonestohook);

		DLLEXPORT void OnEvent(Event** pEvent);

	private:

		vector<shared_ptr<AnimationBlock>> Animations;
		vector<shared_ptr<GameObject::SkeletonBone>> HookedBones;

		int AnimationMSPassed;
		bool IsFrozen;
		bool AreAnimationsUpdated;
	};

}
#endif