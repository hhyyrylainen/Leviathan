#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SKELETONBONE
#include "SkeletonBone.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
DLLEXPORT Leviathan::GameObject::SkeletonBone::SkeletonBone(){
	// let default allocations be //
}

DLLEXPORT Leviathan::GameObject::SkeletonBone::SkeletonBone(const wstring &name, const Float3 &position, int group) : Name(name), RestPosition(position),
	AnimationPosition(RestPosition)	
{
	// set values + initializer list //
	BoneGroup = group;
}

DLLEXPORT Leviathan::GameObject::SkeletonBone::~SkeletonBone(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameObject::SkeletonBone::SetName(const wstring &name){
	Name = name;
}

DLLEXPORT void Leviathan::GameObject::SkeletonBone::SetRestPosition(const Float3 &val){
	RestPosition = val;
}

DLLEXPORT void Leviathan::GameObject::SkeletonBone::SetAnimationPosition(const Float3 &val){
	AnimationPosition = val;
}

DLLEXPORT void Leviathan::GameObject::SkeletonBone::SetBoneGroup(int group){
	BoneGroup = group;
}


// ------------------------------------ //
DLLEXPORT Float3& Leviathan::GameObject::SkeletonBone::GetRestPosition(){
	return RestPosition;
}

DLLEXPORT void Leviathan::GameObject::SkeletonBone::SetPosePosition(const Float3 &pos){
	AnimationPosition = pos;
}

DLLEXPORT void Leviathan::GameObject::SkeletonBone::SetParentName(const wstring &name){
	ParentName = name;
}

// ------------------------------------ //

// ------------------------------------ //

