#ifndef LEVIATHAN_SKELETONBONE
#define LEVIATHAN_SKELETONBONE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseObject.h"

namespace Leviathan{ 
	class LoadedAnimation;
	namespace GameObject{


	class SkeletonLoadingBone /*: public BaseObject*/{
		// friend to skeleton rig for sake of keeping it easy //
		friend class SkeletonRig;
		friend LoadedAnimation;
	public:
		DLLEXPORT SkeletonLoadingBone::SkeletonLoadingBone();
		DLLEXPORT SkeletonLoadingBone::SkeletonLoadingBone(const wstring &name, const Float3 &position, const Float3 &dir, int group);
		DLLEXPORT SkeletonLoadingBone::~SkeletonLoadingBone();

		DLLEXPORT void SetName(const wstring &name);
		DLLEXPORT void SetRestPosition(const Float3 &val);
		DLLEXPORT void SetRestDirection(const Float3 &val);
		//DLLEXPORT void SetAnimationPosition(const Float3 &val);
		DLLEXPORT void SetBoneGroup(int group);
		DLLEXPORT void SetParentName(const wstring &name);

		DLLEXPORT Float3& GetRestPosition();
		//DLLEXPORT void SetPosePosition(const Float3 &pos);
		DLLEXPORT void SetParent(shared_ptr<SkeletonLoadingBone> parent, shared_ptr<SkeletonLoadingBone> thisptr);

	private:
		wstring Name;
		Float3 RestPosition;
		Float3 RestDirection;
		//Float3 AnimationPosition;

		// bone group //
		int BoneGroup;

		weak_ptr<SkeletonLoadingBone> Parent;
		vector<weak_ptr<SkeletonLoadingBone>> Children;
		wstring ParentName;
	};

}}
#endif