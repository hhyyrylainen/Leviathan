#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OVERLAYMASTER
#include "OverlayMaster.h"
#endif
#include "Exceptions\ExceptionNotFound.h"
using namespace Leviathan;
using namespace Rendering;
// ------------------------------------ //
DLLEXPORT Leviathan::Rendering::OverlayMaster::OverlayMaster(Ogre::SceneManager* scene, Ogre::Viewport* mainoverlayview, FontManager* fontloader){
	// create an overlay system //
	_OverlaySystem = new Ogre::OverlaySystem();
	scene->addRenderQueueListener(_OverlaySystem);

	// create the default overlay //
	Ogre::OverlayManager& tmpmanager = Ogre::OverlayManager::getSingleton();
	// Create an overlay
	MainOverlay = tmpmanager.create("MainOverlay");

	MainOverlay->setZOrder(100);

	// fonts are now loaded by default GUI instance
	//// load fonts before any GUI objects //
	//if(!fontloader->LoadFontByName(L"Arial")){

	//	throw ExceptionNotFound(L"Arial font not found", 404, __WFUNCTION__, L"file", L"Arial.ttf");
	//}

	// create panels for each window //
	MainWindowPanel = static_cast<Ogre::OverlayContainer*>(tmpmanager.createOverlayElement("Panel", "MainWindowBasePanel00"));
	MainWindowPanel->setMetricsMode(Ogre::GMM_RELATIVE);
	// spans the whole window and is transparent //
	MainWindowPanel->setPosition(0.0f, 0.0f);
	MainWindowPanel->setDimensions(1.f, 1.f);


	// add the window containers to the overlay //
	MainOverlay->add2D(MainWindowPanel);

	CreateTest(mainoverlayview);

	SetContainerVisibleInRenderTarget(MainWindowPanel, mainoverlayview);

	// make the overlay visible //
	MainOverlay->show();
}

DLLEXPORT Leviathan::Rendering::OverlayMaster::~OverlayMaster(){
	// delete listeners here //

}


DLLEXPORT void Leviathan::Rendering::OverlayMaster::Release(){
	// release overlay here //
	SAFE_DELETE(_OverlaySystem);
}


// ------------------------------------ //



// ------------------------------------ //
void Leviathan::Rendering::OverlayMaster::CreateTest(Ogre::Viewport* visible){

	//Ogre::OverlayManager& tmpmanager = Ogre::OverlayManager::getSingleton();

	//// create a test object //
	//Ogre::OverlayContainer* panel = static_cast<Ogre::OverlayContainer*>(tmpmanager.createOverlayElement("Panel", "Panel01"));
	//panel->setMetricsMode(Ogre::GMM_RELATIVE);
	//panel->setPosition(0.1f, 0.1f);
	//panel->setDimensions(0.1f, 0.1f);
	//panel->setMaterialName("BaseWhite");

	//// add text //
	//Ogre::TextAreaOverlayElement* text = static_cast<Ogre::TextAreaOverlayElement*>(tmpmanager.createOverlayElement("TextArea", "Text01"));

	//text->setFontName("Arial");

	//text->setMetricsMode(Ogre::GMM_RELATIVE);
	//text->setCharHeight(0.05f);

	//text->setAlignment(Ogre::TextAreaOverlayElement::Left);
	//text->setDimensions(0.1f, 0.1f);
	//text->setPosition(0.f, 0.f);
	//text->setColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f));
	//text->setColourTop(Ogre::ColourValue(0.0f, 0.0f, 0.0f));
	//text->setColourBottom(Ogre::ColourValue(0.0f, 0.0f, 0.0f));

	//text->setCaption("Test text that hopefully goes over...");
	//text->show();

	//panel->addChild(text);

	//// add the test container to the overlay //
	//MainWindowPanel->addChild(panel);

	//SetContainerVisibleInRenderTarget(panel, visible);
}

DLLEXPORT void Leviathan::Rendering::OverlayMaster::SetContainerVisibleInRenderTarget(Ogre::OverlayContainer* container, Ogre::Viewport* viewport){
	// add to list the managing class for deletion later //
	OverlaysPreferrers.push_back(unique_ptr<OverlayPreferer>(new OverlayPreferer(container, viewport)));
	// listening //
	viewport->getTarget()->addListener(OverlaysPreferrers.back().get());
}
// ------------------------------------ //
DLLEXPORT Ogre::OverlayContainer* Leviathan::Rendering::OverlayMaster::CreateContainerForRenderBridge(const string &nameofpanel, 
	Ogre::OverlayContainer* owningwindowpanel, const bool &hidden /*= false*/)
{
	// get manager singleton //
	Ogre::OverlayManager& tmpmanager = Ogre::OverlayManager::getSingleton();

	// create the panel with the name //
	Ogre::OverlayContainer* tmpcontainer = static_cast<Ogre::OverlayContainer*>(tmpmanager.createOverlayElement("Panel", nameofpanel));
	tmpcontainer->setMetricsMode(Ogre::GMM_RELATIVE);
	// spans the whole window and is transparent //
	tmpcontainer->setPosition(0.0f, 0.0f);
	tmpcontainer->setDimensions(1.f, 1.f);

	// add to owning window's main container //
	owningwindowpanel->addChild(tmpcontainer);

	// hide the panel if wanted //
	hidden ? tmpcontainer->hide(): tmpcontainer->show();

	// return the created panel //
	return tmpcontainer;
}
// ------------------ OverlayPreferer ------------------ //
Leviathan::Rendering::OverlayPreferer::OverlayPreferer(Ogre::OverlayContainer* target, Ogre::Viewport* activeviewport) : RTarget(target),
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
		for(set<Ogre::OverlayContainer*>::iterator i = setOverlays.begin(); i != setOverlays.end(); i++)
		{
			(*i)->hide();
		}
		RTarget->show();
	}
}

std::set<Ogre::OverlayContainer*> Leviathan::Rendering::OverlayPreferer::setOverlays;



