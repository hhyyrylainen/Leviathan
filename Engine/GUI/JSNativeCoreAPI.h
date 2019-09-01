// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Events/Event.h"

#include "include/cef_app.h"

#include <atomic>

// #define JSNATIVE_CORE_API_VERBOSE

namespace Leviathan { namespace GUI {

class CefApplication;
class JSNativeCoreAPI;

//! \brief Helps with binding c++ functions to JavaScript by allowing lambdas to be easily
//! registered
class JSLambdaFunction : public CefV8Handler {
public:
    using HandlerSignature = std::function<bool(const CefString& name,
        CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
        CefRefPtr<CefV8Value>& retval, CefString& exception)>;

    inline JSLambdaFunction(HandlerSignature handler) : Handler(handler) {}

    inline bool Execute(const CefString& name, CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
        CefString& exception) override
    {
        return Handler(name, object, arguments, retval, exception);
    }

    IMPLEMENT_REFCOUNTING(JSLambdaFunction);

private:
    const HandlerSignature Handler;
};

//! \brief Provides access for JS to NamedVars
//! \todo Change this to an interceptor to not have to call SetValue in AttachYourValues
class JSNamedVarsInterceptor : public CefV8Interceptor {
public:
    JSNamedVarsInterceptor(NamedVars::pointer obj);
    ~JSNamedVarsInterceptor();

    bool Get(int index, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval,
        CefString& exception) override;
    bool Get(const CefString& name, const CefRefPtr<CefV8Value> object,
        CefRefPtr<CefV8Value>& retval, CefString& exception) override;
    bool Set(int index, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value,
        CefString& exception) override;
    bool Set(const CefString& name, const CefRefPtr<CefV8Value> object,
        const CefRefPtr<CefV8Value> value, CefString& exception) override;

    //! \brief This binds all current values to the js object so that
    //! Object.keys can detect them
    //! \warning This doesn't work (apparently this only works with Accessors which we don't
    //! want to use for this, but maybe useful in the future)
    void BindValues(CefRefPtr<CefV8Value> object);

    IMPLEMENT_REFCOUNTING(JSNamedVarsInterceptor);

protected:
    NamedVars::pointer Values;
};

//! \brief Provides access for JavaScript to AudioSource
//! \todo Expose a destroy method to js as relying on garbage collecting doesn't seem that
//! great
class JSAudioSourceInterceptor : public CefV8Interceptor {
public:
    JSAudioSourceInterceptor(JSNativeCoreAPI& messagebridge, int id);
    //! \brief Sends the destroy message for the proxied object
    ~JSAudioSourceInterceptor();

    bool Get(int index, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval,
        CefString& exception) override;
    bool Get(const CefString& name, const CefRefPtr<CefV8Value> object,
        CefRefPtr<CefV8Value>& retval, CefString& exception) override;
    bool Set(int index, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value,
        CefString& exception) override;
    bool Set(const CefString& name, const CefRefPtr<CefV8Value> object,
        const CefRefPtr<CefV8Value> value, CefString& exception) override;

    // JS exposed functions
    DLLEXPORT void Pause();
    DLLEXPORT void Resume();

    IMPLEMENT_REFCOUNTING(JSAudioSourceInterceptor);

protected:
    //! Our ID that we use to send our operations as messages
    int ID;
    JSNativeCoreAPI& MessageBridge;
};


//! \brief Handles javascript functions that have native extensions
//!
//! This needs to be recursively lockable as we can't pass the locks through JavaScript
//! \todo Figure out if CEF actually runs multiple threads in the render process. This may not
//! actually be the case so the lack of locking here might not cause issues. The lock was
//! removed from here to not cause deadlocking, and because ThreadSafeRecursive didn't compile
//! with visual studio while we are processing something else if that is true
class JSNativeCoreAPI : public CefV8Handler /*, public ThreadSafeRecursive*/ {
    friend CefApplication;

    //! \brief Class that holds everything related to a listen callback
    //! \todo Add support for passing event values
    class JSListener {
        friend CefApplication;

    public:
        //! Creates a new listener with a predefined event type
        JSListener(EVENT_TYPE etype, CefRefPtr<CefV8Value> callbackfunc,
            CefRefPtr<CefV8Context> currentcontext);
        //! Creates a new listener from a GenericEvent name
        JSListener(const std::string& eventname, CefRefPtr<CefV8Value> callbackfunc,
            CefRefPtr<CefV8Context> currentcontext);

        //! \brief Executes this if this is a predefined type
        bool ExecutePredefined(const Event& eventdata);

        //! \brief Executes this if this is a generic type
        bool ExecuteGenericEvent(GenericEvent& eventdata);

    protected:
        //! Marks whether EventsType or EventName is filled
        bool IsGeneric;
        //! Event's type when it is a predefined event
        EVENT_TYPE EventsType;
        //! Stores name of the generic event
        std::string EventName;

        //! V8 pointers
        CefRefPtr<CefV8Value> FunctionValueObject;
        CefRefPtr<CefV8Context> FunctionsContext;
    };


public:
    JSNativeCoreAPI(CefApplication* owner);
    ~JSNativeCoreAPI();


    //! \brief Handles calls from javascript
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
        CefString& exception) override;

    //! \brief Handles returned messages from the browser process
    bool HandleProcessMessage(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefProcessId source_process, CefRefPtr<CefProcessMessage> message);

    //! \brief Called when context is released, causes everything to be cleared
    void ClearContextValues();


    //! \brief Handles a packet received by this process
    void HandlePacket(const Event& eventdata);
    //! \brief Handles a generic packet
    void HandlePacket(GenericEvent& eventdata);

    //! \brief Sends a process message to the main process
    void SendProcessMessage(CefRefPtr<CefProcessMessage> message);

    IMPLEMENT_REFCOUNTING(JSNativeCoreAPI);

protected:
    size_t FindRequestByNumber(int number) const;

protected:
    //! Owner stored to be able to use it to bridge our requests to Gui::View
    CefApplication* Owner;

    //! Stores all registered functions
    std::vector<std::shared_ptr<JSListener>> RegisteredListeners;

private:
    std::atomic<int> RequestSequenceNumber = 0;
    //! These are used to handle finished execution of remote calls that return values
    //! The second value is the function in case of errors (can be null for many requests)
    std::vector<
        std::tuple<int, CefRefPtr<CefV8Value>, CefRefPtr<CefV8Value>, CefRefPtr<CefV8Context>>>
        PendingRequestCallbacks;

    // TODO: I was dumb so I messed using this up so this should probably work but isn't used
    // currently in the hopes that doing the check in js before calling native code is faster
    // // These are cached states. These need to match the ones in GUI::View
    // // These are used to not send redundant requests
    // std::atomic<bool> InputFocused = false;
    // std::atomic<bool> ScrollableElement = false;
};


}} // namespace Leviathan::GUI
