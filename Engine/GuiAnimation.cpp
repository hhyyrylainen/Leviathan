#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_ANIMATION_ACTION
#include "GuiAnimation.h"
#endif
using namespace Leviathan;
using namespace Gui;
// ------------------------------------ //
AnimationAction::AnimationAction(){
	Type = GUI_ANIMATION_ERROR;
}
AnimationAction::~AnimationAction(){
	SAFE_DELETE(Data);
}
// ------------------------------------ //
AnimationAction::AnimationAction(GUI_ANIMATION_ACTION type, void* data, int special, bool allowsimult){
	Type = type;
	Data = data;
	AllowSimultanous = allowsimult;
	SpecialInstr = special;
}
// ------------------------------------ //
GUI_ANIMATION_ACTION AnimationAction::GetType(){
	return Type;
}
// ------------------------------------ //
GuiAnimationTypeMove::GuiAnimationTypeMove(int xtarget, int ytarget, int whichfirst, float speed){
	X = xtarget;
	Y = ytarget;
	Priority = whichfirst;
	Speed = speed;
}


// ------------------------------------ //