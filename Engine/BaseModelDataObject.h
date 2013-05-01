#ifndef LEVIATHAN_BASEMODELDATAOBJECT
#define LEVIATHAN_BASEMODELDATAOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseObject.h"
#include "MultiFlag.h"
#include "Rendering/Graphics.h"
// data types //
#include "Rendering/ShaderDataTypes.h"
#include "SkeletonRig.h"
#include "AnimationMasterBlock.h"

namespace Leviathan{ namespace GameObject{

	enum MODELOBJECT_MODEL_TYPE{MODELOBJECT_MODEL_TYPE_ERROR, MODELOBJECT_MODEL_TYPE_NORMAL, MODELOBJECT_MODEL_TYPE_BUMP, MODELOBJECT_MODEL_TYPE_NONLOADED};

	class BaseModelDataObject : public BaseObject{
	protected:
		//struct ModelVertexType{

		//	D3DXVECTOR3 position;
		//	D3DXVECTOR2 texture;
		//	D3DXVECTOR3 normal;
		//};
		//struct ModelVertexFormat{
		//	float x, y, z;
		//};
	public:
		DLLEXPORT BaseModelDataObject::BaseModelDataObject();
		DLLEXPORT virtual BaseModelDataObject::~BaseModelDataObject();

		DLLEXPORT virtual void ReleaseModel() = 0;

		DLLEXPORT virtual bool InitBuffers(ID3D11Device* device) = 0;
		DLLEXPORT virtual void RenderBuffers(ID3D11DeviceContext* devcont) = 0;

		DLLEXPORT virtual bool LoadRenderModel(wstring* file) = 0;

		DLLEXPORT virtual bool WriteToFile(const wstring& file, bool InBinary = false) = 0;

		DLLEXPORT virtual wstring GetModelTypeName() = 0;


		DLLEXPORT static BaseModelDataObject* CreateRequiredModelObject(MultiFlag* modelfileflags);
		
		DLLEXPORT MODELOBJECT_MODEL_TYPE GetType();

		DLLEXPORT int GetIndexCount() const;

		DLLEXPORT virtual SkeletonRig* GetSkeleton();
		DLLEXPORT virtual int GetAnimation(shared_ptr<AnimationMasterBlock> &ReceivedPtr);


		// vars //
		bool IsLoaded;
		bool IsBufferDone;

		wstring ThisName;
	protected:
		MODELOBJECT_MODEL_TYPE Type;

		int VertexCount, IndexCount;
		
		ID3D11Buffer* VertexBuffer;
		ID3D11Buffer* IndexBuffer;
	};

}}
#endif