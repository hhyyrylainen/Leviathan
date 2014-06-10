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
#include "CEGUI/AnimationManager.h"
#include "CEGUI/AnimationInstance.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "CEGUI/Clipboard.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#ifdef WIN32


#else
#include "Window.h"

#endif // WIN32
#include "Exceptions/ExceptionNotFound.h"

// ------------------ GuiClipboardHandler ------------------ //
//! \brief Platform dependent clipboard handler
//! \todo Add support for linux
class Leviathan::Gui::GuiClipboardHandler : public CEGUI::NativeClipboardProvider{
public:
#ifdef WIN32
	GuiClipboardHandler(Window* windprovider) : HWNDSource(windprovider), OurOwnedBuffer(NULL){
	}

#endif // WIN32

	~GuiClipboardHandler(){

		SAFE_DELETE(OurOwnedBuffer);
	}

	//! \brief Returns true when this object can manage the clipboard on this platform
	static bool WorksOnPlatform(){
#ifdef WIN32

		return true;
#elif __linux__

		// Should be available but it isn't //
		return false;

#else
		return false;

#endif // WIN32
	}


#ifdef WIN32

	virtual void sendToClipboard(const CEGUI::String& mimeType, void* buffer, size_t size){
		// Ignore non-text setting //
		if(mimeType != "text/plain"){

			return;
		}


		if(!OpenClipboard(HWNDSource->GetHandle()))	{

			Logger::Get()->Error(L"GuiClipboardHandler: failed to open the clipboard", GetLastError());
			return;
		}

		// Clear the clipboard //
		if(!EmptyClipboard()){

			Logger::Get()->Error(L"GuiClipboardHandler: failed to empty the clipboard", GetLastError());
			return;
		}

		// Convert the line endings //
		string convertedstring = StringOperations::ChangeLineEndsToWindowsString(string(reinterpret_cast<char*>(buffer), size));
		

		// Allocate global data for the text //
		HGLOBAL globaldata = GlobalAlloc(GMEM_FIXED, convertedstring.size()+1);

		memcpy_s(globaldata, convertedstring.size()+1, convertedstring.c_str(), convertedstring.size());

		reinterpret_cast<char*>(globaldata)[convertedstring.size()] = 0;

		// Set the text data //
		if(::SetClipboardData(CF_TEXT, globaldata) == NULL){

			Logger::Get()->Error(L"GuiClipboardHandler: failed to set the clipboard contents", GetLastError());
			CloseClipboard();
			GlobalFree(globaldata);
			return;
		}

		// All done, close clipboard to allow others to use it, too //
		CloseClipboard();
	}

	virtual void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size){
		// Open the clipboard first //
		if(OpenClipboard(HWNDSource->GetHandle())){

			// Only retrieve text //
			HANDLE datahandle = GetClipboardData(CF_TEXT);

			if(datahandle == INVALID_HANDLE_VALUE || datahandle == NULL){
				return;
			}

			// Lock the data for reading //
			char* sourcebuff = reinterpret_cast<char*>(GlobalLock(datahandle));
			
			string fromclipdata = sourcebuff;

			// Unlock the global memory and clipboard //
			GlobalUnlock(datahandle);
			CloseClipboard();

			// Convert line ends to something nice //
			fromclipdata = StringOperations::ChangeLineEndsToUniversalString(fromclipdata);

			// Clear old data //
			SAFE_DELETE(OurOwnedBuffer);

			// Create a new data buffer //
			OurOwnedBuffer = new char[fromclipdata.size()+1];

			// Return the data //
			buffer = OurOwnedBuffer;

			memcpy_s(buffer, fromclipdata.size()+1, fromclipdata.c_str(), fromclipdata.size());

			// Make sure there is a null terminator //
			reinterpret_cast<char*>(buffer)[fromclipdata.size()] = 0;

			mimeType = "text/plain";
			size = fromclipdata.size()+1;
		}
	}



#else

	// Nothing //
	virtual void sendToClipboard(const CEGUI::String& mimeType, void* buffer, size_t size){
		throw ExceptionNotFound(L"The method or operation is not implemented.", 0, __WFUNCTION__, L"Platform", L"linux");
	}

	virtual void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size){
		throw ExceptionNotFound(L"The method or operation is not implemented.", 0, __WFUNCTION__, L"Platform", L"linux");
	}

