#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_JSEVENTINTERFACE
#include "JSEventInterface.h"
#endif
#include "GuiCEFApplication.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
Leviathan::Gui::JSNativeCoreAPI::~JSNativeCoreAPI(){

}

Leviathan::Gui::JSNativeCoreAPI::JSNativeCoreAPI(CefApplication* owner) : Owner(owner){

}
// ------------------------------------ //
void Leviathan::Gui::JSNativeCoreAPI::ClearContextValues(){
	ObjectLock guard(*this);
	// Stop listening //
	Owner->StopListeningForEvents();
	// This should be enough //
	RegisteredListeners.clear();
}
// ------------------------------------ //
bool Leviathan::Gui::JSNativeCoreAPI::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, 
	CefRefPtr<CefV8Value>& retval, CefString& exception)
{
	// Check which function is called //
	if(name == "LOnEvent"){
		// Check are the arguments correct //
		if(arguments.size() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction()){
			// Invalid arguments //
			exception = "Invalid arguments passed, expected: string, function";
			return true;
		}

		// Get the name //
		const wstring eventname = arguments[0]->GetStringValue();

		// Convert it to an event enum value //
		auto iter = EventListenerNameToEventMap.find(eventname);

		// Check was it found //
		if(iter == EventListenerNameToEventMap.end()){
			exception = L"No event matched name: "+eventname;
			return true;
		}

		// Bind it //
		unique_ptr<JSListener> tmplistener(new JSListener((*iter), arguments[1], CefV8Context::GetCurrentContext()));

		ObjectLock guard(*this);

		// Add it //
		RegisteredListeners.push_back(tmplistener);

		// Notify our parent and make it work //
		Owner->StartListeningForEvent(tmplistener.get());

		return true;
	}

	// Not handled //
	return false;
}

void Leviathan::Gui::JSNativeCoreAPI::HandlePacket(const Event &eventdata){
	// Just pass it to all the listeners //
	ObjectLock guard(*this);

	for(size_t i = 0; i < RegisteredListeners.size(); i++){

		RegisteredListeners[i]->ExecutePredefined(eventdata);
	}
}

void Leviathan::Gui::JSNativeCoreAPI::HandlePacket(const GenericEvent &eventdata){
	// Just pass it to all the listeners //
	ObjectLock guard(*this);

	for(size_t i = 0; i < RegisteredListeners.size(); i++){

		RegisteredListeners[i]->ExecuteGenericEvent(eventdata);
	}
}

// ------------------ JSListener ------------------ //
Leviathan::Gui::JSNativeCoreAPI::JSListener::JSListener(const wstring &eventname, CefRefPtr<CefV8Value> callbackfunc, 
	CefRefPtr<CefV8Context> currentcontext) : EventsType(EVENT_TYPE_ERROR), IsGeneric(true), EventName(eventname), FunctionValueObject(callbackfunc), 
	FunctionsContext(currentcontext)
{

}

Leviathan::Gui::JSNativeCoreAPI::JSListener::JSListener(EVENT_TYPE etype, CefRefPtr<CefV8Value> callbackfunc, CefRefPtr<CefV8Context> currentcontext) : 
	EventsType(etype), IsGeneric(false), FunctionValueObject(callbackfunc), FunctionsContext(currentcontext)
{

}
// ------------------------------------ //
bool Leviathan::Gui::JSNativeCoreAPI::JSListener::ExecuteGenericEvent(const GenericEvent &eventdata){
	// Check does it match //

	return false;
}

bool Leviathan::Gui::JSNativeCoreAPI::JSListener::ExecutePredefined(const Event &eventdata){
	// Check does it match //

	return false;
}


