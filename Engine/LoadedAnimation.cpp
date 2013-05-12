#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LOADEDANIMATION
#include "LoadedAnimation.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::LoadedAnimation::LoadedAnimation(){
	// private constructor used by AnimationManager //
}
DLLEXPORT Leviathan::LoadedAnimation::~LoadedAnimation(){

}
// ------------------------------------ //
DLLEXPORT shared_ptr<AnimationBlock> Leviathan::LoadedAnimation::CreateFromThis(){
	shared_ptr<AnimationBlock> createdblock(new AnimationBlock());


	return createdblock;
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

DLLEXPORT void Leviathan::LoadedAnimation::SetBones(const vector<shared_ptr<GameObject::SkeletonBone>> &bones){
	AnimBones = bones;
}

DLLEXPORT void Leviathan::LoadedAnimation::AddNewFrame(shared_ptr<AnimationFrameData> frame){
	AnimationFrames.push_back(frame);
}

DLLEXPORT int Leviathan::LoadedAnimation::ProcessLoadedData(){
	// process some data, that would have to be handled everytime when creating new Animations from this //


	// succeeded //
	return 0;
}

// ------------------------------------ //

// ------------------------------------ //






