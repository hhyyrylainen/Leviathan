// ------------------------------------ //
#include "GuiManager.h"
// ------------------------------------ //
#include "../CEGUIInclude.h"
#include "Common/DataStoring/DataBlock.h"
#include "Common/DataStoring/DataStore.h"
#include "../TimeIncludes.h"
#include "Engine.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "GuiCollection.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreManualObject.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "Rendering/GUI/FontManager.h"
#include "Rendering/GraphicalInputEntity.h"
#include "Rendering/Graphics.h"
#include "Script/ScriptExecutor.h"
#include "Window.h"
#include <thread>

#include <SDL.h>
// ------------------------------------ //

// ------------------ GuiClipboardHandler ------------------ //
class Leviathan::Gui::GuiClipboardHandler :
    public CEGUI::NativeClipboardProvider, public ThreadSafe{
public:

	void sendToClipboard(const CEGUI::String& mimeType, void* buffer, size_t size) override{
        
        GUARD_LOCK();
        
        // Ignore non-text setting //
		if(mimeType != "text/plain"){

			return;
		}

        // Let's hope it is null terminated
        if(SDL_SetClipboardText(reinterpret_cast<const char*>(buffer)) < 0){

            LOG_ERROR("Copy to clipboard failed: " + std::string(SDL_GetError()));
        }
	}

	void retrieveFromClipboard(CEGUI::String& mimeType, void*& buffer, size_t& size) override{

        GUARD_LOCK();

        if(SDL_HasClipboardText() == SDL_FALSE){

        nothinginclipboardlabel:
            buffer = nullptr;
            size = 0;
            mimeType = "";
            return;
        }

        auto* text = SDL_GetClipboardText();

        if(!text){

            goto nothinginclipboardlabel;
        }

        ReceivedClipboardData = std::string(text);

        SDL_free(text);
        text = nullptr;

        
        mimeType = "text/plain";        
        size = ReceivedClipboardData.size();

        // Set the CEGUI data pointer to our string
        buffer = const_cast<char*>(ReceivedClipboardData.c_str());
	}
    
private:
    
    std::string ReceivedClipboardData;
};


using namespace Leviathan;
using namespace Gui;
using namespace std;
// ------------------ GuiManager ------------------ //
Leviathan::Gui::GuiManager::GuiManager() :
    ID(IDFactory::GetID())
{
	
}
Leviathan::Gui::GuiManager::~GuiManager(){

	
}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::Init(Graphics* graph, GraphicalInputEntity* window,
    bool ismain)
{
	GUARD_LOCK();

	ThisWindow = window;
    MainGuiManager = ismain;
	
	// Setup this window's context //
	GuiContext = &CEGUI::System::getSingleton().createGUIContext(
        ThisWindow->GetCEGUIRenderer()->getDefaultRenderTarget());

	// Setup input for the context //
	ContextInput = new CEGUI::InputAggregator(GuiContext);
	ContextInput->initialise(false);

    
   

	// Make the clipboard play nice //
    // Only one clipboard is needed //
	if(MainGuiManager == 1){

        _GuiClipboardHandler = std::make_unique<GuiClipboardHandler>();
        
        CEGUI::System::getSingleton().getClipboard()->setNativeProvider(
            _GuiClipboardHandler.get());
	}

	// Store the initial time //
	LastTimePulseTime = Time::GetThreadSafeSteadyTimePoint();

	return true;
}

void Leviathan::Gui::GuiManager::Release(){
	GUARD_LOCK();
	

	// Stop with the file updates //
	if(FileChangeID){


		auto tmphandler = ResourceRefreshHandler::Get();

		if(tmphandler){

			tmphandler->StopListeningForFileChanges(FileChangeID);
		}
        
		FileChangeID = 0;
	}

	// Default mouse back //
	SetMouseTheme(guard, "none");

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

	ContextInput->removeAllEvents();
	SAFE_DELETE(ContextInput);

	// Destroy the GUI //
	CEGUI::System::getSingleton().destroyGUIContext(*GuiContext);
	GuiContext = NULL;

    // If we are the main window unhook the clipboard //
    if(_GuiClipboardHandler && MainGuiManager){

        CEGUI::System::getSingleton().getClipboard()->setNativeProvider(NULL);
        Logger::Get()->Info("GuiManager: main manager has detached the "
            "clipboard successfully");
    }
    
    Logger::Get()->Info("GuiManager: Gui successfully closed on window");
}

