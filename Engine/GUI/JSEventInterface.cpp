// ------------------------------------ //
#include "JSEventInterface.h"

#include "GuiCEFApplication.h"
#include "JavaScriptHelper.h"
using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
JSNativeCoreAPI::~JSNativeCoreAPI() {}

JSNativeCoreAPI::JSNativeCoreAPI(CefApplication* owner) : Owner(owner) {}
// ------------------------------------ //
void JSNativeCoreAPI::ClearContextValues()
{
    GUARD_LOCK();
    // Stop listening //
    Owner->StopListeningForEvents();
    // This should be enough //
    RegisteredListeners.clear();
}
// ------------------------------------ //
bool JSNativeCoreAPI::Execute(const CefString& name, CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    // Check which function is called //
    if(name == "LOnEvent") {

        // Check are the arguments correct //
        if(arguments.size() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction()) {
            // Invalid arguments //
            exception = "Invalid arguments passed, expected: string, function";
            return true;
        }

        // Get the name //
        const std::string eventname = arguments[0]->GetStringValue();

        // Convert it to an event enum value //
        auto iter = EventListenerNameToEventMap.find(eventname);

        // Check was it found //
        if(iter == EventListenerNameToEventMap.end()) {
            exception = "No event matched name: " + eventname;
            return true;
        }

        // Bind it //
        auto tmplistener = std::make_shared<JSListener>(
            (*iter).second, arguments[1], CefV8Context::GetCurrentContext());

        GUARD_LOCK();

        // Add it //
        RegisteredListeners.push_back(tmplistener);

        // Notify our parent and make it work //
        Owner->StartListeningForEvent(tmplistener.get());

        return true;
    } else if(name == "LOnGeneric") {

        if(arguments.size() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction()) {
            // Invalid arguments //
            exception = "Invalid arguments passed, expected: string, function";
            return true;
        }

        // Get the name //
        const std::string eventname = arguments[0]->GetStringValue();

        // Bind it //
        auto tmplistener = std::make_shared<JSListener>(
            eventname, arguments[1], CefV8Context::GetCurrentContext());

        GUARD_LOCK();

        // Add it //
        RegisteredListeners.push_back(tmplistener);

        // Notify our parent and make it work //
        Owner->StartListeningForEvent(tmplistener.get());

        return true;
    } else if(name == "NotifyViewInputStatus") {

        if(arguments.size() < 1 || !arguments[0]->IsBool()) {
            // Invalid arguments //
            exception = "Invalid arguments passed, expected: bool";
            return true;
        }

        // Pack data to a message and send to the browser process
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("NotifyViewInputStatus");

        CefRefPtr<CefListValue> args = msg->GetArgumentList();

        args->SetBool(0, arguments[0]->GetBoolValue());

        // TODO: is this not as clean way to do this as calling through Owner that also has the
        // browser object
        CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    }

    // Not handled //
    return false;
}

void JSNativeCoreAPI::HandlePacket(const Event& eventdata)
{
    // Just pass it to all the listeners //
    GUARD_LOCK();

    for(size_t i = 0; i < RegisteredListeners.size(); i++) {

        RegisteredListeners[i]->ExecutePredefined(eventdata);
    }
}

void JSNativeCoreAPI::HandlePacket(GenericEvent& eventdata)
{
    // Just pass it to all the listeners //
    GUARD_LOCK();

    for(size_t i = 0; i < RegisteredListeners.size(); i++) {

        RegisteredListeners[i]->ExecuteGenericEvent(eventdata);
    }
}

// ------------------ JSListener ------------------ //
JSNativeCoreAPI::JSListener::JSListener(const std::string& eventname,
    CefRefPtr<CefV8Value> callbackfunc, CefRefPtr<CefV8Context> currentcontext) :
    IsGeneric(true),
    EventsType(EVENT_TYPE_ERROR), EventName(eventname), FunctionValueObject(callbackfunc),
    FunctionsContext(currentcontext)
{}

