#ifndef LEVIATHAN_TEXTUREHOLDER
#define LEVIATHAN_TEXTUREHOLDER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Logger.h"
#include <d3dx11tex.h>

namespace Leviathan{

	class TextureArray : public EngineComponent{
	public:
		DLLEXPORT TextureArray::TextureArray();
		DLLEXPORT TextureArray::~TextureArray();

		DLLEXPORT bool Init(ID3D11Device* dev, wstring texture1, wstring texture2);
		DLLEXPORT void Release();

		DLLEXPORT ID3D11ShaderResourceView** GetTextureArray();
		DLLEXPORT ID3D11ShaderResourceView* GetTexture();

		DLLEXPORT bool IsMultiTexture(){ return Hasmultitexture; };

	private:
		bool Hasmultitexture;

		ID3D11ShaderResourceView* textures[2];
	};

}
#endif