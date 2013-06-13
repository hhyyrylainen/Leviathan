#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ANIMATIONSTREAM
#include "AnimationStream.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::AnimationStream::AnimationStream(int group){
	ID = IDFactory::GetID();
	VertexGroup = group;
}

DLLEXPORT Leviathan::AnimationStream::~AnimationStream(){

}

DLLEXPORT int& Leviathan::AnimationStream::GetVertexGroup(){
	return VertexGroup;
}

DLLEXPORT void Leviathan::AnimationStream::SampleData(Float3 &receivingpos, Float3 &receivingdir){
	// mash together all blocks taking into account the percentages //

	receivingpos = (Float3)0;
	receivingdir = (Float3)0;

	if(Blocks.size() < 1){
		// no blocks //
		DEBUG_BREAK;
		return;
	}

	if(Blocks.size() == 1){
		// just one block //
		const AnimationStreamBlock* blocky = Blocks[0].get();

		if(blocky->FullControlIfOnlyBlock){

			receivingpos = blocky->CurrentBoneChanges->Position;
			receivingdir = blocky->CurrentBoneChanges->Direction;

		} else {
			// needs to still apply percentage multiplier //
			receivingpos = blocky->CurrentBoneChanges->Position.operator*(blocky->ControlPercentage);
			receivingdir = blocky->CurrentBoneChanges->Direction.operator*(blocky->ControlPercentage);
		}
		// all blocks (1 of them) handled //
		return;
	}

	// iterate over blocks and apply them //

	for(size_t i = 0; i < Blocks.size(); i++){
		// get ptr //
		const AnimationStreamBlock* blocky = Blocks[i].get();

		receivingpos = blocky->CurrentBoneChanges->Position.operator*(blocky->ControlPercentage);
		receivingdir = blocky->CurrentBoneChanges->Direction.operator*(blocky->ControlPercentage);
	}

}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


