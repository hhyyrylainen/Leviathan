#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_RENDERABLE
#include "BaseRenderable.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
BaseRenderable::BaseRenderable(){
	Frames = 0;
	Updated = true;
}
BaseRenderable::~BaseRenderable(){

}
// ------------------------------------ //
bool BaseRenderable::IsHidden(){
	return Hidden;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //