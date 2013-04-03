#ifndef LEVIATHAN_RENDERING_TEXTUREMANAGER
#define LEVIATHAN_RENDERING_TEXTUREMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ManagedTexture.h"
#include "TexturePointer.h"

#include <TextureGenerator.h>

#define TEXTUREMANAGER_LATEST_SEARCH_SIZE	10

#define TEXTUREMANAGER_SEARCH_LATEST	1
#define TEXTUREMANAGER_SEARCH_OLDER		2
#define TEXTUREMANAGER_SEARCH_UNLOADED	3
#define TEXTUREMANAGER_SEARCH_UTILITY	4


namespace Leviathan{
	// small forward declaration //
	class Graphics;

	class TextureManager : public EngineComponent{
	public:
		DLLEXPORT TextureManager::TextureManager(bool Main, Graphics* graph);
		DLLEXPORT TextureManager::~TextureManager();

		DLLEXPORT bool Init(const wstring &basedir, int texttimeout, int textunload);
		DLLEXPORT void Release();

		DLLEXPORT void TimePass(int mspassed);


		DLLEXPORT int LoadTexture(wstring& path, bool loadnow = false);

		DLLEXPORT ID3D11ShaderResourceView* GetTextureView(int id, int whichfirst, bool nooldsearch = false);
		DLLEXPORT ManagedTexture* GetTexture(int id, int whichfirst, bool nooldsearch = false);

	private:
		Graphics* GraphInter;

		int InActiveTime;
		int UnLoadTime;
		wstring BaseDir;

		// storage arrays //
		vector<shared_ptr<ManagedTexture>> LastUsed;
		vector<shared_ptr<ManagedTexture>> Expiring; // textures that will soon be unloaded //
		vector<shared_ptr<ManagedTexture>> Unloaded; // textures that haven't been used for a while and must be reloaded to memory //

		vector<shared_ptr<ManagedTexture>> NonInitialized; // textures that are waiting to be loaded to memory //

		vector<shared_ptr<ManagedTexture>> LatestFound; // last TEXTUREMANAGER_LATEST_SEARCH_SIZE amount of used textures //

		vector<shared_ptr<ManagedTexture>> Utility;
		shared_ptr<ManagedTexture> ErrorTexture;

		// texture finding functions //
		shared_ptr<ManagedTexture> SearchLastUsed(int id);
		shared_ptr<ManagedTexture> SearchExpiring(int id);
		shared_ptr<ManagedTexture> SearchUnloaded(int id);
		shared_ptr<ManagedTexture> SearchUtility(int id);
		void RemoveFromUninitialized(int id);

		void AddToLatest(const shared_ptr<ManagedTexture>& toadd);

		//void AddToLatestFounds(const shared_ptr<ManagedTexture>& toadd);

	};

}
#endif