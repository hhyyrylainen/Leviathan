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
#include "Script/ScriptInterface.h"
#include "CEGUI/widgets/PushButton.h"
#include "CEGUI/widgets/FrameWindow.h"
using namespace Leviathan;
using namespace Gui;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::BaseGuiObject::BaseGuiObject(GuiManager* owner, const wstring &name, int fakeid, shared_ptr<ScriptScript> script 
	/*= NULL*/) : EventableScriptObject(script), OwningInstance(owner), FileID(fakeid), Name(name), 
	ID(IDFactory::GetID()), TargetElement(NULL)
{
	
}

DLLEXPORT Leviathan::Gui::BaseGuiObject::~BaseGuiObject(){
	// Unregister events to avoid access violations //
	_UnsubscribeAllEvents();
	
	// Unregister all non-CEGUI events //
	UnRegisterAllEvents();
}
// ------------------------------------ //
DLLEXPORT string Leviathan::Gui::BaseGuiObject::GetNameAsString(){
	return Convert::WstringToString(Name);
}

DLLEXPORT CEGUI::Window* Leviathan::Gui::BaseGuiObject::GetTargetWindow() const{
	return TargetElement;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::BaseGuiObject::LoadFromFileStructure(GuiManager* owner, vector<BaseGuiObject*> &tempobjects, 
	ObjectFileObject& dataforthis)
{
	// parse fake id from prefixes //
	int fakeid = 0;

	for(size_t i = 0; i < dataforthis.GetPrefixesCount(); i++){

		auto str = dataforthis.GetPrefix(i);

		if(StringOperations::StringStartsWith(str, wstring(L"ID"))){
			// get id number //
			StringIterator itr(str);

			auto tempnumber = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_NONE);

			fakeid = Convert::WstringToInt(*tempnumber);

			// Nothing more to find //
			break;
		}
	}

	unique_ptr<BaseGuiObject> tmpptr(new BaseGuiObject(owner, dataforthis.GetName(), fakeid, dataforthis.GetScript()));

	shared_ptr<NamedVariableList> listenon;

	// Get listeners //
	auto paramlist = dataforthis.GetListWithName(L"params");

	if(paramlist){

		listenon = paramlist->GetVariables().GetValueDirect(L"ListenOn");
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

		foundobject = owner->GetMainContext()->getRootWindow()->getChild(Convert::WstringToString(tmpptr->Name));

	} catch(const CEGUI::UnknownObjectException &e){

		// Not found //
		Logger::Get()->Error(L"BaseGuiObject: couldn't find a CEGUI window with name: "+tmpptr->Name+L":");
		Logger::Get()->Write(L"\t> "+Convert::CharPtrToWstring(e.what()));
	}

	if(foundobject){
		// Set the object //
		tmpptr->ConnectElement(foundobject);
	}

	tmpptr->_HookListeners();

	// Call the Init function //
	Event initevent(EVENT_TYPE_INIT, NULL);
	Event* eptr = &initevent;

	tmpptr->OnEvent(&eptr);


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


	for(size_t i = 0; i < containedlisteners.size(); i++){
		// generics cannot be rocket events //
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

		Logger::Get()->Warning(L"BaseGuiObject: _HookListeners: unknown event type "+*containedlisteners[i]->ListenerName+L", did you intent to use Generic type?");
	}
}
// ------------------------------------ //
void Leviathan::Gui::BaseGuiObject::_CallScriptListener(Event** pEvent, GenericEvent** event2){
	if(!Scripting)
		return;
	ScriptModule* mod = Scripting->GetModule();

	if(pEvent){
		// Get the listener name from the event type //
		wstring listenername = GetListenerNameFromType((*pEvent)->GetType());

		// check does the script contain right listeners //
		if(mod->DoesListenersContainSpecificListener(listenername)){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"GuiObject"))
				(new NamedVariableBlock(new VoidPtrBlock(*pEvent), L"Event"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*pEvent)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(listenername)).SetArguments(Args);

			// Run the script //
			shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);
		}
	} else {
		// generic event is passed //
		if(mod->DoesListenersContainSpecificListener(L"", (*event2)->GetTypePtr())){
			// setup parameters //
			vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"GuiObject"))
				(new NamedVariableBlock(new VoidPtrBlock(*event2), L"GenericEvent"));
			// we are returning ourselves so increase refcount
			AddRef();
			(*event2)->AddRef();

			ScriptRunningSetup sargs;
			sargs.SetEntrypoint(mod->GetListeningFunctionName(L"", (*event2)->GetTypePtr())).SetArguments(Args);

			// Run the script //
			shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);
		}
	}
}
// ------------------------------------ //
std::map<wstring, const CEGUI::String*> Leviathan::Gui::BaseGuiObject::CEGUIEventNames;

void Leviathan::Gui::BaseGuiObject::ReleaseCEGUIEventNames(){

	boost::strict_lock<boost::mutex> lockthis(CEGUIEventMutex);

	CEGUIEventNames.clear();
}

void Leviathan::Gui::BaseGuiObject::MakeSureCEGUIEventsAreFine(boost::strict_lock<boost::mutex> &locked){
	// Return if it already has data //
	if(!CEGUIEventNames.empty())
		return;


	// Fill the map //
	CEGUIEventNames = boost::assign::map_list_of
		(L"OnClick", &CEGUI::PushButton::EventClicked)
		(L"OnCloseClicked", &CEGUI::FrameWindow::EventCloseClicked);

}

boost::mutex Leviathan::Gui::BaseGuiObject::CEGUIEventMutex;
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::BaseGuiObject::ConnectElement(CEGUI::Window* windojb){
	GUARD_LOCK_THIS_OBJECT();
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

bool Leviathan::Gui::BaseGuiObject::_HookCEGUIEvent(const wstring &listenername){

	boost::strict_lock<boost::mutex> lockthis(CEGUIEventMutex);

	MakeSureCEGUIEventsAreFine(lockthis);

	// Try to match the name //

	auto iter = CEGUIEventNames.find(listenername);

	if(iter == CEGUIEventNames.end())
		return false;

	// It is a valid event name //
	GUARD_LOCK_THIS_OBJECT();


	// Return if we have no element //
	if(!TargetElement){

		// It is a CEGUI event so return true even though it isn't handled //
		return true;
	}

	// Switch on special cases where special handling can be used //
	CEGUI::Event::Connection createdconnection;

	
	if(iter->second == &CEGUI::PushButton::EventClicked){
		createdconnection = TargetElement->subscribeEvent(*iter->second, CEGUI::Event::Subscriber(&BaseGuiObject::EventOnClick, this));

	} else if(iter->second == &CEGUI::FrameWindow::EventCloseClicked){

		createdconnection = TargetElement->subscribeEvent(*iter->second, CEGUI::Event::Subscriber(&BaseGuiObject::EventOnCloseClicked, this));

	} else {

		// Generic listeners aren't supported since we would have no way of knowing which script listener would have to be called //
		Logger::Get()->Error(L"BaseGuiObject: _HookCEGUIEvent: event is missing a specific clause, name: "+listenername);
		Logger::Get()->Save();
		assert(0 && "Unsupported CEGUI listener type is being called");
		return true;
	}

	CEGUIRegisteredEvents.push_back(createdconnection);

	return true;
}

void Leviathan::Gui::BaseGuiObject::_UnsubscribeAllEvents(){
	GUARD_LOCK_THIS_OBJECT();
	// Loop an disconnect them all //
	for(auto iter = CEGUIRegisteredEvents.begin(); iter != CEGUIRegisteredEvents.end(); ++iter){

		(*iter)->disconnect();
	}

	CEGUIRegisteredEvents.clear();
}
// ------------------------------------ //
bool Leviathan::Gui::BaseGuiObject::_CallCEGUIListener(const wstring &name){
	// There should be one as it is registered //
	ScriptModule* mod = Scripting->GetModule();

	if(!mod)
		return false;

	// Setup the parameters //
	vector<shared_ptr<NamedVariableBlock>> Args = boost::assign::list_of(new NamedVariableBlock(new VoidPtrBlock(this), L"GuiObject"));

	// We are returning ourselves so increase refcount
	AddRef();

	ScriptRunningSetup sargs;
	sargs.SetEntrypoint(mod->GetListeningFunctionName(name)).SetArguments(Args);


	// Run the script //
	shared_ptr<VariableBlock> result = ScriptInterface::Get()->ExecuteScript(Scripting.get(), &sargs);

	if(!result || !result->IsConversionAllowedNonPtr<bool>()){

		return false;
	}

	// Return the value returned by the script //
	return result->ConvertAndReturnVariable<bool>();
}
// ------------------------------------ //
bool Leviathan::Gui::BaseGuiObject::EventDestroyWindow(const CEGUI::EventArgs &args){
	GUARD_LOCK_THIS_OBJECT();

	// This should be safe //
	auto res = static_cast<const CEGUI::WindowEventArgs&>(args);

	assert(res.window == TargetElement && "BaseGuiObject received destruction notification for unsubscribed window");



	// This might be required to not leak memory //
	_UnsubscribeAllEvents();

	TargetElement = NULL;

	return false;
}

bool Leviathan::Gui::BaseGuiObject::EventOnClick(const CEGUI::EventArgs &args){
	// Pass the click event to the script //

	return _CallCEGUIListener(L"OnClick");
}

bool Leviathan::Gui::BaseGuiObject::EventOnCloseClicked(const CEGUI::EventArgs &args){
	// Pass the event to the script //

	return _CallCEGUIListener(L"OnCloseClicked");
}
