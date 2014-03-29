#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_JSEVENTINTERFACE
#include "JSEventInterface.h"
#endif
#include "GuiCEFApplication.h"
#include "JavaScriptHelper.h"
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
Leviathan::Gui::JSNativeCoreAPI::~JSNativeCoreAPI(){

}

Leviathan::Gui::JSNativeCoreAPI::JSNativeCoreAPI(CefApplication* owner) : Owner(owner){

}
// ------------------------------------ //
void Leviathan::Gui::JSNativeCoreAPI::ClearContextValues(){
	GUARD_LOCK_THIS_OBJECT();
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
		shared_ptr<JSListener> tmplistener(new JSListener((*iter).second, arguments[1], CefV8Context::GetCurrentContext()));

		GUARD_LOCK_THIS_OBJECT();

		// Add it //
		RegisteredListeners.push_back(tmplistener);

		// Notify our parent and make it work //
		Owner->StartListeningForEvent(tmplistener.get());

		return true;
	} else if(name == "LOnGeneric"){

		if(arguments.size() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction()){
			// Invalid arguments //
			exception = "Invalid arguments passed, expected: string, function";
			return true;
		}

		// Get the name //
		const wstring eventname = arguments[0]->GetStringValue();

		// Bind it //
		shared_ptr<JSListener> tmplistener(new JSListener(eventname, arguments[1], CefV8Context::GetCurrentContext()));

		GUARD_LOCK_THIS_OBJECT();

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
	GUARD_LOCK_THIS_OBJECT();

	for(size_t i = 0; i < RegisteredListeners.size(); i++){

		RegisteredListeners[i]->ExecutePredefined(eventdata);
	}
}

void Leviathan::Gui::JSNativeCoreAPI::HandlePacket(GenericEvent &eventdata){
	// Just pass it to all the listeners //
	GUARD_LOCK_THIS_OBJECT();

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
	bool Leviathan::Gui::JSNativeCoreAPI::JSListener::ExecuteGenericEvent(GenericEvent &eventdata){
	// Check does it match //
	if(!IsGeneric || EventName != eventdata.GetType()){

		return false;
	}

	// Call the javascript callback with it //
	CefV8ValueList args;

	// Enter the current context //
	FunctionsContext->Enter();

	// Set the type as int //
	args.push_back(CefV8Value::CreateString(EventName));
	
	// Create a new accessor object //
	CefRefPtr<CefV8Accessor> tmpaccess(new JSNamedVarsAccessor(eventdata.GetVariables()));

	// Create the object //
	args.push_back(CefV8Value::CreateObject(tmpaccess));


	// Invoke the function //
	CefRefPtr<CefV8Value> retval = FunctionValueObject->ExecuteFunction(NULL, args);

	// Leave the context //
	FunctionsContext->Exit();

	if(!retval){
		// It failed //
		return false;
	}


	return false;
}

bool Leviathan::Gui::JSNativeCoreAPI::JSListener::ExecutePredefined(const Event &eventdata){
	// Check does it match //
	if(IsGeneric || EventsType != eventdata.GetType()){

		return false;
	}

	// Call the javascript callback, right now people hopefully won't mind if the accessor object is null //
	CefV8ValueList args;

	// Enter the current context //
	FunctionsContext->Enter();

	// Set the type as int //
	args.push_back(CefV8Value::CreateInt(EventsType));

	// We can see if we can pass an int to it //
	auto dataptr = eventdata.GetIntegerDataForEvent();

	if(dataptr){
		// Add the integer value //
		args.push_back(CefV8Value::CreateInt(dataptr->IntegerDataValue));

	} else {
		// We should add an accessor object, but for now null will do //
		args.push_back(CefV8Value::CreateNull());
	}

	// Invoke the function //
	CefRefPtr<CefV8Value> retval = FunctionValueObject->ExecuteFunction(NULL, args);

	FunctionsContext->Exit();

	if(!retval){
		// It failed //
		return false;
	}

	return true;
}
// ------------------ JSNamedVarsAccessor ------------------ //
Leviathan::Gui::JSNamedVarsAccessor::JSNamedVarsAccessor(NamedVars* valueobject){
	valueobject->AddRef();
	OurValues = valueobject;
}

Leviathan::Gui::JSNamedVarsAccessor::~JSNamedVarsAccessor(){
	OurValues->Release();
}
// ------------------------------------ //
bool Leviathan::Gui::JSNamedVarsAccessor::Get(const CefString& name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval, CefString& exception){
	// Try to find the value //
	NamedVariableList* block = OurValues->GetValueDirect(name).get();

	if(!block){
		// Value not found //
		return false;
	}

	// Convert the value //
	retval = JavaScriptHelper::ConvertNamedVariableListToJavaScriptValue(block);

	// Value found //
	return true;
}

bool Leviathan::Gui::JSNamedVarsAccessor::Set(const CefString& name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString& exception){
	// Disallow setting now //
	exception = "Set unallowed for JSNamedVarsAccessor";
	return true;
}


