#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_ANIMATEABLE
#include "AnimateableGui.h"
#endif
#include "GuiManager.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
GuiAnimateable::GuiAnimateable() : AnimationQueue(){
}
GuiAnimateable::~GuiAnimateable(){

}
// ------------------------------------ //
int GuiAnimateable::AnimationTime(int mspassed){
	return -1;
}

void GuiAnimateable::AnimationFinish(){

}
// ------------------------------------ //
void GuiAnimateable::QueueAction(AnimationAction* act){
	//// increment references and add //
	//act->AddRef();
	AnimationQueue.push_back(act);
}

DLLEXPORT void Leviathan::Gui::GuiAnimateable::RemoveActionFromQueue(AnimationAction* actionptr){
	// find //
	for(size_t i = 0; i < AnimationQueue.size(); i++){
		if(AnimationQueue[i] == actionptr){
			// remove //
			return RemoveActionFromQueue(i);
		}
	}
}

DLLEXPORT void Leviathan::Gui::GuiAnimateable::RemoveActionFromQueue(const size_t index){
	// decrement reference with release and remove from vector //
	SAFE_RELEASE(AnimationQueue[index]);
	AnimationQueue.erase(AnimationQueue.begin()+index);
}

// ------------------------------------ //