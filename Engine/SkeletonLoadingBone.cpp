#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SKELETONBONE
#include "SkeletonLoadingBone.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
DLLEXPORT Leviathan::GameObject::SkeletonLoadingBone::SkeletonLoadingBone(){
	// let default allocations be //
}

DLLEXPORT Leviathan::GameObject::SkeletonLoadingBone::SkeletonLoadingBone(const wstring &name, const Float3 &position, const Float3 &dir, int group) :
	Name(name), RestPosition(position), RestDirection(dir)	
{
	// set values + initializer list //
	BoneGroup = group;
}

DLLEXPORT Leviathan::GameObject::SkeletonLoadingBone::~SkeletonLoadingBone(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameObject::SkeletonLoadingBone::SetName(const wstring &name){
	Name = name;
}

DLLEXPORT void Leviathan::GameObject::SkeletonLoadingBone::SetRestPosition(const Float3 &val){
	RestPosition = val;
}

DLLEXPORT void Leviathan::GameObject::SkeletonLoadingBone::SetRestDirection(const Float3 &val){
	RestDirection = val;
}

//DLLEXPORT void Leviathan::GameObject::SkeletonLoadingBone::SetAnimationPosition(const Float3 &val){
//	AnimationPosition = val;
//}

DLLEXPORT void Leviathan::GameObject::SkeletonLoadingBone::SetBoneGroup(int group){
	BoneGroup = group;
}


// ------------------------------------ //
DLLEXPORT Float3& Leviathan::GameObject::SkeletonLoadingBone::GetRestPosition(){
	return RestPosition;
}

//DLLEXPORT void Leviathan::GameObject::SkeletonLoadingBone::SetPosePosition(const Float3 &pos){
//	AnimationPosition = pos;
//}

DLLEXPORT void Leviathan::GameObject::SkeletonLoadingBone::SetParentName(const wstring &name){
	ParentName = name;
}

DLLEXPORT void Leviathan::GameObject::SkeletonLoadingBone::SetParent(shared_ptr<SkeletonLoadingBone> parent, shared_ptr<SkeletonLoadingBone> thisptr){
	// set parent //
	this->Parent = parent;
	// set this as parent's child //
	parent->Children.push_back(thisptr);
}

// ------------------------------------ //

// ------------------------------------ //

