#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LOADEDANIMATION
#include "LoadedAnimation.h"
#endif
using namespace Leviathan;
using namespace ozz::animation;
using namespace ozz;
// ------------------------------------ //
DLLEXPORT Leviathan::LoadedAnimation::LoadedAnimation(){
	// private constructor used by AnimationManager //
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

DLLEXPORT void Leviathan::LoadedAnimation::SetSourceFile(const wstring &source){
	SourceFile = source;
}

DLLEXPORT void Leviathan::LoadedAnimation::SetName(const wstring &name){
	Name = name;
}

DLLEXPORT void Leviathan::LoadedAnimation::SetBaseModelName(const wstring &basemodelname){
	BaseModelName = basemodelname;
}

DLLEXPORT void Leviathan::LoadedAnimation::SetBones(const vector<shared_ptr<GameObject::SkeletonLoadingBone>> &bones){
	BaseBones = bones;
}

DLLEXPORT void Leviathan::LoadedAnimation::AddNewFrame(shared_ptr<AnimationFrameData> frame){
	Frames.push_back(frame);
}

DLLEXPORT int Leviathan::LoadedAnimation::ProcessLoadedData(){
	// process some data, that would have to be handled every time when creating new Animations from this //

	// process data into a ozz animation //
	offline::RawAnimation rawanimation;
	// set data //
	// AnimDuration is float of how many seconds the animation lasts, always //
	rawanimation.duration = AnimDuration;

	// there is 1 track per bone //
	rawanimation.tracks.resize(BaseBones.size());

	for(int i = 0; i < rawanimation.num_tracks(); i++){
		// get ref to track //
		offline::RawAnimation::JointTrack& track = rawanimation.tracks[i];

		// push frames for this bone //

		// reserve enough space to avoid pointless allocations //
		track.translations.reserve(Frames.size());
		track.rotations.reserve(Frames.size());
		track.scales.reserve(Frames.size());

		for(size_t a = 0; a < Frames.size(); a++){
			// count time on this frame //
			float elapsedfrombegin = AnimDuration/(KeyFrames-(int)(a+1));

			const offline::RawAnimation::TranslationKey tkey = {elapsedfrombegin, math::Float3(Frames[a]->FrameBones[i]->RestPosition.X,
				Frames[a]->FrameBones[i]->RestPosition.Y, Frames[a]->FrameBones[i]->RestPosition.Z)};
			track.translations.push_back(tkey);

		}
	}


	// succeeded //
	return 0;
}

// ------------------------------------ //

// ------------------------------------ //






