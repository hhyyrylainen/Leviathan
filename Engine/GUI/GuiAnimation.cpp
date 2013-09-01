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
GuiAnimationTypeMove::GuiAnimationTypeMove(float xtarget, float ytarget, int whichfirst, float speed){
	X = xtarget;
	Y = ytarget;
	Priority = whichfirst;
	Speed = speed;
}

// ------------------ Factory functions ------------------ //
DLLEXPORT AnimationAction* Leviathan::Gui::CreateAnimationActionMove(float xtarget, float ytarget, int whichfirst, float speed, bool allowsimult){
	return new AnimationAction(GUI_ANIMATION_MOVE, new GuiAnimationTypeMove(xtarget, ytarget, whichfirst, speed), 0, allowsimult);
}

DLLEXPORT AnimationAction* Leviathan::Gui::CreateAnimationActionVisibility(bool visible){
	return new AnimationAction(visible ? GUI_ANIMATION_SHOW: GUI_ANIMATION_HIDE, NULL, 0, false);
}
