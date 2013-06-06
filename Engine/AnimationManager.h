#ifndef LEVIATHAN_ANIMATIONMANAGER
#define LEVIATHAN_ANIMATIONMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "LoadedAnimation.h"
#include "FileSystem.h"
#include "WstringIterator.h"
#include "ObjectFileProcessor.h"
#include "SkeletonLoadingBone.h"
#include "LineTokenizer.h"

namespace Leviathan{

	struct IndexedAnimation{
		IndexedAnimation(const wstring &name, const wstring &source) : SourceFile(source), AnimationName(name), CorrespondingAnimation(NULL){
		}
		IndexedAnimation(const wstring &source) : SourceFile(source), AnimationName(), CorrespondingAnimation(NULL){
		}

		shared_ptr<LoadedAnimation> CorrespondingAnimation;
		wstring SourceFile;
		wstring AnimationName;
	};

	class AnimationManager : public EngineComponent{
	public:
		DLLEXPORT AnimationManager::AnimationManager();
		DLLEXPORT AnimationManager::~AnimationManager();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		// getting animations/managing //
		DLLEXPORT shared_ptr<LoadedAnimation> GetAnimation(const wstring &name);
		DLLEXPORT shared_ptr<LoadedAnimation> GetAnimation(int ID);
		DLLEXPORT shared_ptr<LoadedAnimation> GetAnimationFromIndex(int ind);

		DLLEXPORT bool UnloadAnimation(const wstring &name);
		DLLEXPORT bool UnloadAnimation(int ID);
		DLLEXPORT bool UnloadAnimationFromIndex(int ind);


		DLLEXPORT bool IndexAllAnimations(bool LoadToMemory=false);

		DLLEXPORT static AnimationManager* Get();
	private:
		// private loading functions //
		int VerifyAnimLoaded(const wstring &file, bool SkipCheck = false);
		bool IsSourceFileLoaded(const wstring &sourcefile);
		wstring GetAnimationNameFromFile(const wstring &file);

		// ------------------------------------ //
		vector<shared_ptr<LoadedAnimation>> AnimationsInMemory;
		vector<unique_ptr<IndexedAnimation>> AnimationFiles;

		// static access //
		static AnimationManager* instance;
	};

}
#endif