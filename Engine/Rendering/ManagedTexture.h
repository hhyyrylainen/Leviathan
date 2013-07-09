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

		DLLEXPORT ManagedTexture::ManagedTexture(unsigned char* buffer, int bufferelements, int id, ID3D11Device* dev);
		DLLEXPORT ManagedTexture::~ManagedTexture();

		DLLEXPORT bool Load(ID3D11Device* dev);
		DLLEXPORT void UnLoad(bool force);

		DLLEXPORT inline wstring* GetSourceFile();

		DLLEXPORT inline ID3D11ShaderResourceView* GetView();
		DLLEXPORT const shared_ptr<ID3D11ShaderResourceView>& GetPermanent();

		DLLEXPORT inline int GetErrorState() const;
		DLLEXPORT inline int GetID() const;

		DLLEXPORT inline bool IsLoaded() const;

		int UnusedTime;

	private:
		DLLEXPORT ManagedTexture::ManagedTexture();
		// ------------------------------------ //

		int ID;

		int ErrorState;
		bool Loaded;

		// it must be made sure that this will be called with custom releaser SafeReleaser or/and made sure that texture is not deleted //
		shared_ptr<ID3D11ShaderResourceView> Texture;
		shared_ptr<wstring> FromFile;

	};

}
#endif