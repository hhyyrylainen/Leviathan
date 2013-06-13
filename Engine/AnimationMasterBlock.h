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
#include "AnimationStream.h"

#define ANIMATIONMASTERBLOCK_UPDATE_RETURN_FROZEN	2500

namespace Leviathan{

	// used for caching found results to reduce time on forthcoming frames //
	struct BoneGroupFoundIndexes{
		// 
		BoneGroupFoundIndexes(int group, const vector<size_t> &indexes) : IndexesInHookedBones(indexes){
			// set //
			Group = group;
		};

		int Group;
		vector<size_t> IndexesInHookedBones;
	};

	class AnimationMasterBlock : public CallableObject{
	public:
		DLLEXPORT AnimationMasterBlock::AnimationMasterBlock();
		DLLEXPORT AnimationMasterBlock::~AnimationMasterBlock();

		DLLEXPORT int UpdateAnimations(int mspassed, bool force = false);
		DLLEXPORT void HookBones(vector<shared_ptr<GameObject::SkeletonBone>> &bonestohook);

		DLLEXPORT AnimationStream* GetStreamForVertexGroup(const int &group);
		DLLEXPORT BoneGroupFoundIndexes* GetIndexesOfBonesMatchingGroup(const int &group);
		DLLEXPORT GameObject::SkeletonBone* GetBoneOnIndex(const size_t &index);

		DLLEXPORT void OnEvent(Event** pEvent);

		DLLEXPORT bool VerifyBoneStreamChannelsExist();
		DLLEXPORT bool DoAllChannelsUpToMaxBonesExist();


		DLLEXPORT bool VerifyCache();

	private:
		// updates the hooked bones based on BoneDataStreams //
		int RunMixing();
		vector<size_t> ConstructCacheForGroup(const int &group);

		vector<shared_ptr<AnimationBlock>> Animations;
		vector<shared_ptr<AnimationStream>> BoneDataStreams;
		vector<shared_ptr<GameObject::SkeletonBone>> HookedBones;

		// index cache //
		bool BonesChanged : 1;
		vector<BoneGroupFoundIndexes*> CachedIndexes;
		int MaxVertexGroup;


		int AnimationMSPassed;
		bool IsFrozen : 1;
		bool AreAnimationsUpdated : 1;
	};

}
#endif