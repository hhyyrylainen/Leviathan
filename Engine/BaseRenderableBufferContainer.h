#ifndef LEVIATHAN_BASERENDERABLEBUFFERCONTAINER
#define LEVIATHAN_BASERENDERABLEBUFFERCONTAINER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{ namespace Rendering{


	class BaseRenderableBufferContainer{
	public:

		DLLEXPORT inline BaseRenderableBufferContainer(const string &format, const size_t &bufferobjectsize) : InputFormatStr(format), 
			InputObjectSize(bufferobjectsize), VertexBuffer(NULL), IndexBuffer(NULL)
		{

		}
		DLLEXPORT virtual ~BaseRenderableBufferContainer(){

		}

		DLLEXPORT virtual bool Init(ID3D11Device* device);
		DLLEXPORT virtual void Release();

		DLLEXPORT virtual bool SetBuffersForRendering(ID3D11DeviceContext* devcont, int &indexbuffersize);
		DLLEXPORT virtual inline int GetIndexCount() const = 0;

	protected:

		virtual bool RenderBuffers(ID3D11DeviceContext* devcont);

		virtual bool CreateBuffers(ID3D11Device* device) = 0;
		virtual void ReleaseBuffers();
		virtual bool Create1To1IndexBuffer(ID3D11Device* device, const int &icount);

		// ------------------------------------ //
		bool Inited;

		// rendering buffers //
		ID3D11Buffer* VertexBuffer;
		ID3D11Buffer* IndexBuffer;


		string InputFormatStr;
		size_t InputObjectSize;
	};

}}
#endif