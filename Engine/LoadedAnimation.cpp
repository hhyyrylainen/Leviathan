#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LOADEDANIMATION
#include "LoadedAnimation.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "ExceptionInvalidArguement.h"
#include "ComplainOnce.h"

DLLEXPORT Leviathan::LoadedAnimation::LoadedAnimation(){
	// private constructor used by AnimationManager //
	IsFPSDefined = true;
	AnimationFPS = 30;
	AnimationFrameLength = -1;
}
DLLEXPORT Leviathan::LoadedAnimation::~LoadedAnimation(){

}
// ------------------------------------ //
DLLEXPORT wstring& Leviathan::LoadedAnimation::GetSourceFile(){
	return SourceFile;
}

DLLEXPORT wstring& Leviathan::LoadedAnimation::GetName(){
	return Name;
}

DLLEXPORT wstring& Leviathan::LoadedAnimation::GetBaseModelName(){
	return BaseModelName;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LoadedAnimation::SetSourceFile(const wstring &source){
	SourceFile = source;
}

DLLEXPORT void Leviathan::LoadedAnimation::SetName(const wstring &name){
	Name = name;
}

DLLEXPORT void Leviathan::LoadedAnimation::SetBaseModelName(const wstring &basemodelname){
	BaseModelName = basemodelname;
}

DLLEXPORT void Leviathan::LoadedAnimation::SetBones(const vector<shared_ptr<GameObject::SkeletonBone>> &bones){
	AnimBones = bones;
}

DLLEXPORT void Leviathan::LoadedAnimation::SetAnimationFPS(const int &val){
	IsFPSDefined = true;
	AnimationFPS = val;
	AnimationFrameLength = -1;
}

DLLEXPORT void Leviathan::LoadedAnimation::SetFrameMSLength(const int &val){
	IsFPSDefined = false;
	AnimationFPS = -1;
	AnimationFrameLength = val;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::LoadedAnimation::AddNewFrame(shared_ptr<AnimationFrameData> frame){
	AnimationFrames.push_back(frame);
}

DLLEXPORT int Leviathan::LoadedAnimation::ProcessLoadedData(){
	// process some data, that would have to be handled every time when creating new Animations from this //


	// succeeded //
	return 0;
}
// ------------------------------------ //
DLLEXPORT vector<int>& Leviathan::LoadedAnimation::GetVertexGroupsControlled(){
	if(VerticesControlled.size() == 0){
		// can't be right, update //
		VerticesControlled.reserve(AnimBones.size());

		for(size_t i = 0; i < AnimBones.size(); i++){

			VerticesControlled.push_back(AnimBones[i]->GetBoneGroup());
		}
	}

	return VerticesControlled;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::LoadedAnimation::SampleToStreamBlockAtTime(const int &bonegroup, AnimationStreamBlock* block, int mstimepassedfromstart){
	// get frame before (or at current time) and frame after from time passed //
	int FrameBegin = -1;
	int StartFrameStart = -1;
	int FrameEnd = -1;
	int EndFrameStart = -1;

	int tmpframestart = 0;

	// go through frames and stop when frame start time is over or equal to current time //
	for(size_t i = 0; i < AnimationFrames.size(); i++){
		// get start time //
		CalculateFrameStartTime(tmpframestart, AnimationFrames[i].get(), i);

		if(tmpframestart > mstimepassedfromstart){
			// not yet reached this frame //
			if(i == 0){
				// first frame, which shouldn't be here! //
				if(mstimepassedfromstart < 0){
					// found the problem //
					throw new ExceptionInvalidArguement(L"negative passed time", mstimepassedfromstart, __WFUNCSIG__, L"mstimepassedfromstart");
				}
				// might be totally fine //
				ComplainOnce::PrintWarningOnce(L"LoadedAnim_Frames_INVALID_First", L"First frame of animation's start time is not 0, check file");
				// will be able to continue just fine //
				FrameBegin = FrameEnd = AnimationFrames[i]->FrameNumber;
				break;
			}
			// starts on previous and ends on this one //
			FrameBegin = AnimationFrames[i-1]->FrameNumber;
			FrameEnd = AnimationFrames[i]->FrameNumber;
			EndFrameStart = tmpframestart;
			break;

		} else if (tmpframestart == mstimepassedfromstart){
			// exactly this frame //
			FrameBegin = FrameEnd = AnimationFrames[i]->FrameNumber;
			break;
		}
		// not yet found frame end being higher than this current time //
		// set frame begin so that if next frame is the ending frame we don't need to re calculate it //
		StartFrameStart = tmpframestart;
	}
	// check various cases //
	if(FrameBegin == -1 && FrameEnd == -1){
		// animation is past end time //
		return _SampleFramesToBlockWithBlend(0.f, AnimationFrames.back().get(), NULL, block, bonegroup);
		// TODO: send message someplace indicating that animation stream has ended //
	}
	if(FrameBegin == FrameEnd){
		// just one frame //
		return _SampleFramesToBlockWithBlend(0.f, GetFrameFromFrameNumber(FrameBegin).get(), NULL, block, bonegroup);
	}

	// calculate percentage of last frame //
	float percentageofend = ((float)(mstimepassedfromstart-StartFrameStart))/(EndFrameStart-StartFrameStart);

	// get changes on both frames and mix them together with the percentage //
	return _SampleFramesToBlockWithBlend(percentageofend, GetFrameFromFrameNumber(FrameBegin).get(), GetFrameFromFrameNumber(FrameEnd).get(), block, bonegroup);
}

DLLEXPORT void inline Leviathan::LoadedAnimation::CalculateFrameStartTime(int &startms, AnimationFrameData* frame, size_t index /*= SIZE_T_MAX*/){
	// check is it already calculated //
	if(frame->MSStartTime > -1){

		// it is, set it and return //
		startms = frame->MSStartTime;
		return;
	}

	// get frame index (find out what number this frame is) //
	if(index == SIZE_T_MAX){
		// find index by comparing pointers //
		for(size_t i = 0; i < AnimationFrames.size(); i++){
			if(AnimationFrames[i].get() == frame){
				// pointers match //
				index = i;
				break;
			}
		}
	}

	// "switch" on type of time define d//
	if(IsFPSDefined){
		// divide frame number (0 index to have first frame at 0 time) by FPS //
		// multiply by thousand to get milliseconds //
		startms = (int)(((float)index/AnimationFPS)*1000.f);


	} else {

		// frame length in ms is defined //
		startms = index*AnimationFrameLength;
	}
	// store result for later usage //
	frame->MSStartTime = startms;
}

bool Leviathan::LoadedAnimation::_SampleFramesToBlockWithBlend(float blendfactor, AnimationFrameData* frame1, AnimationFrameData* frame2, 
	AnimationStreamBlock* receiver, const int &bonegroup)
{
	// create new data holder if it doesn't exist //
	if(receiver->CurrentBoneChanges.get() == NULL){

		receiver->CurrentBoneChanges = unique_ptr<AnimationBoneData>(new AnimationBoneData((Float3)0, (Float4)0, bonegroup));
	}

	if(blendfactor == 0 || frame2 == NULL){
		// just first frame //

		CopyChangedAmountsToResultsFromFrame(frame1, bonegroup, receiver->CurrentBoneChanges->Position, receiver->CurrentBoneChanges->Direction);

		return true;
	} else if (blendfactor == 1 || frame1 == NULL){
		// just the second frame //

		CopyChangedAmountsToResultsFromFrame(frame2, bonegroup, receiver->CurrentBoneChanges->Position, receiver->CurrentBoneChanges->Direction);

		return true;
	}

	Float3 Frame1Pos, Frame2Pos;
	Float4 Frame1Rot, Frame2Rot;
	// get data //
	CopyChangedAmountsToResultsFromFrame(frame1, bonegroup, Frame1Pos, Frame1Rot);
	CopyChangedAmountsToResultsFromFrame(frame2, bonegroup, Frame2Pos, Frame2Rot);

	// multiply by modifier and set data //
	receiver->CurrentBoneChanges->Position = Frame1Pos*(1.f-blendfactor)+Frame2Pos*blendfactor;
	//receiver->CurrentBoneChanges->Direction = Frame1Rot*(1.f-blendfactor)+Frame2Rot*blendfactor;
	receiver->CurrentBoneChanges->Direction = Frame1Rot.Slerp(Frame2Rot, blendfactor);

	// succeeded //
	return true;
}

DLLEXPORT shared_ptr<AnimationFrameData> Leviathan::LoadedAnimation::GetFrameFromFrameNumber(const int &number){
	for(size_t i = 0; i < AnimationFrames.size(); i++){
		if(AnimationFrames[i]->FrameNumber == number){
			// number matches //
			return AnimationFrames[i];
		}
	}

	// nothing //
	return NULL;
}

DLLEXPORT void Leviathan::LoadedAnimation::CopyChangedAmountsToResultsFromFrame(AnimationFrameData* frame, const int &bonegroup, 
	Float3 &poschangereceiver, Float4 &dirchangereceiver)
{

	GameObject::SkeletonBone* BindPoseBone = NULL;
	AnimationBoneData* AnimatedBone = NULL;

	for(size_t i = 0; i < frame->FrameBones.size(); i++){

		if(frame->FrameBones[i]->BoneGroup == bonegroup){

			AnimatedBone = frame->FrameBones[i];
			break;
		}
	}
	for(size_t i = 0; i < AnimBones.size(); i++){

		if(AnimBones[i]->GetBoneGroup() == bonegroup){

			BindPoseBone = AnimBones[i].get();
			break;
		}
	}

	if(AnimatedBone == NULL || BindPoseBone == NULL){

		DEBUG_BREAK;
		return;
	}

	poschangereceiver = AnimatedBone->Position-BindPoseBone->GetRestPosition();
	dirchangereceiver = AnimatedBone->Direction-BindPoseBone->GetRestDirection();
}

// ------------------------------------ //

// ------------------------------------ //






