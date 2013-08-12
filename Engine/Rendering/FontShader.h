#ifndef LEVIATHAN_FONTSHADER
#define LEVIATHAN_FONTSHADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseShader.h"


namespace Leviathan{ namespace Rendering{


	class FontShader : public BaseShader{
	public:
		DLLEXPORT FontShader();
		DLLEXPORT virtual ~FontShader();

		DLLEXPORT virtual bool DoesInputObjectWork(ShaderRenderTask* paramstocheck) const;

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
}}
#endif