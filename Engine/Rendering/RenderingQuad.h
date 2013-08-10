#ifndef LEVIATHAN_RENDERING_QUAD
#define LEVIATHAN_RENDERING_QUAD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseRenderableBufferContainer.h"


#define COLORQUAD_VERTEXCOUNT						6

namespace Leviathan{ namespace Rendering{


	class RenderingQuad : public BaseRenderableBufferContainer{
	public:
		DLLEXPORT RenderingQuad();
		DLLEXPORT ~RenderingQuad();


		DLLEXPORT bool Update(ID3D11DeviceContext* devcont, const Float2 &pos, const Float2 &sizes, int screenwidth, int screenheight, int coordtype, 
			int flowstyle = 1);

		//DLLEXPORT void Resize(int screenwidth, int screenheight, int quadwidth, int quadheight);
		DLLEXPORT bool Render(ID3D11DeviceContext* devcont, );

		DLLEXPORT inline int GetIndexCount(){
			return COLORQUAD_VERTEXCOUNT;
		}


	private:
		virtual bool CreateBuffers(ID3D11Device* device);
		
		// ---------------------- //
		int UVFlowStyle;
		int CoordType;
		int ScreenWidth, ScreenHeight;
		Float2 QuadSize;
		Float2 Position;

		static const string InputDefinitionType;
	};
}}
#endif