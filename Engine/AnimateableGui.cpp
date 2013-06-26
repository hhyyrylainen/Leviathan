#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_ANIMATEABLE
#include "AnimateableGui.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
GuiAnimateable::GuiAnimateable() : AnimationQueue(){
	Objecttype = -1;
	HigherLevel = true;
	ObjectLevel = GUI_OBJECT_LEVEL_ANIMATEABLE;
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
void GuiAnimateable::QueueAction(shared_ptr<AnimationAction> act){
	AnimationQueue.push_back(act);
}
// ------------------------------------ //