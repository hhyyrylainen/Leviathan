#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ANIMATIONBLOCK
#include "AnimationBlock.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "AnimationMasterBlock.h"
#include "LoadedAnimation.h"

DLLEXPORT Leviathan::AnimationBlock::AnimationBlock(){
	// get id //
	ID = IDFactory::GetID();
}

DLLEXPORT Leviathan::AnimationBlock::~AnimationBlock(){

}

// ------------------------------------ //
DLLEXPORT int& Leviathan::AnimationBlock::GetID(){
	return ID;
}

DLLEXPORT bool Leviathan::AnimationBlock::SampleToStreams(AnimationMasterBlock* block){
	// get time from block //
	int mstimepassed = block->GetBlockMSPassed();

	// get streams for bones //
	vector<shared_ptr<AnimationStream>> &UsedStreams = block->GetStoredStreamsForVertexGroupList(ID, FrameData->GetVertexGroupsControlled());

	// get/add StreamBlocks to streams //

	for(size_t i = 0; i < UsedStreams.size(); i++){

		// get block (adds new if none exist) //
		AnimationStreamBlock* blockonstream = UsedStreams[i]->GetBlockForID(ID);

		// set percentage to be correct //
		blockonstream->ControlPercentage = ControlPercentage;
		blockonstream->FullControlIfOnlyBlock = TakeControlEntirelyIfOnlyBlock;

		// update position updates //
		FrameData->SampleToStreamBlockAtTime(UsedStreams[i]->GetVertexGroup(), blockonstream, mstimepassed);
	}

	// all blocks and settings copied //
	return true;
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //
