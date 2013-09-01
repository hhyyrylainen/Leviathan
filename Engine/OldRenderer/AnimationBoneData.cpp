#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ANIMATIONBONEDATA
#include "AnimationBoneData.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::AnimationBoneData::AnimationBoneData() : Position(0.f), Direction(0.f){
	BoneGroup = -1;
}

DLLEXPORT Leviathan::AnimationBoneData::AnimationBoneData(const Float3 &pos, const Float4 &dir, const int group) : Position(pos), Direction(dir){
	BoneGroup = group;
}

DLLEXPORT Leviathan::AnimationBoneData::~AnimationBoneData(){

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


