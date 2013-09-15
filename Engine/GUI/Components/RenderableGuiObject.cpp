#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASERENDERABLE
#include "RenderableGuiObject.h"
#endif
using namespace Leviathan;
using namespace Gui;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::RenderableGuiObject::RenderableGuiObject(const int &zlocation, const size_t &gcomponentcount, const bool &hidden 
	/*= false*/) : Zorder(zlocation), GComponents(gcomponentcount, NULL), Hidden(hidden), OldHidden(Hidden), NoNUpdatedFrames(0), RBridge(NULL),
	Updated(false)
{
	
}

DLLEXPORT Leviathan::Gui::RenderableGuiObject::~RenderableGuiObject(){
	// release everything //

	// components should be safe to delete through base pointer //
	// no need to use release since render bridge deletion will delete all //
	SAFE_DELETE_VECTOR(GComponents);

	// set bridge to be deleted //
	if(RBridge.get() != NULL)
		RBridge->SetAsClosing();
	RBridge.reset();
}

bool Leviathan::Gui::RenderableGuiObject::VerifyRenderingBridge(Graphics* graph, const int &ID){
	if(RBridge.get())
		return true;

	// try to create new rendering bridge for communicating with renderer //
	RBridge = shared_ptr<RenderBridge>(new RenderBridge(ID, Hidden, Zorder));

	// submit //
	graph->SubmitRenderBridge(RBridge);

	return true;
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //




