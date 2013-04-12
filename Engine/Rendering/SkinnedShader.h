#ifndef LEVIATHAN_SKINNEDSHADER_SMALL
#define LEVIATHAN_SKINNEDSHADER_SMALL
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <d3dx11async.h>
#include "ShaderDataTypes.h"
#include "../SkeletonRig.h"
#include "ComplainOnce.h"

namespace Leviathan{

	enum SHADER_BONECOUNT{ SHADER_BONES_TINY, SHADER_BONES_SMALL, SHADER_BONES_MEDIUM, SHADER_BONES_LARGE, SHADER_BONES_HUGE, SHADER_BONES_MAX};

	class SkinnedShader : public EngineComponent{
	public:
		DLLEXPORT SkinnedShader();
		DLLEXPORT ~SkinnedShader();

		DLLEXPORT bool Init(ID3D11Device* device);
		DLLEXPORT void Release();
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont,int indexcount, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, 
			GameObject::SkeletonRig* Bones, ID3D11ShaderResourceView* texture, Float3 lightDirection, Float4 ambientColor, Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower);

	private:
		bool Inited;
		// ------------------------------- //
		bool InitShader(ID3D11Device* dev, const wstring &shaderfile);
		void ReleaseShader();

		bool SetShaderParams(SHADER_BONECOUNT AmountBones, ID3D11DeviceContext* devcont, D3DXMATRIX worldmatrix, D3DXMATRIX viewmatrix, D3DXMATRIX projectionmatrix, ID3D11ShaderResourceView* texture,
			GameObject::SkeletonRig* Bones, Float3 lightDirection, Float4 ambientColor, Float4 diffuseColor, Float3 cameraPosition, Float4 specularColor, float specularPower);
		void ShaderRender(SHADER_BONECOUNT AmountBones, ID3D11DeviceContext* devcont, int indexcount);

		// these need to be defined for all sizes //
		ID3D11VertexShader* VertexShader_tiny;
		ID3D11Buffer* BoneMatriceBuffer_tiny;
		//ID3D11InputLayout* Layout_tiny;

		ID3D11VertexShader* VertexShader_small;
		ID3D11Buffer* BoneMatriceBuffer_small;
		//ID3D11InputLayout* Layout_small;

		ID3D11VertexShader* VertexShader_medium;
		ID3D11Buffer* BoneMatriceBuffer_medium;
		//ID3D11InputLayout* Layout_medium;

		ID3D11VertexShader* VertexShader_large;
		ID3D11Buffer* BoneMatriceBuffer_large;
		//ID3D11InputLayout* Layout_large;

		ID3D11VertexShader* VertexShader_huge;
		ID3D11Buffer* BoneMatriceBuffer_huge;
		//ID3D11InputLayout* Layout_huge;

		ID3D11VertexShader* VertexShader_max;
		ID3D11Buffer* BoneMatriceBuffer_max;
		//ID3D11InputLayout* Layout_max;

		ID3D11PixelShader* PixelShader;
		ID3D11Buffer* MatrixBuffer;
		ID3D11SamplerState* SamplerState;
		ID3D11Buffer* CameraBuffer;
		ID3D11Buffer* LightBuffer;
		ID3D11InputLayout* Layout;


		// ----------- //
		template<class PointerSize, class BufferSize>
		bool WriteMatricesToBuffer(ID3D11Buffer** buffer, ID3D11DeviceContext* devcont, GameObject::SkeletonRig* Bones){
			// map buffer for writing //
			D3D11_MAPPED_SUBRESOURCE MappedResource;

			HRESULT hr = devcont->Map(*buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
			if(FAILED(hr)){

				Logger::Get()->Error(L"SkinnedShader: SetShaderParams: buffer lock failed",hr);
				return false;
			}

			// Get a pointer to the data in the constant buffer.
			BufferSize* BoneBuffer = (BufferSize*)MappedResource.pData;
			// create wrapper //
			PointerSize PointerToBufferClass(BoneBuffer);
			BoneTransfromBufferWrapper wrap((BoneTransformsPointerBase*)&PointerToBufferClass);

			// copy the current transforms into the buffer //
			if(Bones){
				// get values from the rig //
				if(!Bones->CopyValuesToBuffer(&wrap)){
					// invalid bone stuff //
					Logger::Get()->Error(L"Invalid bones",hr);
					return false;
				}

			} else {
				// clear the buffer //
				(*BoneBuffer) = BufferSize();
			}

			// Unlock the buffer
			devcont->Unmap(*buffer, 0);

			// Set position (avoid overwriting old buffers) //
			int buffernumber = 2;

			// Now set the matrice constant buffer in the vertex shader with the updated values.
			devcont->VSSetConstantBuffers(buffernumber, 1, buffer);
			return true;
		}

	};
}
#endif