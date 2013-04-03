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
	while(DrawActions.size() != 0){
		SAFE_DELETE(DrawActions[0]);
		DrawActions.erase(DrawActions.begin());
	}
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
int RenderBridge::GetSlotIndex(int slot){
	for(unsigned int i = 0; i < DrawActions.size(); i++){
		if(DrawActions[i]->SlotID == slot)
			return i;
	}
	return -1;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //
