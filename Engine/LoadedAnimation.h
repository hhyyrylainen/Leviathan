#ifndef LEVIATHAN_LOADEDANIMATION
#define LEVIATHAN_LOADEDANIMATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SkeletonLoadingBone.h"

#include "Ozz\animation\animation.h"
#include "Ozz\animation\offline\animation_builder.h"

namespace Leviathan{


	struct AnimationFrameData{
		DLLEXPORT AnimationFrameData(int framenumber) : FrameNumber(framenumber){
		};

		// bones on this frame //
		vector<GameObject::SkeletonLoadingBone*> FrameBones;
		int FrameNumber;
	};

	class LoadedAnimation : public Object{
		friend class AnimationManager;
		friend class AnimationMasterBlock;

	public:
		DLLEXPORT LoadedAnimation::~LoadedAnimation();

		DLLEXPORT wstring& GetSourceFile();
		DLLEXPORT wstring& GetName();
		DLLEXPORT wstring& GetBaseModelName();
		DLLEXPORT void SetSourceFile(const wstring &source);
		DLLEXPORT void SetName(const wstring &name);
		DLLEXPORT void SetBaseModelName(const wstring &basemodelname);
		DLLEXPORT void SetBones(const vector<shared_ptr<GameObject::SkeletonLoadingBone>> &bones);

		DLLEXPORT int ProcessLoadedData();

		DLLEXPORT void AddNewFrame(shared_ptr<AnimationFrameData> frame);

	private:
		DLLEXPORT LoadedAnimation::LoadedAnimation();

		// data //
		wstring SourceFile;
		wstring Name;
		wstring BaseModelName;

		vector<shared_ptr<GameObject::SkeletonLoadingBone>> BaseBones;

		vector<shared_ptr<AnimationFrameData>> Frames;

		shared_ptr<ozz::animation::Animation> RealAnimation;

		// some info about animation //
		float AnimDuration;
		//bool IsFPSCount: 1;

		int KeyFrames;
		int BaseBoneCount;

	};

}
#endif