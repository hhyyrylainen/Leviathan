#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_RENDERABLE
#include "BaseRenderable.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseRenderable::BaseRenderable(bool hidden) : Hidden(hidden){

}

DLLEXPORT Leviathan::BaseRenderable::~BaseRenderable(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseRenderable::SetHiddenState(bool hidden){
	Hidden = hidden;
	_OnHiddenStateUpdated();
}
