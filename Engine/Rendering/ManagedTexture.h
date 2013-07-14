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

	class ManagedTexture : public EngineComponent{
		friend TextureManager;
	public:

		DLLEXPORT ManagedTexture::ManagedTexture(wstring &file, int id);
		DLLEXPORT ManagedTexture::ManagedTexture(int id, ID3D11ShaderResourceView* texture, const wstring &source);
		DLLEXPORT ManagedTexture::ManagedTexture(unsigned char* buffer, int bufferelements, int id, const wstring &source, ID3D11Device* dev);
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

		// shared_ptr<ID3D11ShaderResourceView>(NULL, SafeReleaser<ID3D11ShaderResourceView>);
		// texture needs to be ->released //
		ID3D11ShaderResourceView* Texture;
		shared_ptr<wstring> FromFile;

	};

}
#endif