#ifndef LEVIATHAN_DEFAULTSHADERSFILE
#define LEVIATHAN_DEFAULTSHADERSFILE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseShader.h"

namespace Leviathan{ namespace Rendering{


	class LightShader : BaseShader{
	public:
		DLLEXPORT LightShader();
		DLLEXPORT virtual ~LightShader();

		DLLEXPORT virtual bool DoesInputObjectWork(ShaderRenderTask* paramstocheck);

	private:

		virtual bool SetupShaderDataBuffers(ID3D11Device* dev);
		virtual bool SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer);
		virtual void ReleaseShaderDataBuffers();
		virtual bool SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters);
		virtual bool SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters);
		// ------------------------------------ //

		// buffers for passing shader data //
		ID3D11Buffer* MatrixBuffer;
		ID3D11Buffer* CameraBuffer;
		ID3D11Buffer* LightBuffer;
	};

	class GradientShader : BaseShader{
	public:
		DLLEXPORT GradientShader();
		DLLEXPORT virtual ~GradientShader();

		DLLEXPORT virtual bool DoesInputObjectWork(ShaderRenderTask* paramstocheck);

	private:

		virtual bool SetupShaderDataBuffers(ID3D11Device* dev);
		virtual bool SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer);
		virtual void ReleaseShaderDataBuffers();
		virtual bool SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters);
		virtual bool SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters);
		// ------------------------------------ //

		// buffers for passing shader data //
		ID3D11Buffer* MatrixBuffer;
		ID3D11Buffer* ColorsBuffer;
	};

	class LightBumpShader{
	public:
		DLLEXPORT LightBumpShader();
		DLLEXPORT virtual ~LightBumpShader();

		DLLEXPORT virtual bool DoesInputObjectWork(ShaderRenderTask* paramstocheck);

	private:

		virtual bool SetupShaderDataBuffers(ID3D11Device* dev);
		virtual bool SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer);
		virtual void ReleaseShaderDataBuffers();
		virtual bool SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters);
		virtual bool SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters);
		// ------------------------------------ //

		// buffers for passing shader data //
		ID3D11Buffer* MatrixBuffer;
		ID3D11Buffer* CameraBuffer;
		ID3D11Buffer* LightBuffer;
	};

	// ------------------ SkinnedShader ------------------ //
	enum SHADER_BONECOUNT{ SHADER_BONES_TINY, SHADER_BONES_SMALL, SHADER_BONES_MEDIUM, SHADER_BONES_LARGE, SHADER_BONES_HUGE, SHADER_BONES_MAX};

	class SkinnedShader : public BaseShader{
	public:
		DLLEXPORT SkinnedShader();
		DLLEXPORT virtual ~SkinnedShader();

		DLLEXPORT virtual bool DoesInputObjectWork(ShaderRenderTask* paramstocheck);
		DLLEXPORT virtual bool Render(ID3D11DeviceContext* devcont,int indexcount, ShaderRenderTask* Parameters);

	private:

		virtual bool SetupShaderDataBuffers(ID3D11Device* dev);
		virtual bool SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer);
		virtual void ReleaseShaderDataBuffers();
		virtual bool SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters);
		virtual bool SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters);
		// overload some base class functions //
		virtual bool LoadShaderFromDisk(ID3D11Device* dev);
		void virtual ShaderRender(ID3D11DeviceContext* devcont, ID3D11VertexShader* vertexshader, ID3D11PixelShader* pixelshader, const int &indexcount);
		// ------------------------------------ //

		// buffers for passing shader data //
		ID3D11Buffer* MatrixBuffer;
		ID3D11Buffer* CameraBuffer;
		ID3D11Buffer* LightBuffer;

		// these need to be defined for all sizes //
		// tiny shader is the vertex shader with no size in it's name //

		

		ID3D11VertexShader* VertexShader_small;
		ID3D11VertexShader* VertexShader_medium;
		ID3D11VertexShader* VertexShader_large;
		ID3D11VertexShader* VertexShader_huge;
		ID3D11VertexShader* VertexShader_max;

		ID3D11Buffer* BoneMatriceBuffer_tiny;
		ID3D11Buffer* BoneMatriceBuffer_small;
		ID3D11Buffer* BoneMatriceBuffer_medium;
		ID3D11Buffer* BoneMatriceBuffer_large;
		ID3D11Buffer* BoneMatriceBuffer_huge;
		ID3D11Buffer* BoneMatriceBuffer_max;

	public:
		template<class PointerSize, class BufferSize>
		bool WriteMatricesToBuffer(ID3D11Buffer** buffer, ID3D11DeviceContext* devcont, GameObject::SkeletonRig* Bones){
			// this scope that the buffer is closed when setting it //
			{
				auto AutoUnlocker = Rendering::ResourceCreator::MapConstantBufferForWriting<BufferSize>(devcont, *buffer);
				if(AutoUnlocker == NULL){
					// lock failed //
					return false;
				}
				// create wrapper //
				PointerSize PointerToBufferClass(AutoUnlocker->LockedResourcePtr);
				BoneTransfromBufferWrapper wrap((BoneTransformsPointerBase*)&PointerToBufferClass);

				// copy the current transforms into the buffer //
				if(Bones){
					// get values from the rig //
					if(!Bones->CopyValuesToBuffer(&wrap)){
						// invalid bone stuff //
						Logger::Get()->Error(L"Invalid bones",);
						return false;
					}

				} else {
					// clear the buffer //
					(*BoneBuffer) = BufferSize();
				}
			}
			// buffer is auto unlocked here //

			// Now set the matrix constant buffer in the vertex shader with the updated values.
			devcont->VSSetConstantBuffers(3, 1, buffer);
			return true;
		}
	}



}}
#endif