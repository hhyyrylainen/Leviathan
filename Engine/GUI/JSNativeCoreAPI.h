// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Events/Event.h"

#include "include/cef_app.h"

namespace Leviathan { namespace GUI {

class CefApplication;

//! \brief Provides an accessor interface for javascript for accessing NamedVars
class JSNamedVarsAccessor : public CefV8Accessor {
public:
    JSNamedVarsAccessor(NamedVars* valueobject);
    ~JSNamedVarsAccessor();

    virtual bool Get(const CefString& name, const CefRefPtr<CefV8Value> object,
        CefRefPtr<CefV8Value>& retval, CefString& exception) OVERRIDE;

    virtual bool Set(const CefString& name, const CefRefPtr<CefV8Value> object,
        const CefRefPtr<CefV8Value> value, CefString& exception) OVERRIDE;


    void AttachYourValues(CefRefPtr<CefV8Value> thisisyou);

    IMPLEMENT_REFCOUNTING(JSNamedVarsAccessor);

protected:
    NamedVars* OurValues;
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

    //! \brief Called when context is released, causes everything to be cleared
    void ClearContextValues();


    //! \brief Handles a packet received by this process
    void HandlePacket(const Event& eventdata);
    //! \brief Handles a generic packet
    void HandlePacket(GenericEvent& eventdata);

    IMPLEMENT_REFCOUNTING(JSNativeCoreAPI);

protected:
    //! Owner stored to be able to use it to bridge our requests to Gui::View
    CefApplication* Owner;

    //! Stores all registered functions
    std::vector<std::shared_ptr<JSListener>> RegisteredListeners;
};


}} // namespace Leviathan::GUI
