#ifndef LEVIATHAN_BASESHADER
#define LEVIATHAN_BASESHADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ShaderRenderTask.h"
#include "RenderingResourceCreator.h"
#include "ShaderDataTypes.h"


namespace Leviathan{

	// NOTE: On creating shader definition strings (those "BUF:BMAT:TEX:NORMAL")
	// The strings are directly compared thus they always need to have the object in certain order:
	// first there are buffers "BUF:" following these buffers in this order if they are needed
	// BMAT, BMAT, CAMB, BLIGHT, COL[1-8] (e.g. COL2)
	// next follows which texture types "TEX:" are contained also in this strict order:
	// NORMAL, BUMP
	// Finally the format of input "INPUT:" vertices if not default (Float3 pos, Float2 texturecoordinate, Float3 normal)
	// C0, T0, N0, TANG0, BINOR0


	// class that all shaders must inherit //
	class BaseShader{
	public:
		DLLEXPORT BaseShader(const wstring &shaderfile, const string &vsentry, const string &psentry, const string &inputpattern);
		DLLEXPORT virtual ~BaseShader();

		DLLEXPORT inline bool DoesMatchPattern(const string &checktomatch) const{
			return InputPatternForShader == checktomatch;
		}
		DLLEXPORT inline const string& GetShaderPattern() const{
			return InputPatternForShader;
		}

		DLLEXPORT virtual bool Init(ID3D11Device* device);
		DLLEXPORT virtual void Release();

		DLLEXPORT virtual bool DoesInputObjectWork(ShaderRenderTask* paramstocheck) const = 0;

		DLLEXPORT virtual bool Render(ID3D11DeviceContext* devcont,int indexcount, ShaderRenderTask* Parameters);

	protected:
		// main load and unload functions //
		virtual bool CreateShader(ID3D11Device* dev);

		virtual void ReleaseShader();

		// function parts //
		virtual bool LoadShaderFromDisk(ID3D11Device* dev);
		// ResourceCreator::CreateDynamicConstantBufferForVSShader(&MatrixBuffer, sizeof(MatrixBufferType)) should be used //
		virtual bool SetupShaderDataBuffers(ID3D11Device* dev) = 0;
		virtual bool SetupShaderInputLayouts(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer) = 0;
		virtual bool CreateDefaultSamplerState(ID3D11Device* dev);
		virtual bool CreateInputLayout(ID3D11Device* dev, ID3D10Blob* VertexShaderBuffer, D3D11_INPUT_ELEMENT_DESC* layout, const UINT &elementcount);

		virtual void UnloadShader();
		virtual void UnloadSamplerAndLayout();
		virtual void ReleaseShaderDataBuffers() = 0;


		virtual bool SetShaderParams(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters)= 0;
		virtual bool SetNewDataToShaderBuffers(ID3D11DeviceContext* devcont, ShaderRenderTask* parameters)= 0;
		void virtual ShaderRender(ID3D11DeviceContext* devcont, ID3D11VertexShader* vertexshader, ID3D11PixelShader* pixelshader, const int &indexcount);
		// ------------------------------- //
		bool Inited;

		// shaders //
		ID3D11VertexShader* VertexShader;
		ID3D11PixelShader* PixelShader;


		// buffers for passing shader data //


		ID3D11SamplerState* SamplerState;
		// shader data format //
		ID3D11InputLayout* Layout;

		// shader definition strings //
		wstring ShaderFileName;
		string VSShaderEntryPoint, PSShaderEntryPoint, ShaderInputPattern;
		// holds the pattern of input that this shader needs //
		string InputPatternForShader;
	};

	// this shader is included here because it is so basic that it makes a good example for other shaders how to properly do a shader class //
	class TextureShader : BaseShader{
	public:
		DLLEXPORT TextureShader();
		DLLEXPORT virtual ~TextureShader();

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
	};


}
#endif