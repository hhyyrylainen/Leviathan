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
#include "AnimationStream.h"

namespace Leviathan{

	class AnimationManager;

	struct AnimationFrameData{
		DLLEXPORT AnimationFrameData(int framenumber) : FrameNumber(framenumber){
			MSStartTime = -1;
		};
		DLLEXPORT ~AnimationFrameData(){
			// release bone data on this frame //
			SAFE_DELETE_VECTOR(FrameBones);
		}
		// bones on this frame //
		vector<AnimationBoneData*> FrameBones;
		int FrameNumber;
		int MSStartTime;
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
		DLLEXPORT void SetAnimationFPS(const int &val);
		DLLEXPORT void SetFrameMSLength(const int &val);
		// TODO: Better caching of found results to improve performance //
		DLLEXPORT bool SampleToStreamBlockAtTime(const int &bonegroup, AnimationStreamBlock* block, int mstimepassedfromstart);

		DLLEXPORT void inline CalculateFrameStartTime(int &startms, AnimationFrameData* frame, size_t index = SIZE_T_MAX);

		DLLEXPORT int ProcessLoadedData();

		DLLEXPORT vector<int>& GetVertexGroupsControlled();

		DLLEXPORT void AddNewFrame(shared_ptr<AnimationFrameData> frame);
		DLLEXPORT shared_ptr<AnimationFrameData> GetFrameNumber(const int &number);
		DLLEXPORT void CopyChangedAmountsToResultsFromFrame(AnimationFrameData* frame, const int &bonegroup, Float3 &poschangereceiver, Float3 &dirchangereceiver);

	private:
		DLLEXPORT LoadedAnimation::LoadedAnimation();

		bool _SampleFramesToBlockWithBlend(float blendfactor, AnimationFrameData* frame1, AnimationFrameData* frame2, AnimationStreamBlock* receiver, const int &bonegroup);

		// data //
		wstring SourceFile;
		wstring Name;
		wstring BaseModelName;

		bool IsFPSDefined : 1;
		int AnimationFPS;
		int AnimationFrameLength;

		// list of vertex groups that this animation's bones control //
		vector<int> VerticesControlled;



		vector<shared_ptr<GameObject::SkeletonBone>> AnimBones;
		vector<shared_ptr<AnimationFrameData>> AnimationFrames;
	};

}
#endif