#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OVERLAYMASTER
#include "OverlayMaster.h"
#endif
#include "Exceptions\ExceptionNotFound.h"
#include "GUI\GuiManager.h"
using namespace Leviathan;
using namespace Rendering;
// ------------------------------------ //
DLLEXPORT Leviathan::Rendering::OverlayMaster::OverlayMaster(){

}

DLLEXPORT Leviathan::Rendering::OverlayMaster::~OverlayMaster(){
	// delete listeners here //

}


DLLEXPORT void Leviathan::Rendering::OverlayMaster::Release(){

}


// ------------------------------------ //



// ------------------------------------ //
DLLEXPORT void Leviathan::Rendering::OverlayMaster::SetGUIVisibleInViewport(Gui::GuiManager* container, Ogre::Viewport* viewport){
	// add to list the managing class for deletion later //
	OverlaysPreferrers.push_back(unique_ptr<OverlayPreferer>(new OverlayPreferer(container, viewport)));
	// listening //
	viewport->getTarget()->addListener(OverlaysPreferrers.back().get());
}
// ------------------ OverlayPreferer ------------------ //
Leviathan::Rendering::OverlayPreferer::OverlayPreferer(Gui::GuiManager* target, Ogre::Viewport* activeviewport) : RTarget(target),
	_Viewport(activeviewport)
{
	OverlayPreferer::setOverlays.insert(target);
}

Leviathan::Rendering::OverlayPreferer::~OverlayPreferer(){
	OverlayPreferer::setOverlays.erase(setOverlays.find(RTarget));
}
// ------------------------------------ //
void Leviathan::Rendering::OverlayPreferer::postViewportUpdate(const Ogre::RenderTargetViewportEvent & evt){

}

void Leviathan::Rendering::OverlayPreferer::preViewportUpdate(const Ogre::RenderTargetViewportEvent & evt){
	if(evt.source == _Viewport)
	{
		for(set<Gui::GuiManager*>::iterator i = setOverlays.begin(); i != setOverlays.end(); i++)
		{
			(*i)->SetVisible(false);
		}
		RTarget->SetVisible(true);
	}
}

std::set<Gui::GuiManager*> Leviathan::Rendering::OverlayPreferer::setOverlays;



