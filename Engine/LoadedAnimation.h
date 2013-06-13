#ifndef LEVIATHAN_LOADEDANIMATION
#define LEVIATHAN_LOADEDANIMATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "AnimationBlock.h"
#include "SkeletonBone.h"
#include "AnimationBoneData.h"

namespace Leviathan{

	class AnimationManager;

	struct AnimationFrameData{
		DLLEXPORT AnimationFrameData(int framenumber) : FrameNumber(framenumber){
		};
		DLLEXPORT ~AnimationFrameData(){
			// release bone data on this frame //
			SAFE_DELETE_VECTOR(FrameBones);
		}
		// bones on this frame //
		vector<AnimationBoneData*> FrameBones;
		int FrameNumber;
	};

	class LoadedAnimation : public Object{
		friend AnimationManager;
	public:
		DLLEXPORT LoadedAnimation::~LoadedAnimation();

		DLLEXPORT shared_ptr<AnimationBlock> CreateFromThis();

		DLLEXPORT wstring& GetSourceFile();
		DLLEXPORT wstring& GetName();
		DLLEXPORT wstring& GetBaseModelName();
		DLLEXPORT void SetSourceFile(const wstring &source);
		DLLEXPORT void SetName(const wstring &name);
		DLLEXPORT void SetBaseModelName(const wstring &basemodelname);
		DLLEXPORT void SetBones(const vector<shared_ptr<GameObject::SkeletonBone>> &bones);

		DLLEXPORT int ProcessLoadedData();

		DLLEXPORT void AddNewFrame(shared_ptr<AnimationFrameData> frame);

	private:
		DLLEXPORT LoadedAnimation::LoadedAnimation();

		// data //
		wstring SourceFile;
		wstring Name;
		wstring BaseModelName;

		vector<shared_ptr<GameObject::SkeletonBone>> AnimBones;
		vector<shared_ptr<AnimationFrameData>> AnimationFrames;
	};

}
#endif