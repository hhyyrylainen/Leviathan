#ifndef LEVIATHAN_ANIMATIONBLOCK
#define LEVIATHAN_ANIMATIONBLOCK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "AnimationStream.h"

namespace Leviathan{

	class AnimationManager;
	class LoadedAnimation;
	class AnimationMasterBlock;

	class AnimationBlock : public Object{
	friend AnimationManager;
	public:
		// public destructor so that this can be deleted anywhere //
		DLLEXPORT AnimationBlock::~AnimationBlock();

		//************************************
		// Method:    SampleToStreams
		// FullName:  Leviathan::AnimationBlock::SampleToStreams
		// Access:    public 
		// Returns:   bool
		// Qualifier:
		// Parameter: AnimationMasterBlock * block
		// Usage: Gets time on the block and writes current bone offsets to streams copying over affect percentages
		//************************************
		DLLEXPORT bool SampleToStreams(AnimationMasterBlock* block);

		// creating a block //
		DLLEXPORT static shared_ptr<AnimationBlock> CreateBlockFromLoadedAnimation(shared_ptr<LoadedAnimation> parentanimation);

		DLLEXPORT inline int& GetID();

	private:
		// Loaded Animation creates instances of objects //
		DLLEXPORT AnimationBlock::AnimationBlock(shared_ptr<LoadedAnimation> parentanim);

		// data //
		// unique identifier for this object //
		int ID;
		float ControlPercentage;

		bool TakeControlEntirelyIfOnlyBlock : 1;


		shared_ptr<LoadedAnimation> FrameData;
	};

}
#endif