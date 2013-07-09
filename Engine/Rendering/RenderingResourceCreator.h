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

	class ResourceCreator{
	public:
		static void StoreGraphicsInstance(Graphics* instance);

		static ID3D11Buffer* GenerateDefaultIndexBuffer(const int &indexcount);
		static ID3D11Buffer* GenerateDefaultDynamicDefaultTypeVertexBuffer(const int &count);
		static ID3D11Texture2D* GenerateDefaultTexture(const int &width, const int &height, const DXGI_FORMAT &format, const D3D11_BIND_FLAG &bflags); 


		static bool Generate2DCoordinatesFromLocationAndSize(const Float2 &location, const Float2 &size, int coordtype, Float4 &receiver);

		static void Generate2DQuadCoordinatesWithStyle(D3DXVECTOR2 &leftop, D3DXVECTOR2 &leftbottom, D3DXVECTOR2 &righttop, D3DXVECTOR2 &rigthbottom,
			int texturecoordinatestyle);
		static VertexType* GenerateQuadIntoVertexBuffer(const Float2 &location, const Float2 &size, const int &numvertices, const int &Coordtype, 
			const int &style = QUAD_FILLSTYLE_UPPERLEFT_0_BOTTOMRIGHT_1);

		static D3D11_BUFFER_DESC CreateBufferDefinition(const D3D11_BIND_FLAG &bindflags, const UINT bytewidth, bool allowcpu = false); 


	private:
		// private constructors for singleton //
		DLLEXPORT ResourceCreator();
		DLLEXPORT ~ResourceCreator();

		// access to graphics //
		static Graphics* GraphicsAccess;

	};

}}
#endif