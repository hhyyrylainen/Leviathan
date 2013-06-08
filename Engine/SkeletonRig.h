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

#include "SkeletonBone.h"
#include "WstringIterator.h"

#include "AnimationMasterBlock.h"

namespace Leviathan{ namespace GameObject{

	class SkeletonRig : public BaseObject{
	public:
		DLLEXPORT SkeletonRig::SkeletonRig();
		DLLEXPORT SkeletonRig::~SkeletonRig();
		
		DLLEXPORT void Release();
		
		DLLEXPORT void UpdatePose(int mspassed);

		// aren't actually needed //
		//DLLEXPORT bool CreateBuffersForRendering(ID3D11Device* device);
		//DLLEXPORT bool UpdateBuffersForRendering(ID3D11DeviceContext* devcont);
		//DLLEXPORT ID3D11Buffer* FetchBuffer();

		// animation related //
		DLLEXPORT bool StartPlayingAnimation(shared_ptr<AnimationMasterBlock> Animation);
		DLLEXPORT void KillAnimation();
		DLLEXPORT shared_ptr<AnimationMasterBlock> GetAnimation();
		
		DLLEXPORT bool CopyValuesToBuffer(BoneTransfromBufferWrapper* buffer);

		DLLEXPORT bool SaveOnTopOfTextBlock(ObjectFileTextBlock* block);

		DLLEXPORT int GetBoneCount();
		DLLEXPORT bool VerifyBoneGroupExist(int ID, const wstring &name);

		DLLEXPORT static SkeletonRig* LoadRigFromFileStructure(ObjectFileTextBlock* structure, bool NeedToChangeCoordinateSystem);


	private:
		void ReleaseBuffers();
		void ResizeMatriceCount(int newsize);

		// recursive bone updating //
		void UpdateBone(SkeletonBone* bone, D3DXMATRIX* parentmatrix);
		D3DXMATRIX* GetMatrixForBone(SkeletonBone* bone);

		// -------------------- //
		//SkeletalAnimationStream* Animation;
		vector<shared_ptr<D3DXMATRIX>> VerticeTranslationMatrices;
		vector<shared_ptr<SkeletonBone>> RigsBones;
		vector<shared_ptr<IntWstring>> BoneGroups;

		// animation //
		shared_ptr<AnimationMasterBlock> PlayingAnimation;
		bool StopOnNextFrame : 1;
		bool UseTranslatedPositions : 1;
	};

}}
#endif