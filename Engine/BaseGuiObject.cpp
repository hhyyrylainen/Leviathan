#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_GUI
#include "BaseGuiObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
BaseGuiObject::BaseGuiObject() : Scripting(NULL){
	Objecttype = -1;
	HigherLevel = false;
	ObjectLevel = GUI_OBJECT_LEVEL_BASE;
}
BaseGuiObject::~BaseGuiObject(){

}
// ------------------------------------ //
int BaseGuiObject::CompareType(int compare){
	if(compare==this->Objecttype)
		return 0;
	if(compare > Objecttype)
		return 1;
	return -1;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //