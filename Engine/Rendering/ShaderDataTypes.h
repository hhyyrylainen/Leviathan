#ifndef LEVIATHAN_RENDERING_SHADERDATATYPES
#define LEVIATHAN_RENDERING_SHADERDATATYPES
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#define MAX_TRANSFORMS 10

namespace Leviathan{

	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	//struct LightBufferType
	//{
	//	D3DXVECTOR4 diffuseColor;
	//	D3DXVECTOR3 lightDirection;
	//	float padding;
	//};
	struct LightBufferType
	{
		D3DXVECTOR4 ambientColor;
		D3DXVECTOR4 diffuseColor;
		D3DXVECTOR3 lightDirection;
		float specularPower;
		D3DXVECTOR4 specularColor;
	};
	struct BoneTransformsBufferType
	{
		D3DXMATRIX BoneMatrices[MAX_TRANSFORMS];
		//D3DXMATRIX SkinNormalMatrices[MAX_TRANSFORMS];
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
	struct BumpVertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 tangent;
		D3DXVECTOR3 binormal;
	};

	struct BumpModelVertexType
	{
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