DLLEXPORT void GuiManager::EnableStandardGUIThemes(){

    if(MainGuiManager){
        // Load the taharez look //
        LoadGUITheme("TaharezLook.scheme");
    }

    GUARD_LOCK();

    // Set Simonetta as the default font //
    GuiContext->setDefaultFont("Simonetta-Regular");


	// Set the taharez looks active //
	SetMouseTheme(guard, "TaharezLook/MouseArrow");
	GuiContext->setDefaultTooltipType("TaharezLook/Tooltip");
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::ProcessKeyDown(int32_t key, int specialmodifiers){
    
	GUARD_LOCK();
    
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetTogglingKey().Match(key, specialmodifiers, false) &&
            Collections[i]->GetAllowEnable())
        {
			// Is a match, toggle //
			Collections[i]->ToggleState(guard);

			return true;
		}
	}

	return false;
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionState(const string &name, bool state){
	GUARD_LOCK();
	// find collection with name and set it's state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetState() != state){
                
				Collections[i]->ToggleState(guard);
			}
			return;
		}
	}
	// Complain //
	Logger::Get()->Warning("GuiManager: SetCollectionState: couldn't find a collection "
        "with name: " + name);
}

DLLEXPORT void Leviathan::Gui::GuiManager::SetCollectionAllowEnableState(const string &name,
    bool allow /*= true*/)
{
	GUARD_LOCK();
	// find collection with name and set it's allow enable state //
	for(size_t i = 0; i < Collections.size(); i++){
		if(Collections[i]->GetName() == name){
			// set state //
			if(Collections[i]->GetAllowEnable() != allow){
				Logger::Get()->Info("Setting Collection "+Collections[i]->GetName()+
                    " allow enable state "+Convert::ToString(allow));
				Collections[i]->ToggleAllowEnable();
			}
			return;
		}
	}
}
// ------------------------------------ //
void Leviathan::Gui::GuiManager::GuiTick(int mspassed){
	GUARD_LOCK();

    if(ReloadQueued){

        ReloadQueued = false;

        // Reload //
        LOG_INFO("GuiManager: reloading file: " + MainGUIFile);
        
        // Store the current state //
        auto currentstate = GetGuiStates(guard);

        UnLoadGUIFile(guard);

        // Now load it //
        if(!LoadGUIFile(guard, MainGUIFile, true)){

            Logger::Get()->Error("GuiManager: file changed: couldn't load updated file: "+
                MainGUIFile);
        }

        // Apply back the old states //
        ApplyGuiStates(guard, currentstate.get());
    }

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

                // Prevent capturing the mouse if disabled //
                if(DisableGuiMouseCapture){

                    GuiDisallowMouseCapture = true;
                    
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
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnForceGUIOn(){
	DEBUG_BREAK;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::SetDisableMouseCapture(bool newvalue){

    DisableGuiMouseCapture = newvalue;
    // This will cause the capture state to be checked next tick
    GuiMouseUseUpdated = true;
}

// ------------------------------------ //
void Leviathan::Gui::GuiManager::Render(){
	GUARD_LOCK();

	// Pass time //
	auto newtime = Time::GetThreadSafeSteadyTimePoint();
	
	SecondDuration elapsed = newtime-LastTimePulseTime;

	float changval = elapsed.count();

	GuiContext->injectTimePulse(changval);

	// Potentially pass to system //
	if(MainGuiManager){

		CEGUI::System::getSingleton().injectTimePulse(changval);
	}

	LastTimePulseTime = newtime;

    guard.unlock();
    
	// Update inputs //


}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::GuiManager::OnResize(){
	GUARD_LOCK();

	// Notify the CEGUI system //
    // TODO: only to the wanted context
	CEGUI::System* const sys = CEGUI::System::getSingletonPtr();


    int32_t width, height;
    ThisWindow->GetWindow()->GetSize(width, height);
    
	if(sys)
		sys->notifyDisplaySizeChanged(CEGUI::Sizef((float)width,
                (float)height));
}

DLLEXPORT void Leviathan::Gui::GuiManager::OnFocusChanged(bool focused){
	GUARD_LOCK();
	
	// Notify our context //
	if(!focused)
		ContextInput->injectMouseLeaves();

}
// ------------------------------------ //
bool Leviathan::Gui::GuiManager::AddGuiObject(Lock &guard, BaseGuiObject* obj){
	Objects.push_back(obj);
	return true;
}

void Leviathan::Gui::GuiManager::DeleteObject(int id){
	GUARD_LOCK();
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
	GUARD_LOCK();
	for(size_t i = 0; i < Objects.size(); i++){
		if(Objects[i]->GetID() == id)
			return static_cast<int>(i);
	}
	return -1;
}

BaseGuiObject* Leviathan::Gui::GuiManager::GetObject(unsigned int index){
	GUARD_LOCK();
	if(index < Objects.size()){
        
		return Objects[index];
	}
	return NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::LoadGUIFile(Lock &guard, const string &file,
    bool nochangelistener, int iteration /*= 0*/)
{
    if(iteration > 10){

        Logger::Get()->Warning("GuiManager: aborting file load on iteration "+
            Convert::ToString(iteration));
        return false;
    }
    
	// Parse the file //
	auto data = ObjectFileProcessor::ProcessObjectFile(file, Logger::Get());

	if(!data){
		return false;
	}

	MainGUIFile = file;

	NamedVars& varlist = *data->GetVariables();

	string relativepath;
    
	// Get path //
	ObjectFileProcessor::LoadValueFromNamedVars<string>(
        varlist, "GUIBaseFile", relativepath, "", Logger::Get(),
        "GuiManager: LoadGUIFile: no base file defined (in "+file+") : ");

    // This can be used to verify that CEGUI events are properly hooked //
    bool requireevent;

    ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        varlist, "RequireCEGUIHooked", requireevent, false);

	if(!relativepath.size()){

		return false;
	}
	
	// Load it //
	CEGUI::Window* rootwindow = NULL;
	try{

		rootwindow = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(relativepath);

	} catch(const Ogre::Exception &e){

		Logger::Get()->Error("GuiManager: LoadGUIFile: failed to locate file: "+relativepath+":");
		Logger::Get()->Write(string("\t> ")+e.what());
		return false;
	} catch(const CEGUI::GenericException &e2){

		Logger::Get()->Error("GuiManager: LoadGUIFile: failed to parse CEGUI layout: "+
            relativepath+":");
		Logger::Get()->Write(string("\t> ")+e2.what());
		return false;
	} catch(const CEGUI::InvalidRequestException &e3){

		Logger::Get()->Error("GuiManager: LoadGUIFile: failed to parse CEGUI layout: "+
            relativepath+":");
		Logger::Get()->Write(string("\t> ")+e3.what());
		return false;
	}

	// Check did it work //
	if(!rootwindow){

		Logger::Get()->Error("GuiManager: LoadGUIFile: failed to parse layout file: "+
            relativepath);
		return false;
	}

	// Look for animation files //
	auto animslist = varlist.GetValueDirectRaw("GUIAnimations");

	if(animslist){

		if(!animslist->CanAllBeCastedToType<string>()){

			Logger::Get()->Warning("GuiManager: LoadGUIFile: gui file has defined gui animation "
                "files in wrong format (expected a list of strings), file: "+relativepath);

		} else {

			// Load them //
			for(size_t i = 0; i < animslist->GetVariableCount(); i++){

				string curfile;
				animslist->GetValue(i).ConvertAndAssingToVariable<string>(curfile);

				// Check is the file already loaded //
				{
					Lock gguard(GlobalGUIMutex);

					if(IsAnimationFileLoaded(gguard, curfile)){

						// Don't load again //
						continue;
					}

					// Set as loaded //
					SetAnimationFileLoaded(gguard, curfile);
				}

				try{

					CEGUI::AnimationManager::getSingleton().loadAnimationsFromXML(curfile);

				} catch(const Ogre::Exception &e){

					Logger::Get()->Warning("GuiManager: LoadGUIFile: failed to locate gui "
                        " animation file: "+curfile+":");
					Logger::Get()->Write(string("\t> ")+e.what());

				} catch(const CEGUI::GenericException &e2){

					Logger::Get()->Error("GuiManager: LoadGUIFile: failed to parse CEGUI "
                        "animation file layout: "+curfile+":");
					Logger::Get()->Write(string("\t> ")+e2.what());
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
		if(objecto->GetTypeName() == "GuiCollection"){

			if(!GuiCollection::LoadCollection(guard, this, *objecto)){

				// report error //
				Logger::Get()->Error("GuiManager: ExecuteGuiScript: failed to load collection, "
                    "named "+objecto->GetName());
				continue;
			}

			continue;

		} else if(objecto->GetTypeName() == "GuiObject"){

			// try to load //
			if(!BaseGuiObject::LoadFromFileStructure(guard, this, TempOs, *objecto)){

				// report error //
				Logger::Get()->Error("GuiManager: ExecuteGuiScript: failed to load GuiObject, "
                    "named "+objecto->GetName());
                
				continue;
			}

			continue;
		}

		Logger::Get()->Error("GuiManager: ExecuteGuiScript: Unrecognized type! typename: "+
            objecto->GetTypeName());
	}

    // Verify loaded hooks, if wanted //
    if(requireevent){

        bool foundsomething = false;

        auto end = TempOs.end();
        for(auto iter = TempOs.begin(); iter != end; ++iter){

            if((*iter)->IsCEGUIEventHooked()){

                foundsomething = true;
                break;
            }
        }

        if(!foundsomething){

            // Report that we will attempt to reload the file //
            Logger::Get()->Warning("GuiManager: while trying to load \""+file+
                "\" RequireCEGUIHooked is true, but no GUI object has any hooked CEGUI "
                "listeners, retrying load: ");

            UnLoadGUIFile();

            for(size_t i = 0; i < TempOs.size(); i++){
		
                TempOs[i]->ReleaseData();
                SAFE_RELEASE(TempOs[i]);
            }

            Logger::Get()->Write("\t> Doing iteration "+Convert::ToString(iteration+1));

            return LoadGUIFile(guard, file, nochangelistener, iteration+1);
        }
    }
	

	for(size_t i = 0; i < TempOs.size(); i++){

		// add to real objects //
		AddGuiObject(guard, TempOs[i]);
	}

	// This avoids having more and more change listeners each reload //
	if(!nochangelistener){
		// Listen for file changes //
		auto tmphandler = ResourceRefreshHandler::Get();
	
		if(tmphandler){

			// \todo Detect if the files are in different folders and start multiple listeners
			std::vector<const string*> targetfiles =
                boost::assign::list_of(&file)(&relativepath);

			tmphandler->ListenForFileChanges(targetfiles, std::bind(&GuiManager::_FileChanged,
                    this, placeholders::_1, placeholders::_2),
                FileChangeID);
		}
	}

	return true;
}

DLLEXPORT void Leviathan::Gui::GuiManager::UnLoadGUIFile(Lock &guard){

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
DLLEXPORT void Leviathan::Gui::GuiManager::SetMouseTheme(Lock &guard, const string &tname){

	if(tname == "none"){

		// show default window cursor //
		ThisWindow->GetWindow()->SetHideCursor(false);
		return;
	}

	// Set it active //
	GuiContext->getCursor().setDefaultImage(tname);

	// hide window cursor //
	ThisWindow->GetWindow()->SetHideCursor(true);
}
// ------------------------------------ //
DLLEXPORT CEGUI::GUIContext* Leviathan::Gui::GuiManager::GetMainContext(Lock &guard){
	return GuiContext;
}
// ----------------- collection managing --------------------- //
void GuiManager::AddCollection(Lock &guard, GuiCollection* add){
    
	Collections.push_back(add);
}

GuiCollection* Leviathan::Gui::GuiManager::GetCollection(const int &id, const string &name){
	GUARD_LOCK();
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
// ------------------------------------ //
void Leviathan::Gui::GuiManager::_FileChanged(const string &file,
    ResourceFolderListener &caller)
{
	// Any updated file will cause whole reload //
	GUARD_LOCK();

    ReloadQueued = true;

	// Mark everything as non-updated //
	caller.MarkAllAsNotUpdated();
}
// ------------------------------------ //
DLLEXPORT CEGUI::Window* Leviathan::Gui::GuiManager::GetWindowByStringName(Lock &guard,
    const string &namepath)
{
	try{

		return GuiContext->getRootWindow()->getChild(namepath);

	} catch(const CEGUI::UnknownObjectException&){

		// Not found //
		return NULL;
	}
}

DLLEXPORT bool Leviathan::Gui::GuiManager::PlayAnimationOnWindow(Lock &guard,
    const string &windowname, const string &animationname, bool applyrecursively,
    const string &ignoretypenames)
{
	// First get the window //
	auto wind = GetWindowByStringName(guard, windowname);

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


	_PlayAnimationOnWindow(guard, wind, animdefinition, applyrecursively, ignoretypenames);
	return true;
}

DLLEXPORT bool Leviathan::Gui::GuiManager::PlayAnimationOnWindowProxy(const string &windowname,
    const string &animationname)
{
    GUARD_LOCK();
	return PlayAnimationOnWindow(guard, windowname, animationname);
}

void Leviathan::Gui::GuiManager::_PlayAnimationOnWindow(Lock &guard,
    CEGUI::Window* targetwind, CEGUI::Animation* animdefinition, bool recurse,
    const string &ignoretypenames)
{
	// Apply only if the typename doesn't match ignored names //
	if(ignoretypenames.find(targetwind->getType().c_str()) == string::npos &&
        targetwind->getName().at(0) != '_')
    {

		// Create an animation instance //
		CEGUI::AnimationInstance* createdanim =
            CEGUI::AnimationManager::getSingleton().instantiateAnimation(animdefinition);

		// Apply the instance //
		createdanim->setTargetWindow(targetwind);

		createdanim->start();
	}

	// Recurse to child elements if desired //
	if(recurse){

		// Find all child windows and call this method on them //
		for(size_t i = 0; i < targetwind->getChildCount(); i++){
			auto newtarget = targetwind->getChildAtIdx(i);

			_PlayAnimationOnWindow(guard, newtarget, animdefinition, recurse, ignoretypenames);
		}
	}
}
// ------------------------------------ //
DLLEXPORT std::unique_ptr<GuiCollectionStates> Leviathan::Gui::GuiManager::GetGuiStates(
    Lock &guard) const
{
    
	// Create the result object using the size of Collections as all of them are added there //
	unique_ptr<GuiCollectionStates> result(new GuiCollectionStates(Collections.size()));

	// Add all the states and names of the collections //
	for(size_t i = 0; i < Collections.size(); i++){

		result->AddNewEntry(Collections[i]->GetName(), Collections[i]->GetState());
	}

	return result;
}

DLLEXPORT void Leviathan::Gui::GuiManager::ApplyGuiStates(Lock &guard,
    const GuiCollectionStates* states)
{

	// Apply all the states from the object //
	for(size_t i = 0; i < states->CollectionNames.size(); i++){

		auto foundcollect = GetCollection(-1, *states->CollectionNames[i]->Name);

		// If found check whether the states match if not change to the right state //
		if(foundcollect && foundcollect->GetState() != states->CollectionNames[i]->IsEnabled){

			// Change the state //
			foundcollect->UpdateState(guard, states->CollectionNames[i]->IsEnabled);
		}
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::GuiManager::InjectPasteRequest(){
	return ContextInput->injectPasteRequest();
}

DLLEXPORT bool Leviathan::Gui::GuiManager::InjectCopyRequest(){
	return ContextInput->injectCopyRequest();
}

DLLEXPORT bool Leviathan::Gui::GuiManager::InjectCutRequest(){
	return ContextInput->injectCutRequest();
}
// ------------------ Static part ------------------ //
std::vector<string> Leviathan::Gui::GuiManager::LoadedAnimationFiles;

Mutex Leviathan::Gui::GuiManager::GlobalGUIMutex;

bool Leviathan::Gui::GuiManager::IsAnimationFileLoaded(Lock &lock, const string &file){
    
	for(size_t i = 0; i < LoadedAnimationFiles.size(); i++){

		if(LoadedAnimationFiles[i] == file){

			return true;
		}
	}

	// Not found, must not be loaded then //
	return false;
}

void Leviathan::Gui::GuiManager::SetAnimationFileLoaded(Lock &lock, const string &file){

	LoadedAnimationFiles.push_back(file);
}

DLLEXPORT void Leviathan::Gui::GuiManager::KillGlobalCache(){
	Lock guard(GlobalGUIMutex);

	// Release the memory to not look like a leak //
	LoadedAnimationFiles.clear();

	auto single = CEGUI::AnimationManager::getSingletonPtr();

	if(single)
		single->destroyAllAnimations();
}

DLLEXPORT void GuiManager::LoadGUITheme(const std::string &filename){

    LOG_INFO("GuiManager: loading gui theme: " + filename);
    CEGUI::SchemeManager::getSingleton().createFromFile(filename);
}
// ------------------------------------ //
