#ifndef LEVIATHAN_ANIMATIONBONEDATA
#define LEVIATHAN_ANIMATIONBONEDATA
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class AnimationBoneData : public Object{
	public:
		DLLEXPORT AnimationBoneData::AnimationBoneData();
		DLLEXPORT AnimationBoneData::AnimationBoneData(const Float3 &pos, const Float3 &dir, const int group);
		DLLEXPORT AnimationBoneData::~AnimationBoneData();

		// current location/rotation could be anything stored here //
		Float3 Direction;
		Float3 Position;

		// bone group //
		int BoneGroup;
	};

}
#endif