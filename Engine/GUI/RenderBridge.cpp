#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDER_BRIDGE
#include "RenderBridge.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
RenderBridge::RenderBridge(int id, bool hidden, int zorder) : OwningOverlay(NULL), ZVal(zorder), Hidden(hidden), ID(id), BridgesContainer(NULL){

	WantsToClose = false;
}

RenderBridge::~RenderBridge(){
	SAFE_DELETE_VECTOR(DrawActions);

	// delete the container //
	OwningOverlay->GetMainContainerInWindow()->removeChild(BridgesContainer->getName());
	Rendering::OverlayMaster::GetManager().destroyOverlayElement(BridgesContainer);
}
// ------------------------------------ //
void RenderBridge::SetHidden(int slot, bool hidden){
	// loop and set corresponding action's visibility //
	for(size_t i = 0; i < DrawActions.size(); i++){
		if(DrawActions[i]->SlotID == slot){

			DrawActions[i]->SetVisibility(!hidden);
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
DLLEXPORT bool Leviathan::RenderBridge::RenderActions(Rendering::OverlayMaster* rendering){
	// update owning overlay //
	OwningOverlay = rendering;

	// create container if missing //
	if(!BridgesContainer){

		// create a name for panel and then create it //
		string panelname = "RenderBridge_"+Convert::ToString(ID);
		BridgesContainer = rendering->CreateContainerForRenderBridge(panelname, rendering->GetMainContainerInWindow(), Hidden);

		Hidden ? BridgesContainer->hide(): BridgesContainer->show();
	}


	for(size_t i = 0; i < DrawActions.size(); i++){
		// real action to render //
		RenderingGBlob* tempptr = DrawActions[i];

		if(!tempptr->RenderWithOverlay(ZVal, BridgesContainer, OwningOverlay))
			return false;
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RenderBridge::SetVisibility(const bool &visible){
	// set internal value and set container's value //
	Hidden = !visible;

	Hidden ? BridgesContainer->hide(): BridgesContainer->show();
}

DLLEXPORT void Leviathan::RenderBridge::DeleteBlobOnIndex(const size_t &index){
	ARR_INDEX_CHECK((int)index, DrawActions.size()){

		// delete and erase //
		SAFE_DELETE(DrawActions[index]);
		DrawActions.erase(DrawActions.begin()+index);
	}
}

// ------------------------------------ //
