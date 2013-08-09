#ifndef LEVIATHAN_RENDERINGRESOURCECREATOR
#define LEVIATHAN_RENDERINGRESOURCECREATOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ShaderDataTypes.h"

#define QUAD_FILLSTYLE_UPPERLEFT_0_BOTTOMRIGHT_1		1
#define QUAD_FILLSTYLE_UPPERLEFT_1_BOTTOMRIGHT_0		2

namespace Leviathan{ 
	// forward declaration //
	class Graphics;

namespace Rendering{

	// automatic unlock of buffers when going out of scope //
	template<class PType>
	struct BufferLockResult{
		BufferLockResult() : LockedResourcePtr(NULL), DevContForUnlock(NULL), LockedBuffer(NULL){

		}
		BufferLockResult(PType* ptr, ID3D11DeviceContext* usedcontext, ID3D11Buffer* tounlock) : LockedResourcePtr(ptr), 
			DevContForUnlock(usedcontext), LockedBuffer(tounlock)
		{
		}

		~BufferLockResult(){
			if(!LockedResourcePtr){
				// already unlocked //
				return;
			}
			// release locked //
			DevContForUnlock->Unmap(LockedBuffer, 0);
			LockedResourcePtr = NULL;
		}

		PType* LockedResourcePtr;
		ID3D11DeviceContext* DevContForUnlock;
		ID3D11Buffer* LockedBuffer;
	};


	class ResourceCreator{
	public:
		static void StoreGraphicsInstance(Graphics* instance);

		static ID3D11Buffer* GenerateDefaultIndexBuffer(const int &indexcount);
		static ID3D11Buffer* GenerateDefaultDynamicDefaultTypeVertexBuffer(const int &count);

		static bool CreateDynamicConstantBufferForVSShader(ID3D11Buffer** receiver, const UINT &bufferbytewidth);

		static ID3D11Texture2D* GenerateDefaultTexture(const int &width, const int &height, const DXGI_FORMAT &format, const D3D11_BIND_FLAG &bflags); 


		static bool Generate2DCoordinatesFromLocationAndSize(const Float2 &location, const Float2 &size, int coordtype, Float4 &receiver);

		static void Generate2DQuadCoordinatesWithStyle(D3DXVECTOR2 &leftop, D3DXVECTOR2 &leftbottom, D3DXVECTOR2 &righttop, D3DXVECTOR2 &rigthbottom,
			int texturecoordinatestyle);
		static VertexType* GenerateQuadIntoVertexBuffer(const Float2 &location, const Float2 &size, const int &numvertices, const int &Coordtype, 
			const int &style = QUAD_FILLSTYLE_UPPERLEFT_0_BOTTOMRIGHT_1);

		static D3D11_BUFFER_DESC CreateBufferDefinition(const D3D11_BIND_FLAG &bindflags, const UINT bytewidth, bool allowcpu = false); 

		template<class PType>
		static unique_ptr<BufferLockResult<PType>> MapConstantBufferForWriting(ID3D11DeviceContext* devcont, ID3D11Buffer* BufferToLock){
			// lock the buffer for writing //
			D3D11_MAPPED_SUBRESOURCE MappedResource;
			// call the actual lock method //
			HRESULT hr = devcont->Map(BufferToLock, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
			if(FAILED(hr)){
				Logger::Get()->Error(L"MapConstantBufferForWriting: buffer lock failed", hr);
				return NULL;
			}
			// lock succeeded, cast pointer to data to right type //
			PType* dataptr = (PType*)MappedResource.pData;

			return unique_ptr<BufferLockResult<PType>>(new BufferLockResult<PType>(dataptr, devcont, BufferToLock));
		}

	private:
		// private constructors for singleton //
		DLLEXPORT ResourceCreator();
		DLLEXPORT ~ResourceCreator();

		// access to graphics //
		static Graphics* GraphicsAccess;

	};

}}
#endif