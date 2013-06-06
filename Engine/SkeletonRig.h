#ifndef LEVIATHAN_SKELETONRIG
#define LEVIATHAN_SKELETONRIG
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseObject.h"
#include "ObjectFileTextBlock.h"
#include "Rendering\ShaderDataTypes.h"
#include "ComplainOnce.h"
#include "LineTokenizer.h"

#include "ozz/animation/skeleton.h"
#include "ozz/animation/sampling_job.h"
#include "ozz/animation/local_to_model_job.h"
#include "ozz/animation/offline/skeleton_builder.h"

#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/vec_float.h"
#include "ozz/base/maths/quaternion.h"
#include "ozz/base/maths/soa_transform.h"

#include "SkeletonLoadingBone.h"
#include "WstringIterator.h"

#include "AnimationMasterBlock.h"

namespace Leviathan{ namespace GameObject{

	class SkeletonRig : public BaseObject{
	public:
		DLLEXPORT SkeletonRig::SkeletonRig();
		DLLEXPORT SkeletonRig::~SkeletonRig();
		
		DLLEXPORT void Release();
		
		DLLEXPORT bool UpdatePose(int mspassed);

		// animation related //
		DLLEXPORT bool StartPlayingAnimation(shared_ptr<AnimationMasterBlock> Animation);
		DLLEXPORT void KillAnimation();
		DLLEXPORT shared_ptr<AnimationMasterBlock> GetAnimation();
		
		DLLEXPORT bool CopyValuesToBuffer(BoneTransfromBufferWrapper* buffer);

		DLLEXPORT bool SaveOnTopOfTextBlock(ObjectFileTextBlock* block);

		DLLEXPORT int GetBoneCount();
		DLLEXPORT bool VerifyBoneGroupExist(int ID, const wstring &name);

		// loading //
		// TODO: change exporter to give axis rotations
		DLLEXPORT static SkeletonRig* LoadRigFromFileStructure(ObjectFileTextBlock* structure, bool NeedToChangeCoordinateSystem);
		DLLEXPORT static void SetSkeletonLoadingBoneToOzzJoint(SkeletonLoadingBone* bone, ozz::animation::offline::RawSkeleton::Joint* joint);

		DLLEXPORT static D3DXMATRIX CreateFromOzzFloatMatrix(ozz::math::Float4x4* matrice);


	private:
		void ReleaseBuffers();
		void ResizeMatriceCount(size_t newsize);

		// -------------------- //
		//vector<shared_ptr<D3DXMATRIX>> VerticeTranslationMatrices;
		vector<ozz::math::Float4x4*> VerticeTranslationMatrices;
		
		vector<shared_ptr<SkeletonLoadingBone>> RigsBones;
		vector<shared_ptr<IntWstring>> BoneGroups;

		ozz::animation::Skeleton* ModelSkeleton;

		// animation //
		shared_ptr<AnimationMasterBlock> PlayingAnimation;


		bool StopOnNextFrame : 1;
	};

}}
#endif