JSNativeCoreAPI::JSListener::JSListener(EVENT_TYPE etype, CefRefPtr<CefV8Value> callbackfunc,
    CefRefPtr<CefV8Context> currentcontext) :
    IsGeneric(false),
    EventsType(etype), FunctionValueObject(callbackfunc), FunctionsContext(currentcontext)
{}
// ------------------------------------ //
bool JSNativeCoreAPI::JSListener::ExecuteGenericEvent(GenericEvent& eventdata)
{
    // Check does it match //
    if(!IsGeneric || EventName != eventdata.GetType()) {

        return false;
    }

    // Call the javascript callback with it //
    CefV8ValueList args;

    // Enter the current context //
    FunctionsContext->Enter();

    // Set the type as int //
    args.push_back(CefV8Value::CreateString(EventName));

    // Create a new accessor object //
    JSNamedVarsAccessor* directptr = new JSNamedVarsAccessor(eventdata.GetVariables());

    CefRefPtr<CefV8Accessor> tmpaccess(directptr);

    // Create the object //
    CefRefPtr<CefV8Value> arrayobjval = CefV8Value::CreateObject(tmpaccess, nullptr);

    // Attach the values //
    directptr->AttachYourValues(arrayobjval);

    // Add to the args //
    args.push_back(arrayobjval);

    // Invoke the function //
    CefRefPtr<CefV8Value> retval = FunctionValueObject->ExecuteFunction(NULL, args);

    // Leave the context //
    FunctionsContext->Exit();

    if(!retval) {
        // It failed //
        return false;
    }


    return true;
}

bool JSNativeCoreAPI::JSListener::ExecutePredefined(const Event& eventdata)
{
    // Check does it match //
    if(IsGeneric || EventsType != eventdata.GetType()) {

        return false;
    }

    // Call the javascript callback, right now people hopefully won't mind if the accessor
    // object is null //
    CefV8ValueList args;

    // Enter the current context //
    FunctionsContext->Enter();

    // Set the type as int //
    args.push_back(CefV8Value::CreateInt(EventsType));

    // We can see if we can pass an int to it //
    auto dataptr = eventdata.GetIntegerDataForEvent();

    if(dataptr) {
        // Add the integer value //
        args.push_back(CefV8Value::CreateInt(dataptr->IntegerDataValue));

    } else {
        // We should add an accessor object, but for now null will do //
        args.push_back(CefV8Value::CreateNull());
    }

    // Invoke the function //
    CefRefPtr<CefV8Value> retval = FunctionValueObject->ExecuteFunction(NULL, args);

    FunctionsContext->Exit();

    if(!retval) {
        // It failed //
        return false;
    }

    return true;
}
// ------------------ JSNamedVarsAccessor ------------------ //
JSNamedVarsAccessor::JSNamedVarsAccessor(NamedVars* valueobject)
{
    valueobject->AddRef();
    OurValues = valueobject;
}

JSNamedVarsAccessor::~JSNamedVarsAccessor()
{
    OurValues->Release();
}
// ------------------------------------ //
bool JSNamedVarsAccessor::Get(const CefString& name, const CefRefPtr<CefV8Value> object,
    CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    // Try to find the value //
    NamedVariableList* block = OurValues->GetValueDirect(name).get();

    if(!block) {
        // Value not found //
        return false;
    }

    // Convert the value //
    retval = JavaScriptHelper::ConvertNamedVariableListToJavaScriptValue(block);

    // Value found //
    return true;
}

bool JSNamedVarsAccessor::Set(const CefString& name, const CefRefPtr<CefV8Value> object,
    const CefRefPtr<CefV8Value> value, CefString& exception)
{
    // Disallow setting now //
    exception = "Set unallowed for JSNamedVarsAccessor";
    return true;
}
// ------------------------------------ //
void JSNamedVarsAccessor::AttachYourValues(CefRefPtr<CefV8Value> thisisyou)
{
    // All the values need to be attached for this to work properly //
    auto vecval = OurValues->GetVec();

    for(size_t i = 0; i < vecval->size(); i++) {

        // Bind the value //
        thisisyou->SetValue(
            vecval->at(i)->GetName(), V8_ACCESS_CONTROL_DEFAULT, V8_PROPERTY_ATTRIBUTE_NONE);
    }
}
