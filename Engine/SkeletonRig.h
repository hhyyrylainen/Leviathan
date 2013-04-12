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

namespace Leviathan{ namespace GameObject{

	class SkeletonRig : public BaseObject{
	public:
		DLLEXPORT SkeletonRig::SkeletonRig();
		DLLEXPORT SkeletonRig::~SkeletonRig();
		
		DLLEXPORT void Release();
		
		DLLEXPORT void UpdatePose(int mspassed, D3DXMATRIX* WorldMatrix);
		
		// aren't actually needed //
		//DLLEXPORT bool CreateBuffersForRendering(ID3D11Device* device);
		//DLLEXPORT bool UpdateBuffersForRendering(ID3D11DeviceContext* devcont);
		//DLLEXPORT ID3D11Buffer* FetchBuffer();
		
		DLLEXPORT bool CopyValuesToBuffer(BoneTransfromBufferWrapper* buffer);

		DLLEXPORT bool SaveOnTopOfTextBlock(ObjectFileTextBlock* block);

		DLLEXPORT int GetBoneCount();

		DLLEXPORT static SkeletonRig* LoadRigFromFileStructure(ObjectFileTextBlock* structure, bool NeedToChangeCoordinateSystem);

	private:
		void ReleaseBuffers();
		void ResizeMatriceCount(int newsize);

		// -------------------- //
		//SkeletalAnimationStream* Animation;
		vector<shared_ptr<D3DXMATRIX>> VerticeTranslationMatrices;
		
		//float Translated;
	};

}}
#endif