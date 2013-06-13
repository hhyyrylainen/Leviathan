#ifndef LEVIATHAN_ANIMATIONSTREAM
#define LEVIATHAN_ANIMATIONSTREAM
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "AnimationBoneData.h"

namespace Leviathan{

	struct AnimationStreamBlock{

		int AnimationID;
		bool FullControlIfOnlyBlock : 1;
		float ControlPercentage;

		unique_ptr<AnimationBoneData> CurrentBoneChanges;
	};

	class AnimationStream : public Object{
	public:
		DLLEXPORT AnimationStream::AnimationStream(int group);
		DLLEXPORT AnimationStream::~AnimationStream();

		DLLEXPORT inline int& GetVertexGroup();

		DLLEXPORT void SampleData(Float3 &receivingpos, Float3 &receivingdir);

	private:
		// unique identifier //
		int ID;
		// vertex group of the bone this channel controls //
		int VertexGroup;
		// blocks used by animations to add their changes here //
		vector<shared_ptr<AnimationStreamBlock>> Blocks;
	};

}
#endif