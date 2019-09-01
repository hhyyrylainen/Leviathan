// ------------------------------------ //
#include "JSNativeCoreAPI.h"

#include "CEFConversionHelpers.h"
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
    // GUARD_LOCK();
    // Stop listening //
    Owner->StopListeningForEvents();
    // This should be enough //
    RegisteredListeners.clear();

    // Let go of callbacks
    PendingRequestCallbacks.clear();
}
// ------------------------------------ //
bool JSNativeCoreAPI::Execute(const CefString& name, CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    // Check which function is called //

    // These are called a lot
    if(name == "NotifyViewInputStatus") {

        if(arguments.size() < 1 || !arguments[0]->IsBool()) {
            // Invalid arguments //
            exception = "Invalid arguments passed, expected: bool";
            return true;
        }

        // Pack data to a message and send to the browser process
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("NotifyViewInputStatus");

        CefRefPtr<CefListValue> args = msg->GetArgumentList();

        args->SetBool(0, arguments[0]->GetBoolValue());

        SendProcessMessage(msg);
        return true;

    } else if(name == "NotifyViewScrollableStatus") {

        if(arguments.size() < 1 || !arguments[0]->IsBool()) {
            // Invalid arguments //
            exception = "Invalid arguments passed, expected: bool";
            return true;
        }

        // Pack data to a message and send to the browser process
        CefRefPtr<CefProcessMessage> msg =
            CefProcessMessage::Create("NotifyViewScrollableStatus");

        CefRefPtr<CefListValue> args = msg->GetArgumentList();

        args->SetBool(0, arguments[0]->GetBoolValue());

        SendProcessMessage(msg);
        return true;
    }

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

        // GUARD_LOCK();

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

        // GUARD_LOCK();

        // Add it //
        RegisteredListeners.push_back(tmplistener);

        // Notify our parent and make it work //
        Owner->StartListeningForEvent(tmplistener.get());

        return true;

    } else if(name == "CallGenericEvent") {

        if(arguments.size() < 1 || !arguments[0]->IsString() ||
            (arguments.size() > 1 && !arguments[1]->IsObject())) {

            // Invalid arguments //
            exception = "Invalid arguments passed, expected: string, [object]";
            return true;
        }

        // Get the name //
        const std::string eventname = arguments[0]->GetStringValue();

        // Pack data to a message and send to the browser process
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("CallGenericEvent");

        CefRefPtr<CefListValue> args = msg->GetArgumentList();

        args->SetString(0, arguments[0]->GetStringValue());

        if(arguments.size() > 1) {

            CefRefPtr<CefDictionaryValue> dictionary = CefDictionaryValue::Create();
            JSObjectToDictionary(arguments[1], dictionary);

            args->SetDictionary(1, dictionary);
        }

        SendProcessMessage(msg);
        return true;

    } else if(name == "Play2DSoundEffect") {

        if(arguments.size() < 1 || !arguments[0]->IsString()) {
            // Invalid arguments //
            exception = "Invalid arguments passed, expected: string";
            return true;
        }

        // Pack data to a message and send to the browser process
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("Play2DSoundEffect");

        CefRefPtr<CefListValue> args = msg->GetArgumentList();

        args->SetString(0, arguments[0]->GetStringValue());

        SendProcessMessage(msg);
        return true;

    } else if(name == "Play2DSound") {

        if(arguments.size() < 3 || !arguments[0]->IsString() || !arguments[1]->IsBool() ||
            // !arguments[2]->IsBool() ||
            !arguments[2]->IsFunction()) {
            // Invalid arguments //
            exception = "Invalid arguments passed, expected: string, bool, function";
            return true;
        }

        // Add to queue
        const auto requestNumber = ++RequestSequenceNumber;

        PendingRequestCallbacks.push_back(std::make_tuple(
            requestNumber, arguments[2], nullptr, CefV8Context::GetCurrentContext()));

        // Pack data to a message and send to the browser process
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("AudioSource");

        CefRefPtr<CefListValue> args = msg->GetArgumentList();

        args->SetString(0, "new");

        args->SetInt(1, requestNumber);
        args->SetString(2, arguments[0]->GetStringValue());
        args->SetBool(3, arguments[1]->GetBoolValue());
        // args->SetBool(4, arguments[2]->GetBoolValue());
        SendProcessMessage(msg);
        return true;
    } else if(name == "PlayCutscene") {

        if(arguments.size() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction()) {
            // Invalid arguments //
            exception = "Invalid arguments passed, expected: string, function, [function]";
            return true;
        }

        // Add to queue
        const auto requestNumber = ++RequestSequenceNumber;

        PendingRequestCallbacks.push_back(std::make_tuple(requestNumber, arguments[1],
            arguments.size() > 2 ? arguments[2] : nullptr, CefV8Context::GetCurrentContext()));

        // Pack data to a message and send to the browser process
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("PlayCutscene");

        CefRefPtr<CefListValue> args = msg->GetArgumentList();

        args->SetString(0, "Play");
        args->SetInt(1, requestNumber);
        args->SetString(2, arguments[0]->GetStringValue());

        SendProcessMessage(msg);

        return true;
    } else if(name == "CancelCutscene") {

        // Pack data to a message and send to the browser process
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("PlayCutscene");

        CefRefPtr<CefListValue> args = msg->GetArgumentList();

        args->SetString(0, "Cancel");
        SendProcessMessage(msg);

        return true;
    }

    // Not handled //
    return false;
}
// ------------------------------------ //
void JSNativeCoreAPI::SendProcessMessage(CefRefPtr<CefProcessMessage> message)
{
    Owner->SendProcessMessage(message);
}
// ------------------------------------ //
bool JSNativeCoreAPI::HandleProcessMessage(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame, CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    if(message->GetName() == "AudioSource") {

        auto args = message->GetArgumentList();

        // First parameter is the type and second is the request number
        const auto& type = args->GetString(0);
        const auto& requestNumber = args->GetInt(1);

        // Then handle on the type //
        if(type == "new") {

            // Detect creation
            // The only parameter is the id we use for future calls related to this source
            const auto& createdId = args->GetInt(2);

            // GUARD_LOCK();

            // Find the request
            const auto requestIndex = FindRequestByNumber(requestNumber);

            if(requestIndex >= PendingRequestCallbacks.size()) {
                LOG(ERROR) << "JSNativeCoreAPI AudioSource response didn't find request: "
                           << requestNumber;
                return true;
            }

            const auto& callback = PendingRequestCallbacks[requestIndex];

            CefRefPtr<CefV8Context> context = std::get<3>(callback);

#ifdef JSNATIVE_CORE_API_VERBOSE
            LOG(INFO) << "JSNativeCoreAPI AudioSource created: " << createdId;
#endif

            // Enter the current context to be able to create things //
            context->Enter();

            {
                // Create the proxy object
                CefRefPtr<CefV8Value> proxy;

                if(createdId == -1) {
                    // Failed
                    proxy->CreateNull();
                } else {
                    CefRefPtr<CefV8Interceptor> interceptor =
                        new JSAudioSourceInterceptor(*this, createdId);
                    proxy = CefV8Value::CreateObject(nullptr, interceptor);
                    proxy->SetUserData(interceptor);
                }

                // And give it to JavaScript
                CefV8ValueList args;
                args.push_back(proxy);

                // We don't care about the return value
                std::get<1>(callback)->ExecuteFunction(nullptr, args);
            }

            // Leave the context //
            context->Exit();

            // And let go of the callback
            PendingRequestCallbacks.erase(PendingRequestCallbacks.begin() + requestIndex);

            return true;
        }

        LOG(ERROR) << "Unknown JSNativeCoreAPI AudioSource response: "
                   << Convert::Utf16ToUtf8(type);
        return true;

    } else if(message->GetName() == "PlayCutscene") {

        auto args = message->GetArgumentList();

        // First parameter is the type and second is the request number
        const auto& type = args->GetString(0);
        const auto& requestNumber = args->GetInt(1);

        // All of these message types use the request
        // Find the request
        const auto requestIndex = FindRequestByNumber(requestNumber);

        if(requestIndex >= PendingRequestCallbacks.size()) {
            LOG(ERROR) << "JSNativeCoreAPI PlayCutscene response didn't find request: "
                       << requestNumber;
            return true;
        }

        const auto& callback = PendingRequestCallbacks[requestIndex];

        if(type == "Finished") {

            // Call finish callback
            CefV8ValueList args;

            std::get<1>(callback)->ExecuteFunctionWithContext(
                std::get<3>(callback), nullptr, args);

        } else if(type == "Error") {
            // Call error callback
            // We got the error string
            const auto& errorStr = args->GetString(2);

            CefV8ValueList args;
            args.push_back(CefV8Value::CreateString(errorStr));

            std::get<2>(callback)->ExecuteFunctionWithContext(
                std::get<3>(callback), nullptr, args);
        } else {

            LOG(ERROR) << "Unknown JSNativeCoreAPI PlayCutscene response: "
                       << Convert::Utf16ToUtf8(type);
        }

        // And let go of the callback
        PendingRequestCallbacks.erase(PendingRequestCallbacks.begin() + requestIndex);

        return true;
    }


    return false;
}


