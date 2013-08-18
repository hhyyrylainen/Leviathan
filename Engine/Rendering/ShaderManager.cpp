#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SHADERMANAGER
#include "ShaderManager.h"
#endif
using namespace Leviathan;
using namespace Rendering;
// ------------------------------------ //


DLLEXPORT Leviathan::Rendering::ShaderManager::ShaderManager() : _DirectBumpMapShader(NULL), _DirectGradientShader(NULL), _DirectLightShader(NULL),
	_DirectSkinnedShader(NULL), _StoredBumpMapShader(NULL), _StoredGradientShader(NULL), _StoredLightShader(NULL), _StoredSkinnedShader(NULL),
	_StoredTextureShader(NULL)
{

}

DLLEXPORT Leviathan::Rendering::ShaderManager::~ShaderManager(){
	// smart pointers should take care of everything //
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::ShaderManager::Init(ID3D11Device* device){
	// nothing else to initialize but various shader objects //

	// always initialize all shaders in DefaultShaders.h // 
	_DirectTextureShader = new TextureShader();
	CLASS_ALLOC_CHECK(_DirectTextureShader);

	if(!_DirectTextureShader->Init(device)){

		Logger::Get()->Error(L"ShaderManager: Init: failed to initialize TextureShader");
		return false;
	}
	_StoredTextureShader = shared_ptr<StoredShader>(new StoredShader(_DirectTextureShader->GetShaderPattern(), L"TextureShader", _DirectTextureShader));


	_DirectLightShader = new LightShader();
	CLASS_ALLOC_CHECK(_DirectLightShader);

	if(!_DirectLightShader->Init(device)){

		Logger::Get()->Error(L"ShaderManager: Init: failed to initialize LightShader");
		return false;
	}
	_StoredLightShader = shared_ptr<StoredShader>(new StoredShader(_DirectLightShader->GetShaderPattern(), L"LightShader", _DirectLightShader));


	_DirectBumpMapShader = new LightBumpShader();
	CLASS_ALLOC_CHECK(_DirectBumpMapShader);

	if(!_DirectBumpMapShader->Init(device)){

		Logger::Get()->Error(L"ShaderManager: Init: failed to initialize LightBumpShader");
		return false;
	}
	_StoredBumpMapShader = shared_ptr<StoredShader>(new StoredShader(_DirectBumpMapShader->GetShaderPattern(), L"LightBumpShader", _DirectBumpMapShader));



	_DirectGradientShader = new GradientShader();
	CLASS_ALLOC_CHECK(_DirectGradientShader);

	if(!_DirectGradientShader->Init(device)){

		Logger::Get()->Error(L"ShaderManager: Init: failed to initialize GradientShader");
		return false;
	}
	_StoredGradientShader = shared_ptr<StoredShader>(new StoredShader(_DirectGradientShader->GetShaderPattern(), L"GradientShader", _DirectGradientShader));


	_DirectSkinnedShader = new SkinnedShader();
	CLASS_ALLOC_CHECK(_DirectSkinnedShader);

	if(!_DirectSkinnedShader->Init(device)){

		Logger::Get()->Error(L"ShaderManager: Init: failed to initialize SkinnedShader");
		return false;
	}
	_StoredSkinnedShader = shared_ptr<StoredShader>(new StoredShader(_DirectSkinnedShader->GetShaderPattern(), L"SkinnedShader", _DirectSkinnedShader));


	_DirectFontShader = new FontShader();
	CLASS_ALLOC_CHECK(_DirectFontShader);

	if(!_DirectFontShader->Init(device)){

		Logger::Get()->Error(L"ShaderManager: Init: failed to initialize FontShader");
		return false;
	}
	_StoredFontShader = shared_ptr<StoredShader>(new StoredShader(_DirectFontShader->GetShaderPattern(), L"FontShader", _DirectFontShader));

	// push all to the vector //
	Shaders.reserve(5);

	Shaders.push_back(_StoredTextureShader);
	Shaders.push_back(_StoredLightShader);
	Shaders.push_back(_StoredGradientShader);
	Shaders.push_back(_StoredSkinnedShader);
	Shaders.push_back(_StoredFontShader);



	return true;
}

DLLEXPORT void Leviathan::Rendering::ShaderManager::Release(){
	// we should release all shaders //
	for(size_t i = 0; i < Shaders.size(); i++){

		Shaders[i]->ShaderPtr->Release();
	}
	Shaders.clear();


	// pointers to various parts in the vector //
	_StoredSkinnedShader.reset();
	_StoredGradientShader.reset();
	_StoredBumpMapShader.reset();
	_StoredLightShader.reset();
	_StoredTextureShader.reset();

	_DirectTextureShader = NULL;
	_DirectLightShader = NULL;
	_DirectBumpMapShader = NULL;
	_DirectGradientShader = NULL;
	_DirectSkinnedShader = NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::ShaderManager::AutoRender(ID3D11DeviceContext* devcont, const int &indexcount, ShaderRenderTask* torender, 
	const wstring &preferredname)
{
	// find right shader //
	BaseShader* tmpptr = GetShaderMatchingObject(torender, preferredname);

	// render if something found //
	return tmpptr ? tmpptr->Render(devcont, indexcount, torender): false;
}

DLLEXPORT BaseShader* Leviathan::Rendering::ShaderManager::GetShaderMatchingObject(ShaderRenderTask* matchingdata, const wstring &preferredname /*= L""*/){
	// assigning negative to unsigned creates a huge number which is hopefully above shader count //
	size_t RenderIndex = (size_t)-1;
	// if preferred name is set try to use that shader //
	if(preferredname.size() > 0){

		for(size_t i = 0; i < Shaders.size(); i++){

			if(Shaders[i]->ShaderName == preferredname){
				// check for pattern match //
				if(matchingdata->GetShaderPattern() != Shaders[i]->ShaderDefStr){

					DEBUG_BREAK;
				}
				// we can render //
				RenderIndex = i;
				break;
			}
		}
	}


	ARR_INDEX_CHECK(RenderIndex, Shaders.size()){
		// valid index //
		return Shaders[RenderIndex]->ShaderPtr.get();
	}

	// find by comparing shader patterns //
	for(size_t i = 0; i < Shaders.size(); i++){
		if(Shaders[i]->ShaderDefStr == matchingdata->GetShaderPattern()){
			// pattern match //
			RenderIndex = i;
			// if we have a name defined we might want to keep looping until exact match //
			if(preferredname.size() > 0){
				if(preferredname != Shaders[i]->ShaderName)
					continue;
			}
			break;
		}
	}

	ARR_INDEX_CHECK(RenderIndex, Shaders.size()){
		// valid index //
		return Shaders[RenderIndex]->ShaderPtr.get();
	}

	// didn't find any matching shader //
	return NULL;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::Rendering::ShaderManager::PrintShaderError(const wstring &shader, ID3D10Blob* datadump){

	// get errors from dump //
	char* Errors = (char*)(datadump->GetBufferPointer());
	unsigned long Errossize = datadump->GetBufferSize();


	// copy message to wstring //
	string tempstr(Errors, Errossize);

	Logger::Get()->Error(L"[SHADER] "+shader+L" Error:\n"+Convert::StringToWstring(tempstr)+L"\n[END]", false);
	// release error data //
	SAFE_RELEASE(datadump);
}

DLLEXPORT UINT Leviathan::Rendering::ShaderManager::GetShaderCompileFlags(){
	UINT CompileFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#ifdef SHADER_COMPILE_PREFERFLOW
	CompileFlags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
#endif // SHADER_COMPILE_PREFERFLOW
#ifdef SHADER_COMPILE_SKIP_OPTIMIZE
	CompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // SHADER_COMPILE_SKIP_OPTIMIZE
#ifdef SHADER_COMPILE_DEBUG
	CompileFlags |= D3DCOMPILE_DEBUG;
	//CompileFlags |= D3DCOMPILE_
#endif // SHADER_COMPILE_DEBUG
	return CompileFlags;
}
