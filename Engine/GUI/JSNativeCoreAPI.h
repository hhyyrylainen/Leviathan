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

//! \brief Provides an accessor interface for javascript for accessing NamedVars
class JSNamedVarsAccessor : public CefV8Accessor {
public:
    JSNamedVarsAccessor(NamedVars* valueobject);
    ~JSNamedVarsAccessor();

    virtual bool Get(const CefString& name, const CefRefPtr<CefV8Value> object,
        CefRefPtr<CefV8Value>& retval, CefString& exception) override;

    virtual bool Set(const CefString& name, const CefRefPtr<CefV8Value> object,
        const CefRefPtr<CefV8Value> value, CefString& exception) override;

    void AttachYourValues(CefRefPtr<CefV8Value> thisisyou);

    IMPLEMENT_REFCOUNTING(JSNamedVarsAccessor);

protected:
    NamedVars* OurValues;
};

//! \brief Provides access for JavaScript to AudioSource
class JSAudioSourceAccessor : public CefV8Accessor {
public:
    JSAudioSourceAccessor(JSNativeCoreAPI& messagebridge, int id);
    //! \brief Sends the destroy message for the proxied object
    ~JSAudioSourceAccessor();

    bool Get(const CefString& name, const CefRefPtr<CefV8Value> object,
        CefRefPtr<CefV8Value>& retval, CefString& exception) override;

    bool Set(const CefString& name, const CefRefPtr<CefV8Value> object,
        const CefRefPtr<CefV8Value> value, CefString& exception) override;

    // JS exposed functions
    DLLEXPORT void Pause();

    IMPLEMENT_REFCOUNTING(JSAudioSourceAccessor);

protected:
    //! Our ID that we use to send our operations as messages
    int ID;
    JSNativeCoreAPI& MessageBridge;
};


//! \brief Handles javascript functions that have native extensions //
class JSNativeCoreAPI : public CefV8Handler, public ThreadSafe {
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
    bool HandleProcessMessage(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message);

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
};


}} // namespace Leviathan::GUI
