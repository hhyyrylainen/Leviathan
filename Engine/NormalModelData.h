#ifndef LEVIATHAN_NORMALMODELDATA
#define LEVIATHAN_NORMALMODELDATA
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ComplainOnce.h"
#include "TimingMonitor.h"
#include "BaseModelDataObject.h"
#include "LineTokenizer.h"

#include "Rendering/ShaderDataTypes.h"
#include "SkeletonRig.h"

namespace Leviathan{ namespace GameObject{




	class NormalModelData : public BaseModelDataObject{
	public:
		DLLEXPORT NormalModelData::NormalModelData();
		DLLEXPORT NormalModelData::~NormalModelData();

		DLLEXPORT void ReleaseModel();

		DLLEXPORT bool InitBuffers(ID3D11Device* device);

		DLLEXPORT void RenderBuffers(ID3D11DeviceContext* devcont);

		DLLEXPORT virtual bool WriteToFile(const wstring& file, bool InBinary = false);
		DLLEXPORT wstring GetModelTypeName();

		DLLEXPORT bool LoadRenderModel(wstring* file);

		DLLEXPORT bool LoadFromOBJ(wstring* file);
		DLLEXPORT bool LoadFromLEVMO(wstring* file);

		DLLEXPORT SkeletonRig* GetSkeleton();
		DLLEXPORT virtual int GetAnimation(shared_ptr<AnimationMasterBlock> &ReceivedPtr);

	protected:
		// structs //
		struct LoadingFace{
		public:
			LoadingFace();
			// ------------------ //

			vector<int> VertexIDS;
			vector<Float2> UVs; 
			//vector<CombinedClass<Int1, Float2>> Data;
		};

	protected:
		// data //
		bool IsVertexGroupsUsed;
		vector<NormalModelVertexType*> pRenderModel;
		vector<Int3*> ModelFaceData;

		//vector<int> RenderModelVerticeVertexGroups;

		SkeletonRig* Skeleton;

	};

}}
#endif