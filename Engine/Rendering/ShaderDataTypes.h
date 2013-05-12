#ifndef LEVIATHAN_RENDERING_SHADERDATATYPES
#define LEVIATHAN_RENDERING_SHADERDATATYPES
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#define MAX_BONES_TINY	4
#define MAX_BONES_SMALL 10
#define MAX_BONES_MEDIUM 24
#define MAX_BONES_LARGE 35
#define MAX_BONES_HUGE	50
#define MAX_BONES_MAX	100

namespace Leviathan{

	struct MatrixBufferType{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct LightBufferType{
		D3DXVECTOR4 ambientColor;
		D3DXVECTOR4 diffuseColor;
		D3DXVECTOR3 lightDirection;
		float specularPower;
		D3DXVECTOR4 specularColor;
	};
	// ---- Bone matrice classes ---- //
	class BoneTransformsPointerBase : public Object{
	public:
		// used to be able to have pointers to all different types of Bone matrix buffer classes
		DLLEXPORT virtual ~BoneTransformsPointerBase();
		DLLEXPORT virtual short GetMaxBoneCount() = 0;
	};

	class BoneTransfromBufferWrapper{
	public:
		// has bone count and pointer through a base pointer to bone transform matrice pointer class //
		BoneTransfromBufferWrapper(BoneTransformsPointerBase* data);

		// these are used to be able to get proper object out of the wrapper //
		short BoneCount;
		BoneTransformsPointerBase* Data;

	};

	class BoneTransformsBufferTypeTiny /*: public BoneTransformsBufferTypeBase*/{
	public:
		// utility function //

		DLLEXPORT short GetMaxBoneCount();
		// the bone count is different from all others //
		D3DXMATRIX BoneMatrices[MAX_BONES_TINY];
	};
	class BoneTransformsBufferTypeSmall /*: public BoneTransformsBufferTypeBase*/{
	public:
		// utility function //

		DLLEXPORT short GetMaxBoneCount();
		// the bone count is different from all others //
		D3DXMATRIX BoneMatrices[MAX_BONES_SMALL];
	};
	class BoneTransformsBufferTypeMedium /*: public BoneTransformsBufferTypeBase*/{
	public:
		// utility function //

		DLLEXPORT short GetMaxBoneCount();
		// the bone count is different from all others //
		D3DXMATRIX BoneMatrices[MAX_BONES_MEDIUM];
	};
	class BoneTransformsBufferTypeLarge /*: public BoneTransformsBufferTypeBase*/{
	public:
		// utility function //

		DLLEXPORT short GetMaxBoneCount();
		// the bone count is different from all others //
		D3DXMATRIX BoneMatrices[MAX_BONES_LARGE];
	};
	class BoneTransformsBufferTypeHuge /*: public BoneTransformsBufferTypeBase*/{
	public:
		// utility function //

		DLLEXPORT short GetMaxBoneCount();
		// the bone count is different from all others //
		D3DXMATRIX BoneMatrices[MAX_BONES_HUGE];
	};
	class BoneTransformsBufferTypeMax /*: public BoneTransformsBufferTypeBase*/{
	public:
		// utility function //

		DLLEXPORT short GetMaxBoneCount();
		// the bone count is different from all others //
		D3DXMATRIX BoneMatrices[MAX_BONES_MAX];
	};
	// ----- pointers to classes ----- //
	class BoneTransformsPointerTiny : public BoneTransformsPointerBase{
	public:
		BoneTransformsPointerTiny(BoneTransformsBufferTypeTiny* data);

		// storing the real class under this pointer //
		BoneTransformsBufferTypeTiny* Data;
		// virtual function for bone count //
		DLLEXPORT virtual short GetMaxBoneCount();
	};
	class BoneTransformsPointerSmall : public BoneTransformsPointerBase{
	public:
		BoneTransformsPointerSmall(BoneTransformsBufferTypeSmall* data);

		// storing the real class under this pointer //
		BoneTransformsBufferTypeSmall* Data;
		// virtual function for bone count //
		DLLEXPORT virtual short GetMaxBoneCount();
	};
	class BoneTransformsPointerMedium : public BoneTransformsPointerBase{
	public:
		BoneTransformsPointerMedium(BoneTransformsBufferTypeMedium* data);

		// storing the real class under this pointer //
		BoneTransformsBufferTypeMedium* Data;
		// virtual function for bone count //
		DLLEXPORT virtual short GetMaxBoneCount();
	};
	class BoneTransformsPointerLarge : public BoneTransformsPointerBase{
	public:
		BoneTransformsPointerLarge(BoneTransformsBufferTypeLarge* data);

		// storing the real class under this pointer //
		BoneTransformsBufferTypeLarge* Data;
		// virtual function for bone count //
		DLLEXPORT virtual short GetMaxBoneCount();
	};
	class BoneTransformsPointerHuge : public BoneTransformsPointerBase{
	public:
		BoneTransformsPointerHuge(BoneTransformsBufferTypeHuge* data);

		// storing the real class under this pointer //
		BoneTransformsBufferTypeHuge* Data;
		// virtual function for bone count //
		DLLEXPORT virtual short GetMaxBoneCount();
	};
	class BoneTransformsPointerMax : public BoneTransformsPointerBase{
	public:
		BoneTransformsPointerMax(BoneTransformsBufferTypeMax* data);

		// storing the real class under this pointer //
		BoneTransformsBufferTypeMax* Data;
		// virtual function for bone count //
		DLLEXPORT virtual short GetMaxBoneCount();
	};
	// old version //
	//class BoneTransfromBufferWrapper{
	//public:
	//	// has bone count and pointer through a base pointer to bone transform matrice class //
	//	//BoneTransfromBufferWrapper(short maxbones, BoneTransformsBufferTypeBase* object);
	//	BoneTransfromBufferWrapper(BoneTransformsBufferTypeTiny* data);
	//	BoneTransfromBufferWrapper(BoneTransformsBufferTypeSmall* data);
	//	BoneTransfromBufferWrapper(BoneTransformsBufferTypeMedium* data);
	//	BoneTransfromBufferWrapper(BoneTransformsBufferTypeLarge* data);
	//	BoneTransfromBufferWrapper(BoneTransformsBufferTypeHuge* data);
	//	BoneTransfromBufferWrapper(BoneTransformsBufferTypeMax* data);

	//	// this one automatically gets the maxbones //
	//	//BoneTransfromBufferWrapper(BoneTransformsBufferTypeBase* object);
	//	// these are used to be able to get proper object out of the wrapper //
	//	short BoneCount;
	//	//BoneTransformsBufferTypeBase* Data;
	//	BoneTransformsBufferTypeTiny* TinyData;
	//	BoneTransformsBufferTypeSmall* SmallData;
	//	BoneTransformsBufferTypeMedium* MediumData;
	//	BoneTransformsBufferTypeLarge* LargeData;
	//	BoneTransformsBufferTypeHuge* HugeData;
	//	BoneTransformsBufferTypeMax* MaxData;
	//};

	// ----     ----- //
	// must match the one in font renderer
	
	struct VertexType{

		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};

	struct ModelVertexType{

		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
	};
	struct ModelGroupedVertex{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;

		//int VertexGroupID1, VertexGroupID2, VertexGroupID3, VertexGroupID4;
		UINT4 VertexGroups;
		//float VertexGroupWeight1, VertexGroupWeight2, VertexGroupWeight3, VertexGroupWeight4;
		Float4 VertexGroupWeights;
		//D3DXVECTOR3 VertexGroupWeights;
		//D3DXVECTOR4 VertexGroupWeights;
	};
	struct ModelVertexFormat{
		float x, y, z;
	};
	struct BumpVertexType{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 tangent;
		D3DXVECTOR3 binormal;
	};

	struct BumpModelVertexType{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
	};
	class NormalModelVertexType{
	public:
		DLLEXPORT virtual ~NormalModelVertexType(){};

		float x, y, z;
		float tu, tv;
		float nx, ny, nz;

		//virtual void DummyFunction(){};
	};
	class NormalModelVertexTypeVertexGroups : public NormalModelVertexType{
	public:
		//int VertexGroupID1, VertexGroupID2, VertexGroupID3, VertexGroupID4;
		UINT4 VertexGroups;
		//float VertexGroupWeight1, VertexGroupWeight2, VertexGroupWeight3, VertexGroupWeight4;
		Float4 VertexGroupWeights;
	};
	struct ColorBuffer
	{
		D3DXVECTOR4 ColorStart;
		D3DXVECTOR4 ColorEnd;
	};
	struct CameraBufferType
	{
		D3DXVECTOR3 cameraPosition;
		float padding;
	};
	

}
#endif