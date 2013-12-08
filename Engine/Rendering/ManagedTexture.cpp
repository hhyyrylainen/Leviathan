#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_MANAGEDTEXTURE
#include "ManagedTexture.h"
#endif
#include "Utility/ComplainOnce.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ManagedTexture::ManagedTexture() : FromFile(NULL){
	// set right "error" state //
	ErrorState = TEXTURE_ERROR_STATE_NON_LOADED;
	Loaded = false;

	UnusedTime = 0;
	LoadedFromMemory = false;
}

DLLEXPORT Leviathan::ManagedTexture::ManagedTexture(const wstring &file, const int &id, const TEXTURETYPE &type) : FromFile(new wstring(file)), ID(id), 
	TextureType(type)
{
	// set right "error" state //
	ErrorState = TEXTURE_ERROR_STATE_NON_LOADED;
	Loaded = false;

	UnusedTime = 0;
	LoadedFromMemory = false;
}

DLLEXPORT Leviathan::ManagedTexture::ManagedTexture(unsigned char* buffer, int bufferelements, int id, const wstring &source, const TEXTURETYPE &type) 
	: ID(id), FromFile(new wstring(source)), TextureType(type)
{
	// load the texture from memory
	HRESULT hr = E_FAIL;
	ComplainOnce::PrintWarningOnce(L"no textureloading", L"no textureloading in" __WFUNCTION__);
	if(FAILED(hr)){

		Logger::Get()->Error(L"ManagedTexture: CTor: Failed to load texture from memory("+Convert::ToHexadecimalWstring<void*>((void*)buffer)+L"): \""
			+*FromFile+L"\" ID "+Convert::ToWstring(id)+L", loaded error texture");
		ErrorState = TEXTURE_ERROR_STATE_USE_ERROR;
		return;
	}

	// texture is properly loaded //
	Loaded = true;
	ErrorState = TEXTURE_ERROR_STATE_NONE;
	UnusedTime = 0;
	LoadedFromMemory = true;
}

DLLEXPORT Leviathan::ManagedTexture::~ManagedTexture(){
	// just so that if there is some special functionality added it gets called //
	UnLoad(true); 
}

//DLLEXPORT Leviathan::ManagedTexture::ManagedTexture(const int &id, ID3D11ShaderResourceView* texture, const wstring &source, const TEXTURETYPE 
//	&type) : ID(id), Texture(texture), FromFile(new wstring(source)), TextureType(type)
//{
//	// texture is already loaded to memory //
//	Loaded = true;
//	ErrorState = TEXTURE_ERROR_STATE_NONE;
//	UnusedTime = 0;
//	LoadedFromMemory = true;
//}
// ------------------------------------ //
DLLEXPORT void Leviathan::ManagedTexture::UnLoad(bool force){
	// check is this still valid //
	
	// implement feature to not unload generated textures //
	// TODO: implement a callback system that allows this function to call callback that tells if it can be unloaded //
	if((force) || (!LoadedFromMemory)){
		// safely release the pointer //
		//SAFE_RELEASE(Texture);
	}
}
// ------------------------------------ //