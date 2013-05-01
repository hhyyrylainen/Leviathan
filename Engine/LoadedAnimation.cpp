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
	return shared_ptr<AnimationBlock>(new AnimationBlock());
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

// ------------------------------------ //

// ------------------------------------ //






