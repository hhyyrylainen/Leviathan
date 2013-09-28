#ifndef LEVIATHAN_RENDERING_TEXTUREMANAGER
#define LEVIATHAN_RENDERING_TEXTUREMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ManagedTexture.h"

#include "TextureGenerator.h"

#define TEXTUREMANAGER_LATEST_SEARCH_SIZE	10

#define TEXTUREMANAGER_SEARCH_LATEST				1
#define TEXTUREMANAGER_SEARCH_OLDER					2
#define TEXTUREMANAGER_SEARCH_UNLOADED				3
#define TEXTUREMANAGER_SEARCH_UTILITY				4
#define TEXTUREMANAGER_SEARCH_VOLATILEGENERATED		5

#define TEXTURE_INACTIVETIME		30
#define TEXTURE_UNLOADTIME			30000


namespace Leviathan{
	// small forward declaration //
	class Graphics;

	class TextureManager : public EngineComponent{
	public:
		DLLEXPORT TextureManager::TextureManager(bool Main, Graphics* graph);
		DLLEXPORT TextureManager::~TextureManager();

		DLLEXPORT bool Init(const wstring &basedir);
		DLLEXPORT void Release();

		DLLEXPORT void TimePass(int mspassed);

		DLLEXPORT void UnloadVolatile(const int &ID);
		DLLEXPORT bool AddVolatileGenerated(const int &ID, const wstring &source, ID3D11ShaderResourceView* texture, const TEXTURETYPE &type);


		DLLEXPORT int LoadTexture(wstring& path, const TEXTURETYPE &type, bool loadnow = false);

		DLLEXPORT shared_ptr<ManagedTexture> GetTexture(int id, int whichfirst, bool nooldsearch = false);
		// compares supplied pointer with the error texture's pointer //
		DLLEXPORT inline bool IsTextureError(const ManagedTexture* possibleerrorptr){
			// compare to error pointer //
			return ErrorTexture.get() == possibleerrorptr;
		}

	private:
		Graphics* GraphInter;
		wstring BaseDir;

		// storage arrays //
		vector<shared_ptr<ManagedTexture>> LastUsed;
		vector<shared_ptr<ManagedTexture>> Expiring; // textures that will soon be unloaded //
		vector<shared_ptr<ManagedTexture>> Unloaded; // textures that haven't been used for a while and must be reloaded to memory //

		vector<shared_ptr<ManagedTexture>> NonInitialized; // textures that are waiting to be loaded to memory //

		vector<shared_ptr<ManagedTexture>> LatestFound; // last TEXTUREMANAGER_LATEST_SEARCH_SIZE amount of used textures //

		vector<shared_ptr<ManagedTexture>> Utility;
		vector<shared_ptr<ManagedTexture>> VolatileGenerated;
		shared_ptr<ManagedTexture> ErrorTexture;

		// texture finding functions //
		shared_ptr<ManagedTexture> SearchLastUsed(int id);
		shared_ptr<ManagedTexture> SearchExpiring(int id);
		shared_ptr<ManagedTexture> SearchUnloaded(int id);
		shared_ptr<ManagedTexture> SearchUtility(int id);
		shared_ptr<ManagedTexture> SearchVolatile(int id);
		void RemoveFromUninitialized(int id);

		void AddToLatest(const shared_ptr<ManagedTexture>& toadd);
	};

}
#endif