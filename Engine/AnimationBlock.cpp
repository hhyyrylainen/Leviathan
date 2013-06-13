#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ANIMATIONBLOCK
#include "AnimationBlock.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
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
	int mstimepassed = block->AnimationMSPassed;

	// get streams for bones //


	// get/add StreamBlocks to streams //



	// update all blocks //


	return true;
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //
