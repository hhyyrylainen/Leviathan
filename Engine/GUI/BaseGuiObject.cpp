#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASEOBJECT
#include "BaseGuiObject.h"
#endif
#include "Iterators/StringIterator.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Script/ScriptModule.h"
#include <boost/assign/list_of.hpp>
#include "GUI/GuiManager.h"
#include "Common/StringOperations.h"
#include "Script/ScriptRunningSetup.h"
#include "Script/ScriptExecutor.h"
#include "CEGUI/widgets/PushButton.h"
#include "CEGUI/widgets/FrameWindow.h"
#include "CEGUI/widgets/Combobox.h"
#include "../Handlers/IDFactory.h"
using namespace Leviathan;
using namespace Gui;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::BaseGuiObject::BaseGuiObject(GuiManager* owner, const std::string &name,
    int fakeid, std::shared_ptr<ScriptScript> script /*= NULL*/) :
    EventableScriptObject(script), OwningInstance(owner), FileID(fakeid), Name(name),
    ID(IDFactory::GetID()), TargetElement(NULL)
{
	
}

DLLEXPORT Leviathan::Gui::BaseGuiObject::~BaseGuiObject(){


}

DLLEXPORT void Leviathan::Gui::BaseGuiObject::ReleaseData(){

	// Unregister events to avoid access violations //
	_UnsubscribeAllEvents();

	// Unregister all non-CEGUI events //
	UnRegisterAllEvents();

	GUARD_LOCK();

	_LeaveBondBridge();

	// Make sure that the module doesn't use us //
	if(Scripting){

		auto module = Scripting->GetModuleSafe();

		if(module){

			module->SetAsInvalid();

			auto directptr = module.get();
			Scripting.reset();
			module.reset();

			// The module should be destroyed sometime //
			ScriptExecutor::Get()->DeleteModule(directptr);
		}
	}
}
// ------------------------------------ //
DLLEXPORT string Leviathan::Gui::BaseGuiObject::GetName(){

	return Name;
}

DLLEXPORT CEGUI::Window* Leviathan::Gui::BaseGuiObject::GetTargetWindow() const{
	return TargetElement;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::BaseGuiObject::LoadFromFileStructure(Lock &ownerlock,
    GuiManager* owner, vector<BaseGuiObject*> &tempobjects, ObjectFileObject& dataforthis)
{
	// parse fake id from prefixes //
	int fakeid = 0;

	for(size_t i = 0; i < dataforthis.GetPrefixesCount(); i++){

		auto str = dataforthis.GetPrefix(i);

		if(StringOperations::StringStartsWith(str, std::string("ID"))){
			// get id number //
			StringIterator itr(str);

			auto tempnumber = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

			fakeid = Convert::StringTo<int>(*tempnumber);

			// Nothing more to find //
			break;
		}
	}

	unique_ptr<BaseGuiObject> tmpptr(new BaseGuiObject(owner, dataforthis.GetName(), fakeid, dataforthis.GetScript()));

	shared_ptr<NamedVariableList> listenon;

	// Get listeners //
	auto paramlist = dataforthis.GetListWithName("params");

	if(paramlist){

		listenon = paramlist->GetVariables().GetValueDirect("ListenOn");
	}

	if(!tmpptr){

		return false;
	}
		
	// Listening start //
	if(listenon){
		tmpptr->StartMonitoring(listenon->GetValues());
	}

	// Find the CEGUI object //
	CEGUI::Window* foundobject = NULL;
	try{
		// Names starting with '_' are not considered to be targeting specific CEGUI windows //
		if(tmpptr->Name.find_first_of(L'_') != 0)
			foundobject = owner->GetMainContext(ownerlock)
                ->getRootWindow()->getChild(tmpptr->Name);

	} catch(const CEGUI::UnknownObjectException &e){

		// Not found //
		Logger::Get()->Error("BaseGuiObject: couldn't find a CEGUI window with name: "+
            tmpptr->Name+":");
        
		Logger::Get()->Write("\t> "+string(e.what()));
	}

	if(foundobject){
		// Set the object //
		tmpptr->ConnectElement(foundobject);
	}

	tmpptr->_HookListeners();

	// Call the Init function //
	Event initevent(EVENT_TYPE_INIT, NULL);
	Event* eptr = &initevent;

    {
        ownerlock.unlock();
        
        tmpptr->OnEvent(&eptr);

        ownerlock.lock();
    }

	tempobjects.push_back(tmpptr.release());
	return true;
}
// ------------------------------------ //
void Leviathan::Gui::BaseGuiObject::_HookListeners(){
	// first we need to get probably right listeners and register with them //
	if(!Scripting)
		return;
	ScriptModule* mod = Scripting->GetModule();

	std::vector<shared_ptr<ValidListenerData>> containedlisteners;

	mod->GetListOfListeners(containedlisteners);

	// Allow the module to hot-reload our files //
	_BondWithModule(mod);


	for(size_t i = 0; i < containedlisteners.size(); i++){
		// Generics cannot be CEGUI events //
		if(containedlisteners[i]->GenericTypeName && containedlisteners[i]->GenericTypeName->size() > 0){
			// custom event listener //

			RegisterForEvent(*containedlisteners[i]->GenericTypeName);

			continue;
		}

		// Check is this a CEGUI event which will be registered //
		if(_HookCEGUIEvent(*containedlisteners[i]->ListenerName))
			continue;


		// look for global events //
		EVENT_TYPE etype = ResolveStringToType(*containedlisteners[i]->ListenerName);
		if(etype != EVENT_TYPE_ERROR){

			RegisterForEvent(etype);
			continue;
		}

		Logger::Get()->Warning("BaseGuiObject: _HookListeners: unknown event type "+
            *containedlisteners[i]->ListenerName+", did you intent to use Generic type?");
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::BaseGuiObject::IsCEGUIEventHooked() const{
    
    GUARD_LOCK();
    return !CEGUIRegisteredEvents.empty();
}
// ------------------------------------ //
void Leviathan::Gui::BaseGuiObject::_CallScriptListener(Event** pEvent, GenericEvent** event2){
	if(!Scripting)
		return;
	ScriptModule* mod = Scripting->GetModule();

	if(pEvent){
		// Get the listener name from the event type //
		const std::string& listenername = GetListenerNameFromType((*pEvent)->GetType());

		// check does the script contain right listeners //
		if(mod->DoesListenersContainSpecificListener(listenername)){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(
                new NamedVariableBlock(
                    new VoidPtrBlock(this), "GuiObject")) (new NamedVariableBlock(
                            new VoidPtrBlock(*pEvent), "Event"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*pEvent)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername)).SetArguments(Args);

			// Run the script //
			shared_ptr<VariableBlock> result = ScriptExecutor::Get()->RunSetUp(
                Scripting.get(), &sargs);
		}
	} else {
		// generic event is passed //
		if(mod->DoesListenersContainSpecificListener("", (*event2)->GetTypePtr())){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(
                new NamedVariableBlock(new VoidPtrBlock(this), "GuiObject")) (
                    new NamedVariableBlock(new VoidPtrBlock(*event2), "GenericEvent"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*event2)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName("",
                    (*event2)->GetTypePtr())).SetArguments(Args);

			// Run the script //
			shared_ptr<VariableBlock> result = ScriptExecutor::Get()->RunSetUp(
                Scripting.get(), &sargs);
		}
	}
}
// ------------------------------------ //
DLLEXPORT std::unique_ptr<ScriptRunningSetup> Leviathan::Gui::BaseGuiObject::GetParametersForInit(){
	return _GetArgsForAutoFunc();
}

DLLEXPORT std::unique_ptr<ScriptRunningSetup> Leviathan::Gui::BaseGuiObject::GetParametersForRelease(){
	return _GetArgsForAutoFunc();
}

unique_ptr<ScriptRunningSetup> Leviathan::Gui::BaseGuiObject::_GetArgsForAutoFunc(){
	// Setup the parameters //
	vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(
            new VoidPtrBlock(this), "GuiObject"))
        (new NamedVariableBlock(new VoidPtrBlock((Event*)nullptr), "Event"));

	// we are returning ourselves so increase refcount
	AddRef();

	unique_ptr<ScriptRunningSetup> sargs(new ScriptRunningSetup);
	sargs->SetArguments(Args);

	return sargs;
}
// ------------------------------------ //
std::map<std::string, const CEGUI::String*> Leviathan::Gui::BaseGuiObject::CEGUIEventNames;

void Leviathan::Gui::BaseGuiObject::ReleaseCEGUIEventNames(){

	Lock lockthis(CEGUIEventMutex);

	CEGUIEventNames.clear();
}

void Leviathan::Gui::BaseGuiObject::MakeSureCEGUIEventsAreFine(Lock &locked){
	// Return if it already has data //
	if(!CEGUIEventNames.empty())
		return;


	// Fill the map //
	CEGUIEventNames.insert(make_pair(LISTENERNAME_ONCLICK, &CEGUI::PushButton::EventClicked));
	CEGUIEventNames.insert(make_pair(LISTENERNAME_ONCLOSECLICKED,
            &CEGUI::FrameWindow::EventCloseClicked));
    CEGUIEventNames.insert(make_pair(LISTENERNAME_LISTSELECTIONACCEPTED,
            &CEGUI::Combobox::EventListSelectionAccepted));
	

}

Mutex Leviathan::Gui::BaseGuiObject::CEGUIEventMutex;
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::BaseGuiObject::PrintWindowsRecursive(Lock &guard, CEGUI::Window* target /*= NULL*/,
    size_t level /*= 0*/) const
{

	VerifyLock(guard);

	// Print the heading if this is the first level //
	if(level == 0)
		Logger::Get()->Write("\n// ------------------ Layout of object \""+Name+
            "\" ------------------ //");

	CEGUI::Window* actualtarget = target != NULL ? target: TargetElement;

	if(!actualtarget)
		return;

	// Print this window //
	for(size_t i = 0; i < level; i++){

		Logger::Get()->DirectWriteBuffer("    ");
	}

	Logger::Get()->Write("    > "+string(actualtarget->getName().c_str()));
    
	// Recurse to child windows //
	for(size_t i = 0; i < actualtarget->getChildCount(); i++){
		auto newtarget = actualtarget->getChildAtIdx(i);

		PrintWindowsRecursive(guard, newtarget, level+1);
	}
}

DLLEXPORT void Leviathan::Gui::BaseGuiObject::PrintWindowsRecursiveProxy(){
	GUARD_LOCK();
	PrintWindowsRecursive(guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::BaseGuiObject::ConnectElement(CEGUI::Window* windojb){
	GUARD_LOCK();
	// Unconnect from previous ones //
	if(TargetElement){

		DEBUG_BREAK;
	}

	// Store the pointer //
	TargetElement = windojb;

	// Register for the destruction event //
	auto unhookevent = TargetElement->subscribeEvent(CEGUI::Window::EventDestructionStarted, 
		CEGUI::Event::Subscriber(&BaseGuiObject::EventDestroyWindow, this));

	// Apparently this also has to be disconnected //
	CEGUIRegisteredEvents.push_back(unhookevent);
}

bool Leviathan::Gui::BaseGuiObject::_HookCEGUIEvent(const std::string &listenername){

    {
        // This should be fine to be only locked during the next call as
        // it is the only place where the map could be updated
        Lock lockthis(CEGUIEventMutex);

        MakeSureCEGUIEventsAreFine(lockthis);
    }

	// Try to match the name //

	auto iter = CEGUIEventNames.find(listenername);

	if(iter == CEGUIEventNames.end())
		return false;

	// It is a valid event name //
	GUARD_LOCK();


	// Return if we have no element //
	if(!TargetElement){

		// It is a CEGUI event so return true even though it isn't handled //
		return true;
	}

	// Switch on special cases where special handling can be used //
	CEGUI::Event::Connection createdconnection;

	
	if(iter->second == &CEGUI::PushButton::EventClicked){
		createdconnection = TargetElement->subscribeEvent(*iter->second, CEGUI::Event::Subscriber(
                &BaseGuiObject::EventOnClick, this));

	} else if(iter->second == &CEGUI::FrameWindow::EventCloseClicked){

		createdconnection = TargetElement->subscribeEvent(*iter->second, CEGUI::Event::Subscriber(
                &BaseGuiObject::EventOnCloseClicked, this));

	} else if(iter->second == &CEGUI::Combobox::EventListSelectionAccepted){
        
        createdconnection = TargetElement->subscribeEvent(*iter->second,
            CEGUI::Event::Subscriber(&BaseGuiObject::EventOnListSelectionAccepted, this));
        
    } else {

		// Generic listeners aren't supported since we would have no way of knowing which script listener
        // would have to be called
		Logger::Get()->Error("BaseGuiObject: _HookCEGUIEvent: event is missing a specific clause, name: "+
            listenername);
        
		Logger::Get()->Save();
		assert(0 && "Unsupported CEGUI listener type is being called");
		return true;
	}

	CEGUIRegisteredEvents.push_back(createdconnection);

	return true;
}

void Leviathan::Gui::BaseGuiObject::_UnsubscribeAllEvents(){
	GUARD_LOCK();
	// Loop an disconnect them all //
	for(auto iter = CEGUIRegisteredEvents.begin(); iter != CEGUIRegisteredEvents.end(); ++iter){

		(*iter)->disconnect();
	}

	CEGUIRegisteredEvents.clear();
}
// ------------------------------------ //
bool Leviathan::Gui::BaseGuiObject::_CallCEGUIListener(const std::string &name,
    std::function<void(vector<shared_ptr<NamedVariableBlock>>&)> extraparam /*= NULL*/)
{
	// There should be one as it is registered //
	ScriptModule* mod = Scripting->GetModule();

	if(!mod){
        Logger::Get()->Error("BaseGuiObject: CallCEGUIListener: the script module is no longer "
            "valid");
		return false;
    }

	// Setup the parameters //
	vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(
        new NamedVariableBlock(new VoidPtrBlock(this), "GuiObject"));

	// We are returning ourselves so increase refcount
	AddRef();

    // A calling function might want to add extra parameters //
    if(extraparam)
        extraparam(Args);

	ScriptRunningSetup sargs;
	sargs.SetEntrypoint(mod->GetListeningFunctionName(name)).SetArguments(Args);


	// Run the script //
	shared_ptr<VariableBlock> result = ScriptExecutor::Get()->RunSetUp(Scripting.get(), &sargs);

	if(!result || !result->IsConversionAllowedNonPtr<bool>()){

		return false;
	}

	// Return the value returned by the script //
	return result->ConvertAndReturnVariable<bool>();
}
// ------------------------------------ //
bool Leviathan::Gui::BaseGuiObject::EventDestroyWindow(const CEGUI::EventArgs &args){
	GUARD_LOCK();

	// This should be safe //
	auto res = static_cast<const CEGUI::WindowEventArgs&>(args);

	assert(res.window == TargetElement &&
        "BaseGuiObject received destruction notification for unsubscribed window");



	// This might be required to not leak memory //
	_UnsubscribeAllEvents();

	TargetElement = NULL;

	return false;
}

bool Leviathan::Gui::BaseGuiObject::EventOnClick(const CEGUI::EventArgs &args){
	// Pass the click event to the script //

	return _CallCEGUIListener(LISTENERNAME_ONCLICK);
}

bool Leviathan::Gui::BaseGuiObject::EventOnCloseClicked(const CEGUI::EventArgs &args){
	// Pass the event to the script //

	return _CallCEGUIListener(LISTENERNAME_ONCLOSECLICKED);
}

bool Leviathan::Gui::BaseGuiObject::EventOnListSelectionAccepted(const CEGUI::EventArgs &args){

    auto res = static_cast<const CEGUI::WindowEventArgs&>(args);

    auto targetwind = dynamic_cast<CEGUI::Combobox*>(res.window);

    if(!targetwind){

        Logger::Get()->Error("BaseGuiObject: CEGUI listener for ListSelectionAccepted didn't get "
            "a Combobox as argument");
        return false;
    }

    auto item = targetwind->getSelectedItem();

    if(!item){
        
        return false;
    }

    std::string selectedtext(item->getText().c_str());
    
    return _CallCEGUIListener(LISTENERNAME_LISTSELECTIONACCEPTED, std::bind<void>([](
                const string &selecttext, vector<shared_ptr<NamedVariableBlock>> &paramlist)
            -> void
        {

            paramlist.push_back(make_shared<NamedVariableBlock>(new StringBlock(selecttext),
                    "Text"));

        }, selectedtext, std::placeholders::_1));
}
