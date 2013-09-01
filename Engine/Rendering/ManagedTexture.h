#ifndef LEVIATHAN_RENDERING_MANAGEDTEXTURE
#define LEVIATHAN_RENDERING_MANAGEDTEXTURE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

#define TEXTURE_ERROR_STATE_NONE		2500
#define TEXTURE_ERROR_STATE_NON_LOADED	2501
#define TEXTURE_ERROR_STATE_UN_LOADED	2501
#define TEXTURE_ERROR_STATE_USE_ERROR	2502

namespace Leviathan{
	// forward declaration for friend //
	class TextureManager;


	// texture types //
	enum TEXTURETYPE {
		TEXTURETYPE_NORMAL = 0x1, 
		TEXTURETYPE_BUMPMAP = 0x2, 
		TEXTURETYPE_BLENDMAP = 0x4, 
		TEXTURETYPE_LIGHTMAP = 0x8,
		TEXTURETYPE_TEXT = 0x10, 
		//TEXTURETYPE_BASEGRAPHICAL = 0x20
		//0x40
		//0x80 // first byte full
		//0x100
		//0x200
		//0x400
		//0x800
		//0x1000
		//0x2000
		//0x4000
		//0x8000 // second byte full (int range might stop here(
		//0x10000
		//0x20000
		//0x40000
		//0x80000
		//0x100000
		//0x200000
		//0x400000
		//0x800000 // third byte full
		//0x1000000
		//0x2000000
		//0x4000000
		//0x8000000
		//0x10000000
		//0x20000000
		//0x40000000
		//0x80000000 // fourth byte full (will need QWORD here)
		//0x100000000
	};



	class ManagedTexture : public EngineComponent{
		friend TextureManager;
	public:

		DLLEXPORT ManagedTexture::ManagedTexture(const wstring &file, const int &id, const TEXTURETYPE &type);
		DLLEXPORT ManagedTexture::ManagedTexture(unsigned char* buffer, int bufferelements, int id, const wstring &source, const TEXTURETYPE &type);
		DLLEXPORT ManagedTexture::~ManagedTexture();

		DLLEXPORT bool Load(ID3D11Device* dev);
		DLLEXPORT void UnLoad(bool force);

		// quick access inline functions //
		DLLEXPORT inline shared_ptr<wstring> GetSourceFile(){
			return FromFile;
		}

		DLLEXPORT inline ID3D11ShaderResourceView* GetView(){
			return Texture;
		}
		DLLEXPORT inline int GetErrorState() const{
			return ErrorState;
		}
		DLLEXPORT inline TEXTURETYPE GetType() const{
			return TextureType;
		}
		DLLEXPORT inline int GetID() const{
			return ID;
		}
		DLLEXPORT inline bool IsLoaded() const{
			return Loaded;
		}

		int UnusedTime;

	private:
		DLLEXPORT ManagedTexture::ManagedTexture();
		// ------------------------------------ //

		int ID;

		int ErrorState;
		bool Loaded;
		bool LoadedFromMemory;
		TEXTURETYPE TextureType;

		// shared_ptr<ID3D11ShaderResourceView>(NULL, SafeReleaser<ID3D11ShaderResourceView>);
		// texture needs to be ->released //
		ID3D11ShaderResourceView* Texture;
		shared_ptr<wstring> FromFile;

	};

}
#endif