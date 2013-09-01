#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDER_BRIDGE
#include "RenderBridge.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
RenderBridge::RenderBridge(){
	WantsToClose = true;
	ZVal = -1;
	Hidden = true;
	ID = -1;
}
RenderBridge::RenderBridge(int id, bool hidden, int zorder){
	WantsToClose = false;
	ZVal = zorder;
	Hidden = hidden;
	ID = id;
}
RenderBridge::~RenderBridge(){
	SAFE_DELETE_VECTOR(DrawActions);
}
// ------------------------------------ //
void RenderBridge::SetHidden(int slot, bool hidden){
	for(unsigned int i = 0; i < DrawActions.size(); i++){
		if(DrawActions[i]->SlotID == slot){

			DrawActions[i]->Hidden = hidden;
			break;
		}
	}
}
size_t RenderBridge::GetSlotIndex(int slot){
	for(size_t i = 0; i < DrawActions.size(); i++){
		if(DrawActions[i] == NULL)
			continue;
		if(DrawActions[i]->SlotID == slot)
			return i;
	}
	// not found just push back NULL and return it //
	DrawActions.push_back(NULL);
	return DrawActions.size()-1;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //
