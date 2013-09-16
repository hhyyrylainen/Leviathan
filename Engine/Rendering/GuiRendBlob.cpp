#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_GUIRBLOB
#include "GuiRendBlob.h"
#endif
#include "Graphics.h"
#include "Common\DataStoring\DataStore.h"
using namespace Leviathan;
// ------------------------------------ //

Leviathan::RenderingGBlob::RenderingGBlob(const int &id, const int &relativez, const int &slotid, const bool &hidden) : RelativeZ(relativez), 
	SlotID(slotid), Hidden(hidden), ID(id)
{

}
Leviathan::RenderingGBlob::~RenderingGBlob(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::RenderingGBlob::SetVisibility(const bool &visible){
	// store new value //
	Hidden = !visible;
	// force derived classes to update //
	_VisibilityChanged();
}

// ---------------- ColorQuardRendBlob -------------------- //
Leviathan::ColorQuadRendBlob::ColorQuadRendBlob(Graphics* graph, const int &id, const int &relativez, const int &slotid, const bool &hidden) : 
	RenderingGBlob(id, relativez, slotid, hidden), OwningContainer(NULL), Added(false)
{
	// create new object //
	Rendering::OverlayMaster* tmp = graph->GetOverlayMaster();

	Ogre::OverlayManager& tmpmanager = Rendering::OverlayMaster::GetManager();

	Panel = static_cast<Ogre::OverlayContainer*>(tmpmanager.createOverlayElement("Panel", GetNameForSingleObject()));
	Panel->setMaterialName("GuiGradient");
	
	_VisibilityChanged();
}
Leviathan::ColorQuadRendBlob::~ColorQuadRendBlob(){
	// remove overlay elements we created //
	if(Added){
		OwningContainer->removeChild(Panel->getName());
	}
	Rendering::OverlayMaster::GetManager().destroyOverlayElement(Panel);
}
void Leviathan::ColorQuadRendBlob::Update(Graphics* graph, const int &relativez, const Float2 &xypos, const Float4 &color, const Float4 &color2, 
	const Float2 &size, int colortranstype, int coordinatetype)
{
	// update z-order //
	RelativeZ = relativez;

	//Panel->getTechnique()->getPass(0)->getTextureUnitState(0)->setTextureName();

	Panel->SetRelativeZOrder((USHORT)RelativeZ);

	//TODO: set z order //
	Panel->setMetricsMode(coordinatetype == GUI_POSITIONABLE_COORDTYPE_RELATIVE ? Ogre::GMM_RELATIVE: Ogre::GMM_PIXELS);
	Panel->setPosition(xypos.X, xypos.Y);
	Panel->setDimensions(size.X, size.Y);

	// set colours //
	Panel->setCustomParameter(0, color);
	Panel->setCustomParameter(1, color2);
}

DLLEXPORT bool Leviathan::ColorQuadRendBlob::RenderWithOverlay(const int &basezorder, Ogre::OverlayContainer* bridgecontainer, 
	Rendering::OverlayMaster* rendering)
{
	OwningContainer = bridgecontainer;
	if(!Added){
		OwningContainer->addChild(Panel);
		Added = true;
	}

	// should be fine //
	return true;
}

void Leviathan::ColorQuadRendBlob::_VisibilityChanged(){
	// update overlay objects //
	Hidden ? Panel->hide(): Panel->show();
}

// ------------------ TextRendBlob ------------------ //
DLLEXPORT Leviathan::TextRendBlob::TextRendBlob(Graphics* graph, const int &id, const int &relativez, const int &slotid, const bool &hidden) : 
	RenderingGBlob(id, relativez, slotid, hidden), OwningContainer(NULL), Added(false)
{
	// create new object //
	Rendering::OverlayMaster* tmp = graph->GetOverlayMaster();

	Ogre::OverlayManager& tmpmanager = Rendering::OverlayMaster::GetManager();

	Text = static_cast<Ogre::TextAreaOverlayElement*>(tmpmanager.createOverlayElement("TextArea", GetNameForSingleObject()));


	_VisibilityChanged();
}

DLLEXPORT Leviathan::TextRendBlob::~TextRendBlob(){
	// remove overlay elements we created //
	if(Added){
		OwningContainer->removeChild(Text->getName());
	}
	Rendering::OverlayMaster::GetManager().destroyOverlayElement(Text);
}

DLLEXPORT void Leviathan::TextRendBlob::Update(int relativez, const Float2 &xypos, const Float4 &color, float sizemod, const wstring &text, 
	const wstring &font, int coordtype /*= GUI_POSITIONABLE_COORDTYPE_RELATIVE*/, bool fittobox /*= false*/, const Float2 box /*= (Float2)0*/, const 
	float &adjustcutpercentage /*= 0.4f*/)
{
	// update z-order //
	RelativeZ = relativez;

	wstring texttoset;

	if(fittobox){

		// use font manager class to adjust the size //

		size_t charindexfit;
		float hybridscale, maxscale;
		Float2 finalsize;

		Rendering::FontManager::AdjustTextSizeToFitBox(Rendering::FontManager::GetFontPtrFromName(Convert::WstringToString(font)), box, text, 
			coordtype, charindexfit, maxscale, hybridscale, finalsize, adjustcutpercentage);

		// adjust set text //
		if(charindexfit != text.length()-1){

			texttoset = text.substr(0, charindexfit+1)+L"...";
		} else {
			texttoset = text;
		}
		Text->setCharHeight(hybridscale);

	} else {
		texttoset = text;
		Text->setCharHeight((32.f/DataStore::Get()->GetHeight())*sizemod);
	}

	Text->SetRelativeZOrder((USHORT)RelativeZ);

	//TODO: set z order //
	Text->setMetricsMode(coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE ? Ogre::GMM_RELATIVE: Ogre::GMM_PIXELS);
	Text->setPosition(xypos.X, xypos.Y);
	//Text->setDimensions(size.X, size.Y);
	Text->setColour(color);
	Text->setFontName(Convert::WstringToString(font));
	Text->setAlignment(Ogre::TextAreaOverlayElement::Left);
	Text->setCaption(texttoset);
	
}

void Leviathan::TextRendBlob::_VisibilityChanged(){
	Hidden ? Text->hide(): Text->show();
}

DLLEXPORT bool Leviathan::TextRendBlob::RenderWithOverlay(const int &basezorder, Ogre::OverlayContainer* bridgecontainer, 
	Rendering::OverlayMaster* rendering)
{
	OwningContainer = bridgecontainer;
	if(!Added){
		OwningContainer->addChild(Text);
		Added = true;
	}
	// should be fine //
	return true;
}

