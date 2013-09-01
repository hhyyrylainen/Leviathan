#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TEXTUREHOLDER
#include "TextureArray.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
TextureArray::TextureArray(){

	textures[0] = NULL;
	textures[1] = NULL;
}
TextureArray::~TextureArray(){

}
// ------------------------------------ //
bool TextureArray::Init(ID3D11Device* dev, wstring texture1, wstring texture2){
	HRESULT hr = S_OK;

	// load the texture from file
	hr = D3DX11CreateShaderResourceViewFromFile(dev, texture1.c_str(), NULL, NULL, &textures[0], NULL);

	if(FAILED(hr))
	{
		Logger::Get()->Error(L"Failed to load texture from file: "+texture1);
		return false;
	}
	if(texture2 == L""){
		Hasmultitexture = false;
		return true;
	}
	Hasmultitexture = true;
	hr = D3DX11CreateShaderResourceViewFromFile(dev, texture2.c_str(), NULL, NULL, &textures[1], NULL);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"Failed to load texture from file: "+texture2);
		return false;
	}

	return true;
}
void TextureArray::Release(){
	SAFE_RELEASE(textures[0]);
	SAFE_RELEASE(textures[1]);
}
// ------------------------------------ //
ID3D11ShaderResourceView** TextureArray::GetTextureArray(){

	return textures;
}
ID3D11ShaderResourceView* TextureArray::GetTexture(){

	return textures[0];
}
// ------------------------------------ //

// ------------------------------------ //

