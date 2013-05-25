#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_GUI
#include "BaseGuiObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
BaseGuiObject::BaseGuiObject(){
	Objecttype = -1;
	HigherLevel = false;
	ObjectLevel = GUI_OBJECT_LEVEL_BASE;
	Scripting = NULL;
	StaticCall = NULL;
}
BaseGuiObject::~BaseGuiObject(){
	//SAFE_DELETE(StaticCall);
}

ScriptCaller* BaseGuiObject::StaticCall = NULL;
// ------------------------------------ //
int BaseGuiObject::CompareType(int compare){
	if(compare==this->Objecttype)
		return 0;
	if(compare > Objecttype)
		return 1;
	return -1;
}
// ------------------------------------ //
ScriptCaller* BaseGuiObject::GetCallerForObjectType(BaseGuiObject* customize){
	if(StaticCall == NULL){
		// generate new //
		StaticCall = new ScriptCaller();
		// push specific functions //

	}
	// customize certain functions //
	return StaticCall;
}
// ------------------------------------ //

// ------------------------------------ //