#endif // WIN32


private:

#ifdef WIN32

	Window* HWNDSource;

#endif // WIN32


	//! The owned buffer, which has to be deleted by this
	void* OurOwnedBuffer;

};




// ------------------ GuiManager ------------------ //
Leviathan::Gui::GuiManager::GuiManager() : ID(IDFactory::GetID()), Visible(true), GuiMouseUseUpdated(true), GuiDisallowMouseCapture(true),
	LastTimePulseTime(Misc::GetThreadSafeSteadyTimePoint()), MainGuiManager(false), ThisWindow(NULL), GuiContext(NULL), FileChangeID(0),
	_GuiClipboardHandler(NULL)
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
	
		
	// Create Ogre resources //
	if(!_CreateInternalOgreResources(window->GetWindow()->GetOverlayScene())){

		Logger::Get()->Error(L"GuiManager: Init: failed to create internal Ogre resources");
		return false;
	}

	// Create the clipboard handler for this window //
	_GuiClipboardHandler = new GuiClipboardHandler(window->GetWindow());
	
	// Setup this window's context //
	GuiContext = &CEGUI::System::getSingleton().createGUIContext(ThisWindow->GetCEGUIRenderer()->getDefaultRenderTarget());


	// Set Simonetta as the default font //
	GuiContext->setDefaultFont("Simonetta-Regular");

	// Set the taharez looks active //
	SetMouseTheme(L"TaharezLook/MouseArrow");
	GuiContext->setDefaultTooltipType("TaharezLook/Tooltip");


	// Make the clipboard play nice //
	if(GraphicalInputEntity::GetGlobalWindowCount() == 1){

		// Only one clipboard is needed //
		if(_GuiClipboardHandler->WorksOnPlatform())
			CEGUI::System::getSingleton().getClipboard()->setNativeProvider(_GuiClipboardHandler);
	}


	// Store the initial time //
	LastTimePulseTime = Misc::GetThreadSafeSteadyTimePoint();


	return true;
}

void Leviathan::Gui::GuiManager::Release(){
	GUARD_LOCK_THIS_OBJECT();

	// Stop with the file updates //
	if(FileChangeID){


		auto tmphandler = ResourceRefreshHandler::Get();

		if(tmphandler){

			tmphandler->StopListeningForFileChanges(FileChangeID);
		}
		FileChangeID = 0;
	}

	// default mouse back //
	SetMouseTheme(L"none");

	// Release objects first //

	for(size_t i = 0; i < Objects.size(); i++){

		Objects[i]->ReleaseData();
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

	SAFE_DELETE(_GuiClipboardHandler);

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

	float changval = elapsed.count();

	GuiContext->injectTimePulse(changval);

	// Potentially pass to system //
	if(MainGuiManager){

		CEGUI::System::getSingleton().injectTimePulse(changval);
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

			Objects[i]->ReleaseData();
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
DLLEXPORT bool Leviathan::Gui::GuiManager::LoadGUIFile(const wstring &file, bool nochangelistener /*= false*/){

	// Parse the file //
	auto data = ObjectFileProcessor::ProcessObjectFile(file);

	if(!data){
		return false;
	}

	MainGUIFile = file;

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
	CEGUI::Window* rootwindow = NULL;
	try{

		rootwindow = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(Convert::WstringToString(relativepath));

	} catch(const Ogre::Exception &e){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to locate file: "+relativepath+L":");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e.what()));
		return false;
	} catch(const CEGUI::GenericException &e2){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse CEGUI layout: "+relativepath+L":");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e2.what()));
		return false;
	} catch(const CEGUI::InvalidRequestException &e3){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse CEGUI layout: "+relativepath+L":");
		Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e3.what()));
		return false;
	}

	// Check did it work //
	if(!rootwindow){

		Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse layout file: "+relativepath);
		return false;
	}

	// We need to lock now //
	GUARD_LOCK_THIS_OBJECT();



	// Look for animation files //
	auto animslist = varlist.GetValueDirectRaw(L"GUIAnimations");

	if(animslist){

		if(!animslist->CanAllBeCastedToType<wstring>()){

			Logger::Get()->Warning(L"GuiManager: LoadGUIFile: gui file has defined gui animation files in wrong format (expected list of strings), file: "+relativepath);

		} else {

			// Load them //
			for(size_t i = 0; i < animslist->GetVariableCount(); i++){

				wstring curfile;
				animslist->GetValue(i).ConvertAndAssingToVariable<wstring>(curfile);

				// Check is the file already loaded //
				{
					ObjectLock gguard(GlobalGUIMutex);

					if(IsAnimationFileLoaded(gguard, curfile)){

						// Don't load again //
						continue;
					}

					// Set as loaded //
					SetAnimationFileLoaded(gguard, curfile);
				}

				try{

					CEGUI::AnimationManager::getSingleton().loadAnimationsFromXML(Convert::WstringToString(curfile));

				} catch(const Ogre::Exception &e){

					Logger::Get()->Warning(L"GuiManager: LoadGUIFile: failed to locate gui animation file: "+curfile+L":");
					Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e.what()));

				} catch(const CEGUI::GenericException &e2){

					Logger::Get()->Error(L"GuiManager: LoadGUIFile: failed to parse CEGUI animation file layout: "+curfile+L":");
					Logger::Get()->Write(L"\t> "+Convert::StringToWstring(e2.what()));
				}
			}
		}
	}

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

	// This avoids having more and more change listeners each reload //
	if(!nochangelistener){
		// Listen for file changes //
		auto tmphandler = ResourceRefreshHandler::Get();
	
		if(tmphandler){

			// \todo Detect if the files are in different folders and start multiple listeners
			std::vector<const wstring*> targetfiles = boost::assign::list_of(&file)(&relativepath);

			tmphandler->ListenForFileChanges(targetfiles, boost::bind(&GuiManager::_FileChanged, this, _1, _2), FileChangeID);
		}
	}



	return true;
}

