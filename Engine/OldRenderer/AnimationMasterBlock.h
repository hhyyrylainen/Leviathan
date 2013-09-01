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

	struct StreamsForAnimationBlock{
		StreamsForAnimationBlock(const int &id){
			AnimationBlockID = id;
		}

		int AnimationBlockID;
		vector<shared_ptr<AnimationStream>> Streams;
	};

	class AnimationMasterBlock : public CallableObject{
	public:
		DLLEXPORT AnimationMasterBlock::AnimationMasterBlock();
		DLLEXPORT AnimationMasterBlock::~AnimationMasterBlock();

		DLLEXPORT int UpdateAnimations(int mspassed, bool force = false);
		DLLEXPORT void HookBones(vector<shared_ptr<GameObject::SkeletonBone>> &bonestohook);

		DLLEXPORT shared_ptr<AnimationStream> GetStreamForVertexGroup(const int &group);
		DLLEXPORT BoneGroupFoundIndexes* GetIndexesOfBonesMatchingGroup(const int &group);
		DLLEXPORT GameObject::SkeletonBone* GetBoneOnIndex(const size_t &index);

		DLLEXPORT void OnEvent(Event** pEvent);

		DLLEXPORT bool AddAnimationBlock(shared_ptr<AnimationBlock> newblock);

		DLLEXPORT bool VerifyBoneStreamChannelsExist();
		DLLEXPORT bool DoAllChannelsUpToMaxBonesExist();

		DLLEXPORT vector<shared_ptr<AnimationStream>>& GetStoredStreamsForVertexGroupList(const int &AnimID, const vector<int> &vgroups);

		DLLEXPORT bool VerifyCache();

		DLLEXPORT int GetBlockMSPassed();

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
		vector<StreamsForAnimationBlock*> CachedStreamsForAnimationBlock;
		int MaxVertexGroup;


		int AnimationMSPassed;
		bool IsFrozen : 1;
		bool AreAnimationsUpdated : 1;
	};

}
#endif