size_t JSNativeCoreAPI::FindRequestByNumber(int number) const
{
    for(size_t i = 0; i < PendingRequestCallbacks.size(); ++i) {

        if(std::get<0>(PendingRequestCallbacks[i]) == number)
            return i;
    }

    return -1;
}

// ------------------------------------ //
void JSNativeCoreAPI::HandlePacket(const Event& eventdata)
{
    // Just pass it to all the listeners //
    // GUARD_LOCK();

    for(size_t i = 0; i < RegisteredListeners.size(); i++) {

        RegisteredListeners[i]->ExecutePredefined(eventdata);
    }
}

void JSNativeCoreAPI::HandlePacket(GenericEvent& eventdata)
{
    // Just pass it to all the listeners //
    // GUARD_LOCK();

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

    // Enter the current context //
    FunctionsContext->Enter();

    {
        // Call the javascript callback with it //
        CefV8ValueList args;

        // First parameter is the event name //
        args.push_back(CefV8Value::CreateString(EventName));

        // Create a new accessor object //
        CefRefPtr<JSNamedVarsInterceptor> interceptor =
            new JSNamedVarsInterceptor(eventdata.GetVariables());

        // Create the object //
        CefRefPtr<CefV8Value> arrayobjval = CefV8Value::CreateObject(nullptr, interceptor);

        // Doesn't work
        // interceptor->BindValues(arrayobjval);

        // But user data needs to be set
        arrayobjval->SetUserData(interceptor);

        // Add to the args //
        args.push_back(arrayobjval);

        // Invoke the function //
        CefRefPtr<CefV8Value> retval = FunctionValueObject->ExecuteFunction(NULL, args);

        // if(!retval) {
        //     // It failed //
        //     return false;
        // }
    }

    // Leave the context //
    FunctionsContext->Exit();

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
JSNamedVarsInterceptor::JSNamedVarsInterceptor(NamedVars::pointer obj) : Values(obj) {}

JSNamedVarsInterceptor::~JSNamedVarsInterceptor() {}
// ------------------------------------ //
bool JSNamedVarsInterceptor::Get(int index, const CefRefPtr<CefV8Value> object,
    CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    return false;
}

bool JSNamedVarsInterceptor::Get(const CefString& name, const CefRefPtr<CefV8Value> object,
    CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    if(name == "keys") {
        retval = CefV8Value::CreateFunction("keys",
            new JSLambdaFunction(
                [](const CefString& name, CefRefPtr<CefV8Value> object,
                    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                    CefString& exception) -> bool {
                    if(name == "keys") {

                        // The JSNamedVarsInterceptor is the user data
                        if(!object) {
                            exception = "No 'this' passed to function";
                            return true;
                        }

                        auto userData = object->GetUserData();

                        if(!userData) {
                            exception = "'this' has no userdata";
                            return true;
                        }

                        auto* casted = dynamic_cast<JSNamedVarsInterceptor*>(userData.get());

                        if(!casted) {
                            exception =
                                "'this' was of wrong type. Excepted JSNamedVarsInterceptor";
                            return true;
                        }

                        if(!casted->Values) {

                            retval = CefV8Value::CreateArray(0);
                            return true;
                        }

                        auto vecval = casted->Values->GetVec();

                        const int size = static_cast<int>(vecval->size());

                        retval = CefV8Value::CreateArray(size);

                        for(int i = 0; i < size; i++) {

                            retval->SetValue(
                                i, CefV8Value::CreateString(vecval->at(i)->GetName()));
                        }

                        return true;
                    }

                    return false;
                }));
        return true;
    }

    if(!Values)
        return false;

    // Try to find the value //
    NamedVariableList* block = Values->GetValueDirect(name).get();

    if(!block) {
        // Value not found //
        return false;
    }

    // Convert the value //
    retval = JavaScriptHelper::ConvertNamedVariableListToJavaScriptValue(block);

    // Value found //
    return true;
}

bool JSNamedVarsInterceptor::Set(int index, const CefRefPtr<CefV8Value> object,
    const CefRefPtr<CefV8Value> value, CefString& exception)
{
    return false;
}

bool JSNamedVarsInterceptor::Set(const CefString& name, const CefRefPtr<CefV8Value> object,
    const CefRefPtr<CefV8Value> value, CefString& exception)
{
    exception = "TODO: Set for JSNamedVarsInterceptor";
    return true;
}

void JSNamedVarsInterceptor::BindValues(CefRefPtr<CefV8Value> object)
{
    auto vecval = Values->GetVec();

    for(size_t i = 0; i < vecval->size(); i++) {

        LOG(INFO) << "Binding index: " << i;

        // Bind the value //
        object->SetValue(
            vecval->at(i)->GetName(), V8_ACCESS_CONTROL_DEFAULT, V8_PROPERTY_ATTRIBUTE_NONE);
    }
}
// ------------------------------------ //
// JSAudioSourceInterceptor
JSAudioSourceInterceptor::JSAudioSourceInterceptor(JSNativeCoreAPI& messagebridge, int id) :
    ID(id), MessageBridge(messagebridge)
{}

JSAudioSourceInterceptor::~JSAudioSourceInterceptor()
{
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("AudioSource");

    CefRefPtr<CefListValue> args = message->GetArgumentList();

    args->SetString(0, "destroy");
    args->SetInt(1, ID);

    MessageBridge.SendProcessMessage(message);
}
// ------------------------------------ //
bool JSAudioSourceInterceptor::Get(const CefString& name, const CefRefPtr<CefV8Value> object,
    CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    if(name == "Pause") {
        retval = CefV8Value::CreateFunction("Pause",
            new JSLambdaFunction(
                [](const CefString& name, CefRefPtr<CefV8Value> object,
                    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                    CefString& exception) -> bool {
                    if(name == "Pause") {

                        // The JSAudioSourceInterceptor is the user data
                        if(!object) {
                            exception = "No 'this' passed to function";
                            return true;
                        }

                        auto userData = object->GetUserData();

                        if(!userData) {
                            exception = "'this' has no userdata";
                            return true;
                        }

                        auto* casted = dynamic_cast<JSAudioSourceInterceptor*>(userData.get());

                        if(!casted) {
                            exception =
                                "'this' was of wrong type. Excepted JSAudioSourceInterceptor";
                            return true;
                        }

                        casted->Pause();
                        return true;
                    }

                    return false;
                }));
        return true;
    } else if(name == "Resume") {
        retval = CefV8Value::CreateFunction("Resume",
            new JSLambdaFunction(
                [](const CefString& name, CefRefPtr<CefV8Value> object,
                    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
                    CefString& exception) -> bool {
                    if(name == "Resume") {

                        // The JSAudioSourceInterceptor is the user data
                        if(!object) {
                            exception = "No 'this' passed to function";
                            return true;
                        }

                        auto userData = object->GetUserData();

                        if(!userData) {
                            exception = "'this' has no userdata";
                            return true;
                        }

                        auto* casted = dynamic_cast<JSAudioSourceInterceptor*>(userData.get());

                        if(!casted) {
                            exception =
                                "'this' was of wrong type. Excepted JSAudioSourceInterceptor";
                            return true;
                        }

                        casted->Resume();
                        return true;
                    }

                    return false;
                }));
        return true;
    }

    return false;
}

bool JSAudioSourceInterceptor::Get(int index, const CefRefPtr<CefV8Value> object,
    CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    return false;
}

bool JSAudioSourceInterceptor::Set(int index, const CefRefPtr<CefV8Value> object,
    const CefRefPtr<CefV8Value> value, CefString& exception)
{
    return false;
}

bool JSAudioSourceInterceptor::Set(const CefString& name, const CefRefPtr<CefV8Value> object,
    const CefRefPtr<CefV8Value> value, CefString& exception)
{
    return false;
}
// ------------------------------------ //
DLLEXPORT void JSAudioSourceInterceptor::Pause()
{
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("AudioSource");

    CefRefPtr<CefListValue> args = message->GetArgumentList();

    args->SetString(0, "Pause");
    // No callback
    args->SetInt(1, -1);
    args->SetInt(2, ID);

    MessageBridge.SendProcessMessage(message);
}

DLLEXPORT void JSAudioSourceInterceptor::Resume()
{
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("AudioSource");

    CefRefPtr<CefListValue> args = message->GetArgumentList();

    args->SetString(0, "Resume");
    // No callback
    args->SetInt(1, -1);
    args->SetInt(2, ID);

    MessageBridge.SendProcessMessage(message);
}
