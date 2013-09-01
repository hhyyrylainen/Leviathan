#ifndef LEVIATHAN_ANIMATIONMANAGER
#define LEVIATHAN_ANIMATIONMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "FileSystem.h"
#include "Utility\Iterators\WstringIterator.h"
#include "ObjectFiles\ObjectFileProcessor.h"
#include "ObjectFiles\LineTokenizer.h"

namespace Leviathan{

	struct IndexedAnimation{
		IndexedAnimation(const wstring &name, const wstring &source) : SourceFile(source), AnimationName(name), CorrespondingAnimation(NULL){
		}
		IndexedAnimation(const wstring &source) : SourceFile(source), AnimationName(), CorrespondingAnimation(NULL){
		}

		shared_ptr<int> CorrespondingAnimation;
		wstring SourceFile;
		wstring AnimationName;
		bool Loaded;
	};

	class AnimationManager : public EngineComponent{
	public:
		DLLEXPORT AnimationManager::AnimationManager();
		DLLEXPORT AnimationManager::~AnimationManager();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		// getting animations/managing //
		DLLEXPORT shared_ptr<IndexedAnimation> GetAnimation(const wstring &name);

		DLLEXPORT bool UnloadAnimation(const wstring &name);
		DLLEXPORT bool UnloadAnimation(int ID);
		DLLEXPORT bool UnloadAnimationFromIndex(int ind);


		DLLEXPORT bool IndexAllAnimations(bool LoadToMemory=false);

		DLLEXPORT static AnimationManager* Get();
	private:

		int VerifyAnimLoaded(const wstring &file);
		bool IsSourceFileLoaded(const wstring &sourcefile);

		wstring GetAnimationNameFromFile(const wstring &file);


		vector<unique_ptr<IndexedAnimation>> AnimationFiles;
		vector<shared_ptr<IndexedAnimation>> AnimationsInMemory;

		// static access //
		static AnimationManager* instance;
	};

}
#endif