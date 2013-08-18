#ifndef LEVIATHAN_SHADERMANAGER
#define LEVIATHAN_SHADERMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "DefaultShaders.h"
#include "FontShader.h"

namespace Leviathan{ namespace Rendering{

	struct StoredShader{
		StoredShader(const string &shaderdefinition, const wstring &name, BaseShader* baseptr) : ShaderDefStr(shaderdefinition), ShaderName(name),
			ShaderPtr(baseptr)
		{

		}
		~StoredShader(){
			// safely release the shader ptr //
			ShaderPtr->Release();
			ShaderPtr.reset();
		}

		shared_ptr<BaseShader> ShaderPtr;
		string ShaderDefStr;
		wstring  ShaderName;
	};


	class ShaderManager : public EngineComponent{
	public:
		DLLEXPORT ShaderManager::ShaderManager();
		DLLEXPORT ShaderManager::~ShaderManager();
		DLLEXPORT bool Init(ID3D11Device* device);
		DLLEXPORT void Release();

		// preferredname can be used to use a specific shader if you know the right name (e.g. font renderer forces FontShader usage with this) //
		DLLEXPORT bool AutoRender(ID3D11DeviceContext* devcont, const int &indexcount, ShaderRenderTask* torender, const wstring &preferredname);

		DLLEXPORT BaseShader* GetShaderMatchingObject(ShaderRenderTask* matchingdata, const wstring &preferredname = L"");


		DLLEXPORT static void PrintShaderError(const wstring &shader, ID3D10Blob* datadump);
		DLLEXPORT static UINT GetShaderCompileFlags();

	private:

		// vector that holds all loaded shaders //
		vector<shared_ptr<StoredShader>> Shaders;

		// pointers to various parts in the vector //
		shared_ptr<StoredShader> _StoredTextureShader;
		TextureShader* _DirectTextureShader;
		shared_ptr<StoredShader> _StoredLightShader;
		LightShader* _DirectLightShader;
		shared_ptr<StoredShader> _StoredBumpMapShader;
		LightBumpShader* _DirectBumpMapShader;
		shared_ptr<StoredShader> _StoredGradientShader;
		GradientShader* _DirectGradientShader;
		shared_ptr<StoredShader> _StoredSkinnedShader;
		SkinnedShader* _DirectSkinnedShader;
		shared_ptr<StoredShader> _StoredFontShader;
		FontShader* _DirectFontShader;
	};
}}
#endif

	

