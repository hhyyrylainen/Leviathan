#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_MAIN
#include "GuiManager.h"
#endif
#include "Engine.h"
#include "Script/ScriptInterface.h"
#include "FileSystem.h"
#include "Rendering/Graphics.h"
#include <boost/assign/list_of.hpp>
#include "Rendering/GUI/FontManager.h"
#include "GuiCollection.h"
#include "Common/DataStoring/DataStore.h"
#include "Common/DataStoring/DataBlock.h"
#include "Rendering/GraphicalInputEntity.h"
#include "Window.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreManualObject.h"
#include "OgreHardwarePixelBuffer.h"
#include "CEGUI/System.h"
#include "CEGUI/WindowManager.h"
#include "Common/Misc.h"
#include "CEGUI/RenderTarget.h"
#include "CEGUI/Window.h"
#include "ObjectFiles/ObjectFileProcessor.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
Leviathan::Gui::GuiManager::GuiManager() : ID(IDFactory::GetID()), Visible(true), GuiMouseUseUpdated(true), GuiDisallowMouseCapture(true),
	LastTimePulseTime(Misc::GetThreadSafeSteadyTimePoint()), MainGuiManager(false)
{
	
}
Leviathan::Gui::GuiManager::~GuiManager(){

	
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::Init(AppDef* vars, Graphics* graph, GraphicalInputEntity* window){
	GUARD_LOCK_THIS_OBJECT();

	ThisWindow = window;

	// Detect if this is the first GuiManager //
	if(GraphicalInputEntity::GetGlobalWindowCount() == 1)
		MainGuiManager = true;

	Window* wind = window->GetWindow();

	// Create Ogre resources //
	if(!_CreateInternalOgreResources(window->GetWindow()->GetOverlayScene())){

		Logger::Get()->Error(L"GuiManager: Init: failed to create internal Ogre resources");
		return false;
	}
	
	// Setup this window's context //
	GuiContext = &CEGUI::System::getSingleton().createGUIContext(ThisWindow->GetCEGUIRenderer()->getDefaultRenderTarget());


	// Set Simonetta as the default font //
	GuiContext->setDefaultFont("Simonetta-Regular");

	// Set the taharez looks active //
	SetMouseTheme(L"TaharezLook/MouseArrow");
	GuiContext->setDefaultTooltipType("TaharezLook/Tooltip");

	return true;
}

void Leviathan::Gui::GuiManager::Release(){
	GUARD_LOCK_THIS_OBJECT();
	// default mouse back //
	SetMouseTheme(L"none");

	// Release objects first //

	for(size_t i = 0; i < Objects.size(); i++){
		// object's release function will do everything needed (even deleting if last reference) //
		SAFE_RELEASE(Objects[i]);
	}
	Objects.clear();

	// GuiCollections are now also reference counted //
	for(size_t i = 0; i < Collections.size(); i++){
		SAFE_RELEASE(Collections[i]);
	}

	Collections.clear();


	// Destroy the GUI //
	CEGUI::System::getSingleton().destroyGUIContext(*GuiContext);
	GuiContext = NULL;

	_ReleaseOgreResources();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::ProcessKeyDown(OIS::KeyCode key, int specialmodifiers){
	GUARD_LOCK_THIS_OBJECT();
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetTogglingKey().Match(key, specialmodifiers, false) && Collections[i]->GetAllowEnable()){
			// is a match, toggle //
			Collections[i]->ToggleState();

			return true;
		}
	}


	return false;
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionStateProxy(string name, bool state){
	// call the actual function //
	SetCollectionState(Convert::StringToWstring(name), state);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionState(const wstring &name, bool state){
	GUARD_LOCK_THIS_OBJECT();
	// find collection with name and set it's state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetState() != state){
				Logger::Get()->Info(L"Setting Collection "+Collections[i]->GetName()+L" state "+Convert::ToWstring(state));
				Collections[i]->ToggleState();
			}
			return;
		}
	}
	// Complain //
	Logger::Get()->Warning(L"GuiManager: SetCollectionState: couldn't find collection with name: "+name);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionAllowEnableState(const wstring &name, bool allow /*= true*/){
	GUARD_LOCK_THIS_OBJECT();
	// find collection with name and set it's allow enable state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetAllowEnable() != allow){
				Logger::Get()->Info(L"Setting Collection "+Collections[i]->GetName()+L" allow enable state "+Convert::ToWstring(allow));
				Collections[i]->ToggleAllowEnable();
			}
			return;
		}
	}
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::GuiTick(int mspassed){
	GUARD_LOCK_THIS_OBJECT();

	// check if we want mouse //
	if(GuiMouseUseUpdated){
		GuiMouseUseUpdated = false;

		// scan if any collections keep GUI active //
		bool active = false;


		for(size_t i = 0; i < Collections.size(); i++){

			if(Collections[i]->KeepsGUIActive()){
				active = true;
				break;
			}
		}

		if(active != GuiDisallowMouseCapture){
			// state updated //
			GuiDisallowMouseCapture = active;

			if(GuiDisallowMouseCapture){
				// disable mouse capture //
				ThisWindow->SetMouseCapture(false);

			} else {

				// activate direct mouse capture //
				if(!ThisWindow->SetMouseCapture(true)){
					// failed, GUI must be forced to stay on //
					OnForceGUIOn();
					GuiDisallowMouseCapture = true;
					GuiMouseUseUpdated = true;
				}
			}
		}
	}
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnForceGUIOn(){
	DEBUG_BREAK;
}

void Leviathan::Gui::GuiManager::Render(){
	GUARD_LOCK_THIS_OBJECT();

	// Pass time //
	auto newtime = Misc::GetThreadSafeSteadyTimePoint();

	SecondDuration elapsed = newtime-LastTimePulseTime;

	GuiContext->injectTimePulse(elapsed.count());

	// Potentially pass to system //
	if(MainGuiManager){

		CEGUI::System::getSingleton().injectTimePulse(elapsed.count());
	}

	LastTimePulseTime = newtime;

	// Update inputs //
	ThisWindow->GetWindow()->GatherInput(GuiContext);

}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::OnResize(){
	GUARD_LOCK_THIS_OBJECT();

	// Notify the CEGUI system //
	CEGUI::System* const sys = CEGUI::System::getSingletonPtr();
	if(sys)
		sys->notifyDisplaySizeChanged(CEGUI::Sizef((float)ThisWindow->GetWindow()->GetWidth(), (float)ThisWindow->GetWindow()->GetHeight()));
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnFocusChanged(bool focused){
	GUARD_LOCK_THIS_OBJECT();
	
	// Notify our context //
	if(!focused)
		GuiContext->injectMouseLeaves();

}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::AddGuiObject(BaseGuiObject* obj){
	GUARD_LOCK_THIS_OBJECT();
	Objects.push_back(obj);
	return true;
}

void Leviathan::Gui::GuiManager::DeleteObject(int id){
	GUARD_LOCK_THIS_OBJECT();
	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id){

			SAFE_RELEASE(Objects[i]);
			Objects.erase(Objects.begin()+i);
			return;
		}
	}
}

int Leviathan::Gui::GuiManager::GetObjectIndexFromId(int id){
	GUARD_LOCK_THIS_OBJECT();
	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id)
			return i;
	}
	return -1;
}

BaseGuiObject* Leviathan::Gui::GuiManager::GetObject(unsigned int index){
	GUARD_LOCK_THIS_OBJECT();
	ARR_INDEX_CHECK(index, Objects.size()){
		return Objects[index];
	}
	return NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::LoadGUIFile(const wstring &file){

	// Parse the file //
	auto data = ObjectFileProcessor::ProcessObjectFile(file);

	if(!data){
		return false;
	}

	NamedVars& varlist = *data->GetVariables();

	// we need to load the corresponding rocket file first //
	wstring relativepath;
	// get path //
	ObjectFileProcessor::LoadValueFromNamedVars<wstring>(varlist, L"GUIBaseFile", relativepath, L"", true,
		L"GuiManager: LoadGUIFile: no base file defined (in "+file+L") : ");

	if(!relativepath.size()){

		return false;
	}
	
	// Load it //
	CEGUI::Window* rootwindow = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(Convert::WstringToString(relativepath));

	// Check did it work //
	if(!rootwindow){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse layout file: "+relativepath);
		return false;
	}

	// We need to lock now //
	GUARD_LOCK_THIS_OBJECT();

	// Set it as the visible sheet //
	GuiContext->setRootWindow(rootwindow);


	// temporary object data stores //
	vector<BaseGuiObject*> TempOs;

	// reserve space //
	size_t totalcount = data->GetTotalObjectCount();

	TempOs.reserve(totalcount);


	for(size_t i = 0; i < totalcount; i++){

		auto objecto = data->GetObjectFromIndex(i);

		// Check what type the of the object is //
		if(objecto->GetTypeName() == L"GuiCollection"){

			if(!GuiCollection::LoadCollection(this, *objecto)){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load collection, named "+objecto->GetName());
				continue;
			}

			continue;

		} else if(objecto->GetTypeName() == L"GuiObject"){

			// try to load //
			if(!BaseGuiObject::LoadFromFileStructure(this, TempOs, *objecto)){

				// report error //
				Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: failed to load GuiObject, named "+objecto->GetName());
				continue;
			}

			continue;
		}

		Logger::Get()->Error(L"GuiManager: ExecuteGuiScript: Unrecognized type! typename: "+objecto->GetTypeName());
	}
	

	for(size_t i = 0; i < TempOs.size(); i++){

		// add to real objects //
		AddGuiObject(TempOs[i]);
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseTheme(const wstring &tname){
	GUARD_LOCK_THIS_OBJECT();

	if(tname == L"none"){

		// show default window cursor //
		ThisWindow->GetWindow()->SetHideCursor(false);
		if(tname == L"none")
			return;
	}

	// Set it active //
	GuiContext->getMouseCursor().setDefaultImage(Convert::WstringToString(tname));

	// hide window cursor //
	ThisWindow->GetWindow()->SetHideCursor(true);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseFileVisibleState(bool state){
	GUARD_LOCK_THIS_OBJECT();
	// Set mouse drawing flag //
}
// ------------------------------------ //
DLLEXPORT CEGUI::GUIContext* Leviathan::Gui::GuiManager::GetMainContext(){
	return GuiContext;
}
// ----------------- collection managing --------------------- //
void GuiManager::AddCollection(GuiCollection* add){
	GUARD_LOCK_THIS_OBJECT();
	Collections.push_back(add);
}
GuiCollection* Leviathan::Gui::GuiManager::GetCollection(const int &id, const wstring &name){
	GUARD_LOCK_THIS_OBJECT();
	// look for collection based on id or name //
	for(size_t i = 0; i < Collections.size(); i++){
		if(id >= 0){
			if(Collections[i]->GetID() != id){
				// no match //
				continue;
			}
		} else {
			// name should be specified, check for it //
			if(Collections[i]->GetName() != name){
				continue;
			}
		}

		// match
		return Collections[i];
	}

	return NULL;
}
// -------------------------------------- //
bool Leviathan::Gui::GuiManager::_CreateInternalOgreResources(Ogre::SceneManager* windowsscene){

	// Well we don't anymore need anything here since GuiView handles this //



	// Create a quad for mouse displaying //

	// Use RENDER_QUEUE_MAX to render on top of everything



	return true;
}

void Leviathan::Gui::GuiManager::_ReleaseOgreResources(){
	// We probably don't need to do anything //

}