DLLEXPORT void Leviathan::Gui::GuiManager::UnLoadGUIFile(){

	GUARD_LOCK_THIS_OBJECT();

	// Unload all objects //
	for(size_t i = 0; i < Objects.size(); i++){
		
		Objects[i]->ReleaseData();
		SAFE_RELEASE(Objects[i]);
	}

	Objects.clear();

	// Unload all collections //
	for(size_t i = 0; i < Collections.size(); i++){
		SAFE_RELEASE(Collections[i]);
	}

	Collections.clear();


	// Unload the CEGUI file //
	auto curroot = GuiContext->getRootWindow();

	CEGUI::WindowManager::getSingleton().destroyWindow(curroot);

	GuiContext->setRootWindow(NULL);
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
// ------------------------------------ //
void Leviathan::Gui::GuiManager::_FileChanged(const wstring &file, ResourceFolderListener &caller){
	// Any updated file will cause whole reload //
	GUARD_LOCK_THIS_OBJECT();

	// Store the current state //
	auto currentstate = GetGuiStates();

	UnLoadGUIFile();

	// Now load it //
	if(!LoadGUIFile(MainGUIFile, true)){

		Logger::Get()->Error(L"GuiManager: file changed: couldn't load updated file: "+MainGUIFile);
	}

	// Apply back the old states //
	ApplyGuiStates(currentstate.get());

	// Mark everything as non-updated //
	caller.MarkAllAsNotUpdated();

}
// ------------------ Static part ------------------ //
std::vector<wstring> Leviathan::Gui::GuiManager::LoadedAnimationFiles;

boost::recursive_mutex Leviathan::Gui::GuiManager::GlobalGUIMutex;

bool Leviathan::Gui::GuiManager::IsAnimationFileLoaded(ObjectLock &lock, const wstring &file){
	assert(lock.owns_lock(&GlobalGUIMutex) && "Wrong object locked in GuiManager::IsAnimationFileLoaded");

	for(size_t i = 0; i < LoadedAnimationFiles.size(); i++){

		if(LoadedAnimationFiles[i] == file){

			return true;
		}
	}

	// Not found, must not be loaded then //
	return false;
}

void Leviathan::Gui::GuiManager::SetAnimationFileLoaded(ObjectLock &lock, const wstring &file){
	assert(lock.owns_lock(&GlobalGUIMutex) && "Wrong object locked in GuiManager::IsAnimationFileLoaded");

	LoadedAnimationFiles.push_back(file);
}

DLLEXPORT void Leviathan::Gui::GuiManager::KillGlobalCache(){
	ObjectLock guard(GlobalGUIMutex);

	// Release the memory to not look like a leak //
	LoadedAnimationFiles.clear();

	auto single = CEGUI::AnimationManager::getSingletonPtr();

	if(single)
		single->destroyAllAnimations();
}
// ------------------------------------ //
DLLEXPORT CEGUI::Window* Leviathan::Gui::GuiManager::GetWindowByStringName(const string &namepath){
	GUARD_LOCK_THIS_OBJECT();
	try{

		return GuiContext->getRootWindow()->getChild(namepath);

	} catch(const CEGUI::UnknownObjectException&){

		// Not found //
		return NULL;
	}
}

DLLEXPORT bool Leviathan::Gui::GuiManager::PlayAnimationOnWindow(const string &windowname, const string &animationname, bool applyrecursively, 
	const string &ignoretypenames)
{
	// First get the window //
	auto wind = GetWindowByStringName(windowname);

	if(!wind)
		return false;

	// Next create the animation instance //
	CEGUI::Animation* animdefinition = NULL;
	
	try{

		animdefinition = CEGUI::AnimationManager::getSingleton().getAnimation(animationname);

	} catch(const CEGUI::UnknownObjectException&){

		return false;
	}


	if(!animdefinition)
		return false;


	_PlayAnimationOnWindow(wind, animdefinition, applyrecursively, ignoretypenames);
	return true;
}

DLLEXPORT bool Leviathan::Gui::GuiManager::PlayAnimationOnWindowProxy(const string &windowname, const string &animationname){
	return PlayAnimationOnWindow(windowname, animationname);
}

void Leviathan::Gui::GuiManager::_PlayAnimationOnWindow(CEGUI::Window* targetwind, CEGUI::Animation* animdefinition, bool recurse, 
	const string &ignoretypenames)
{
	// Apply only if the typename doesn't match ignored names //
	if(ignoretypenames.find(targetwind->getType().c_str()) == string::npos && targetwind->getName().at(0) != '_'){

		//Logger::Get()->Write(L"Animating thing: "+Convert::CharPtrToWstring(targetwind->getNamePath().c_str()));

		// Create an animation instance //
		CEGUI::AnimationInstance* createdanim = CEGUI::AnimationManager::getSingleton().instantiateAnimation(animdefinition);

		// Apply the instance //
		createdanim->setTargetWindow(targetwind);

		createdanim->start();
	}

	// Recurse to child elements if desired //
	if(recurse){

		// Find all child windows and call this method on them //
		for(size_t i = 0; i < targetwind->getChildCount(); i++){
			auto newtarget = targetwind->getChildAtIdx(i);

			_PlayAnimationOnWindow(newtarget, animdefinition, recurse, ignoretypenames);
		}
	}
}
// ------------------------------------ //
DLLEXPORT unique_ptr<GuiCollectionStates> Leviathan::Gui::GuiManager::GetGuiStates() const{
	GUARD_LOCK_THIS_OBJECT();
	// Create the result object using the size of Collections as all of them are added there //
	unique_ptr<GuiCollectionStates> result(new GuiCollectionStates(Collections.size()));

	// Add all the states and names of the collections //
	for(size_t i = 0; i < Collections.size(); i++){

		result->AddNewEntry(Collections[i]->GetName(), Collections[i]->GetState());
	}

	return result;
}

DLLEXPORT void Leviathan::Gui::GuiManager::ApplyGuiStates(const GuiCollectionStates* states){
	GUARD_LOCK_THIS_OBJECT();
	// Apply all the states from the object //
	for(size_t i = 0; i < states->CollectionNames.size(); i++){

		auto foundcollect = GetCollection(-1, *states->CollectionNames[i]->Name);

		// If found check whether the states match if not change to the right state //
		if(foundcollect && foundcollect->GetState() != states->CollectionNames[i]->IsEnabled){

			// Change the state //
			foundcollect->UpdateState(states->CollectionNames[i]->IsEnabled);
		}
	}
}

// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::InjectPasteRequest(){
	return GuiContext->injectPasteRequest();
}

DLLEXPORT bool Leviathan::Gui::GuiManager::InjectCopyRequest(){
	return GuiContext->injectCopyRequest();
}

DLLEXPORT bool Leviathan::Gui::GuiManager::InjectCutRequest(){
	return GuiContext->injectCutRequest();
}
