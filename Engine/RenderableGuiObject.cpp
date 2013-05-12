#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASE_RENDERABLE
#include "RenderableGuiObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
RenderableGuiObject::RenderableGuiObject(){
	Objecttype = -1;
	HigherLevel = true;
	ObjectLevel = GUI_OBJECT_LEVEL_RENDERABLE;
	NoNUpdatedFrames = 0;
}
RenderableGuiObject::~RenderableGuiObject(){

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //