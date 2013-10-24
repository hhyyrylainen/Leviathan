#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_MANAGEDTEXTURE
#include "ManagedTexture.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ManagedTexture::ManagedTexture() : Texture(NULL), FromFile(NULL){
	// set right "error" state //
	ErrorState = TEXTURE_ERROR_STATE_NON_LOADED;
	Loaded = false;

	UnusedTime = 0;
	LoadedFromMemory = false;
}

DLLEXPORT Leviathan::ManagedTexture::ManagedTexture(const wstring &file, const int &id, const TEXTURETYPE &type) : FromFile(new wstring(file)), ID(id), 
	TextureType(type), Texture(NULL)
{
	// set right "error" state //
	ErrorState = TEXTURE_ERROR_STATE_NON_LOADED;
	Loaded = false;

	UnusedTime = 0;
	LoadedFromMemory = false;
}

DLLEXPORT Leviathan::ManagedTexture::ManagedTexture(unsigned char* buffer, int bufferelements, int id, const wstring &source, ID3D11Device* dev, 
	const TEXTURETYPE &type) : ID(id), FromFile(new wstring(source)), TextureType(type)
{
	// load the texture from memory
	HRESULT hr = D3DX11CreateShaderResourceViewFromMemory(dev, buffer, sizeof(char)*bufferelements, NULL, NULL, &Texture, NULL);
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

DLLEXPORT Leviathan::ManagedTexture::ManagedTexture(const int &id, ID3D11ShaderResourceView* texture, const wstring &source, const TEXTURETYPE 
	&type) : ID(id), Texture(texture), FromFile(new wstring(source)), TextureType(type)
{
	// texture is already loaded to memory //
	Loaded = true;
	ErrorState = TEXTURE_ERROR_STATE_NONE;
	UnusedTime = 0;
	LoadedFromMemory = true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ManagedTexture::Load(ID3D11Device* dev){
	// loads the texture //
	if(Loaded){
		// already loaded //
		return true;
	}

	// load the texture from file
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(dev, (*FromFile).c_str(), NULL, NULL, &Texture, NULL);
	if(FAILED(hr)){

		Logger::Get()->Error(L"Failed to load texture from file: \""+*FromFile+L"\" loaded error texture");
		ErrorState = TEXTURE_ERROR_STATE_USE_ERROR;
		Loaded = true;
		return false;
	}

	ErrorState = TEXTURE_ERROR_STATE_NONE;
	Loaded = true;
	return true;
}

DLLEXPORT void Leviathan::ManagedTexture::UnLoad(bool force){
	// check is this still valid //
	if(Texture == NULL){
		if(!force)
			Logger::Get()->Error(L"ManagedTexture: UnLoad: already unloaded texture, ID: "+Convert::ToWstring(ID), true);
		return;
	}
	
	// implement feature to not unload generated textures //
	// TODO: implement a callback system that allows this function to call callback that tells if it can be unloaded //
	if((force) || (!LoadedFromMemory)){
		// safely release the pointer //
		SAFE_RELEASE(Texture);
	}
}
// ------------------------------------ //