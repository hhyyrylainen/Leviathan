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
	class AnimationMasterBlock;
namespace GameObject{

	class SkeletonBone : public BaseObject{
		// friend to skeleton rig for sake of keeping it easy //
		friend class SkeletonRig;
		friend AnimationMasterBlock;
	public:
		DLLEXPORT SkeletonBone::SkeletonBone();
		DLLEXPORT SkeletonBone::SkeletonBone(const wstring &name, const Float3 &position, const Float4 &direction, int group);
		DLLEXPORT SkeletonBone::~SkeletonBone();

		DLLEXPORT void SetName(const wstring &name);
		DLLEXPORT void SetRestPosition(const Float3 &val);
		DLLEXPORT void SetRestDirection(const Float4 &val);
		DLLEXPORT void SetAnimationPosition(const Float3 &val);
		DLLEXPORT void SetAnimationDirection(const Float4 &val);
		DLLEXPORT void SetBoneGroup(int group);
		DLLEXPORT void SetParentName(const wstring &name);
		DLLEXPORT void SetParentPtr(shared_ptr<SkeletonBone> parent, shared_ptr<SkeletonBone> thisptr);
		DLLEXPORT void AddChildren(shared_ptr<SkeletonBone> bone);

		DLLEXPORT void CopyAnimationDataFromOther(const SkeletonBone &other);

		DLLEXPORT Float3& GetRestPosition();
		DLLEXPORT Float4& GetRestDirection();
		DLLEXPORT Float4& GetAnimationDirection();
		DLLEXPORT Float3& GetAnimationPosition();

		DLLEXPORT shared_ptr<D3DXMATRIX> GetInvBindPoseFinalMatrix();
		DLLEXPORT int GetBoneGroup();

		DLLEXPORT void SetPosePosition(const Float3 &pos);

	private:

		shared_ptr<D3DXMATRIX> CalculateInvBindPose();
		// ------------------------------------ //
		wstring Name;
		Float3 RestPosition;
		// quaternion rotations //
		Float4 RestDirection;
		Float4 AnimationDirection;

		Float3 AnimationPosition;

		// bone group //
		int BoneGroup;

		shared_ptr<D3DXMATRIX> InvBindComplete;

		weak_ptr<SkeletonBone> Parent;

		vector<weak_ptr<SkeletonBone>> Children;

		wstring ParentName;
	};

}}
#endif