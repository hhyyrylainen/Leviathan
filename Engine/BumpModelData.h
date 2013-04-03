#ifndef LEVIATHAN_BUMPMODELDATA
#define LEVIATHAN_BUMPMODELDATA
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseModelDataObject.h"

namespace Leviathan{ namespace GameObject{

	class BumpModelData : public BaseModelDataObject {
	protected:
		//struct BumpVertexType
		//{
		//	D3DXVECTOR3 position;
		//	D3DXVECTOR2 texture;
		//	D3DXVECTOR3 normal;
		//	D3DXVECTOR3 tangent;
		//	D3DXVECTOR3 binormal;
		//};

		//struct BumpModelVertexType
		//{
		//	float x, y, z;
		//	float tu, tv;
		//	float nx, ny, nz;
		//	float tx, ty, tz;
		//	float bx, by, bz;
		//};

		//struct BumpTempVertexType
		//{
		//	float x, y, z;
		//	float tu, tv;
		//	float nx, ny, nz;
		//};

		void BumpCalculateTangentBinormal(BumpModelVertexType vertex1, BumpModelVertexType vertex2, BumpModelVertexType vertex3, ModelVertexFormat& tangent, ModelVertexFormat& binormal);
		void BumpCalculateModelVectors();
	public:
		DLLEXPORT BumpModelData::BumpModelData();
		DLLEXPORT BumpModelData::~BumpModelData();

		DLLEXPORT virtual void ReleaseModel();

		DLLEXPORT virtual bool InitBuffers(ID3D11Device* device);

		DLLEXPORT virtual void RenderBuffers(ID3D11DeviceContext* devcont);

		DLLEXPORT virtual bool LoadRenderModel(wstring* file);

		DLLEXPORT virtual bool WriteToFile(const wstring& file, bool InBinary = false);

		DLLEXPORT virtual wstring GetModelTypeName();



	protected:
		BumpModelVertexType* BumbModel;
	};

}}
#endif