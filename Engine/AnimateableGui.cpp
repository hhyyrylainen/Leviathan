#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_ANIMATEABLE
#include "AnimateableGui.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
GuiAnimateable::GuiAnimateable(){
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
void GuiAnimateable::QueueAction(AnimationAction* act){
	Queue.push_back(act);
}
//void GuiAnimateable::QueueActionForObject(GuiAnimateable* object, AnimationAction* action){
//	try{
//	object->Queue.push_back(action);
//	}
//	catch(exception e){
//		Logger::Print("error");
//	}
//	catch(string e){
//		Logger::Print("error");
//	}
//	catch(...){
//		Logger::Print("error");
//	}
//}
// ------------------------------------ //
void GuiAnimateable::SetValue(int semanticid, float val){
	// overload this in all child classes for getting values
	return;
}
float GuiAnimateable::GetValue(int semanticid){
	// overload this in all child classes for getting values
	return -1.0f;
}
// ------------------------------------ //