#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_MANAGEDTEXTURE
#include "ManagedTexture.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ManagedTexture::ManagedTexture() : Texture(NULL){
	ErrorState = TEXTURE_ERROR_STATE_NON_LOADED;
	UnusedTime = 0;
	Loaded = false;
}

ManagedTexture::ManagedTexture(wstring &file, int id){
	FromFile = shared_ptr<wstring>(new wstring(file));
	ID = id;

	ErrorState = TEXTURE_ERROR_STATE_NON_LOADED;

	Texture = shared_ptr<ID3D11ShaderResourceView>(NULL, SafeReleaser<ID3D11ShaderResourceView>);

	UnusedTime = 0;
	Loaded = false;
}
ManagedTexture::ManagedTexture(unsigned char* buffer, int bufferelements, int id, ID3D11Device* dev){
	ID = id;
	FromFile = shared_ptr<wstring>(new wstring(L"GRATE"));

	UnusedTime = 0;
	// load the texture from memory
	ID3D11ShaderResourceView* tempview = NULL;
	//HRESULT hr = D3DX11CreateShaderResourceViewFromFile(dev, (*FromFile).c_str(), NULL, NULL, &(tempview), NULL);
	HRESULT hr = D3DX11CreateShaderResourceViewFromMemory(dev, buffer, sizeof(unsigned char)*bufferelements, NULL,NULL,&tempview, NULL);
	if(FAILED(hr)){

		Logger::Get()->Error(L"Failed to load texture from memory!!!!: \""+*FromFile+L"\" loaded error texture");
		ErrorState = TEXTURE_ERROR_STATE_USE_ERROR;
		return;
	}
	// take new pointer //
	Texture = shared_ptr<ID3D11ShaderResourceView>(tempview, SafeReleaser<ID3D11ShaderResourceView>);

	Loaded = true;
	ErrorState = TEXTURE_ERROR_STATE_NONE;
}



ManagedTexture::~ManagedTexture(){
	UnLoad(true); // just so that if there is some special functionality added it gets called
	FromFile.reset();
}
// ------------------------------------ //
wstring* ManagedTexture::GetSourceFile(){
	return FromFile.get();
}
// ------------------------------------ //
bool ManagedTexture::Load(ID3D11Device* dev){
	// load the texture //
	if(Loaded)
		return true;
	// load the texture from file
	ID3D11ShaderResourceView* tempview = NULL;
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(dev, (*FromFile).c_str(), NULL, NULL, &(tempview), NULL);
	if(FAILED(hr)){

		Logger::Get()->Error(L"Failed to load texture from file: \""+*FromFile+L"\" loaded error texture");
		ErrorState = TEXTURE_ERROR_STATE_USE_ERROR;
		Loaded = true;
		return false;
	}
	// take new pointer //
	Texture = shared_ptr<ID3D11ShaderResourceView>(tempview, SafeReleaser<ID3D11ShaderResourceView>);


	ErrorState = TEXTURE_ERROR_STATE_NONE;
	Loaded = true;
	return true;
}
void ManagedTexture::UnLoad(bool force){
	// check is this still valid //
	try{
		if(FromFile.get() == NULL)
			return;
	}
	catch(...){
		// invalid //
#ifdef _DEBUG
		Logger::Get()->Error(L"ManagedTexture: trying to re unload already unloaded texture", ID, true);
#endif
		return;
	}

	// texture deallocator should have been set to safe releaser so that it safely releases the resource //
	// implement feature to not unload generated textures //
	if((force) | (*FromFile != L"GRATE")){
		// fetch the pointer and if it exists release it //
		if((Texture.get() != NULL) && (Loaded)){
			Texture.reset();
			Loaded = false;
		}
	}
}
// ------------------------------------ //
ID3D11ShaderResourceView* ManagedTexture::GetView(){
	return Texture.get();
}
const shared_ptr<ID3D11ShaderResourceView>& ManagedTexture::GetPermanent(){
	return Texture;
}
int ManagedTexture::GetErrorState() const{
	return ErrorState;
}
int ManagedTexture::GetID() const{
	return ID;
}
bool ManagedTexture::IsLoaded() const{
	return Loaded;
}
// ------------------------------------ //