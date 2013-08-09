#ifndef LEVIATHAN_SHADERRENDERTASK
#define LEVIATHAN_SHADERRENDERTASK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ManagedTexture.h"
#include "SkeletonRig.h"


namespace Leviathan{
	// forward declaration for friend //
	class RenderingShaderBase;

	struct BaseTextureHolder{
		inline BaseTextureHolder(const int &textures, const __int32 textureflags) : TextureCount(textures), TextureFlags(textureflags){

		}
		virtual ~BaseTextureHolder(){

		}

		int TextureCount;
		__int32 TextureFlags;
	};

	struct SingleTextureHolder : public BaseTextureHolder{
		inline SingleTextureHolder(shared_ptr<ManagedTexture> texture1) : BaseTextureHolder(1, texture1->GetType()), Texture1(texture1){

		}


		shared_ptr<ManagedTexture> Texture1;
	};


	struct BaseMatrixBufferData{
		BaseMatrixBufferData(const D3DXMATRIX &worldm, const D3DXMATRIX &viewm, const D3DXMATRIX &projm);


		D3DXMATRIX WorldMatrix;
		D3DXMATRIX ViewMatrix;
		D3DXMATRIX ProjectionMatrix;
	};

	struct BaseLightBufferData{
		BaseLightBufferData(const D3DXVECTOR3 & lightdir, const D3DXVECTOR3 &camerapos, const D3DXVECTOR4 &ambient, const D3DXVECTOR4 &diffusecolor,
			const D3DXVECTOR4 &specularcolor, const float &specularpower) : LightDirection(lightdir), CameraPosition(camerapos), AmbientColor(ambient),
			DiffuseColor(diffusecolor), SpecularColor(specularcolor), SpecularPower(specularpower)
		{
			
		}


		D3DXVECTOR3 LightDirection;
		D3DXVECTOR3 CameraPosition;
		D3DXVECTOR4 AmbientColor;
		D3DXVECTOR4 DiffuseColor;
		D3DXVECTOR4 SpecularColor;
		float SpecularPower;
	};

	struct BaseSkinningData{
		BaseSkinningData(GameObject::SkeletonRig* bones) : Bones(Bones){

		}

		GameObject::SkeletonRig* Bones;
	};


	// TODO: split this class into parts which can be independently added to this base class object //
	// e.g. ShaderRenderLightData
	class ShaderRenderTask : public Object{
		friend RenderingShaderBase;
	public:
		DLLEXPORT ShaderRenderTask::ShaderRenderTask();
		DLLEXPORT ShaderRenderTask::~ShaderRenderTask();

		DLLEXPORT inline ShaderRenderTask* SetBaseMatrixBuffer(BaseMatrixBufferData* newbdata){
			// release old and set new //
			SAFE_DELETE(BMatData);
			BMatData = newbdata;
			return this;
		}

		DLLEXPORT inline ShaderRenderTask* SetBaseLightBuffer(BaseLightBufferData* newbdata){
			// release old and set new //
			SAFE_DELETE(BLightData);
			BLightData = newbdata;
			return this;
		}

		DLLEXPORT inline ShaderRenderTask* SetTextures(BaseTextureHolder* newtextures){
			// release old and set new //
			SAFE_DELETE(TextureObjects);
			TextureObjects = newtextures;
			return this;
		}

		DLLEXPORT inline ShaderRenderTask* SetVertexSkinningData(BaseSkinningData* newskinningdata){
			// release old and set new //
			SAFE_DELETE(VertexSkinningData);
			VertexSkinningData = newskinningdata;
			return this;
		}

		// get functions //
		DLLEXPORT inline BaseMatrixBufferData* GetBaseMatrixBufferData(){
			return BMatData;
		}
		DLLEXPORT inline BaseTextureHolder* GetBaseTextureHolder(){
			return TextureObjects;
		}
		DLLEXPORT inline BaseLightBufferData* GetBaseLightBufferData(){
			return BLightData;
		}
		DLLEXPORT inline BaseSkinningData* GetBaseSkinningData(){
			return VertexSkinningData;
		}

	protected:

		// ------------------------------------ //
		// probably not the most efficient way but it'll do for now //
		BaseMatrixBufferData* BMatData;
		// this is a base class pointer so that we can have any number of textures //
		BaseTextureHolder* TextureObjects;
		// stores lighting data //
		BaseLightBufferData* BLightData;
		// used for vertex skinning supporting shaders //
		BaseSkinningData* VertexSkinningData;
	};

}
#endif