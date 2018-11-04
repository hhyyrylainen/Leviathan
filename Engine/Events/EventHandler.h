// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "CallableObject.h"
#include "Event.h"

namespace Leviathan {

//! \brief Allows object to register for events that can be fired from anywhere
//!
//! This is recursive to allow EventCallbacks to also fire events. This makes it possible to
//! cause a stackoverflow but makes it easier to make events that fire a different event
class EventHandler : public ThreadSafeRecursive {
public:
    DLLEXPORT EventHandler();
    DLLEXPORT ~EventHandler();

    DLLEXPORT bool Init();
    DLLEXPORT void Release();

    //! \param event The event to send. Reference count will be decremented by this
    DLLEXPORT void CallEvent(Event* event);
    DLLEXPORT inline void CallEvent(const Event::pointer& event)
    {
        event->AddRef();
        CallEvent(event.get());
    }

    //! \param event The event to send. Reference count will be decremented by this
    DLLEXPORT void CallEvent(GenericEvent* event);
    //! \todo Swap this with the function above to make this more efficient (and switch to
    //! using this)
    DLLEXPORT inline void CallEvent(const GenericEvent::pointer& event)
    {
        event->AddRef();
        CallEvent(event.get());
    }

    DLLEXPORT bool RegisterForEvent(CallableObject* toregister, EVENT_TYPE totype);
    DLLEXPORT bool RegisterForEvent(
        CallableObject* toregister, const std::string& genericname);

    DLLEXPORT void Unregister(CallableObject* caller, EVENT_TYPE type, bool all = false);
    DLLEXPORT void Unregister(
        CallableObject* caller, const std::string& genericname, bool all = false);

private:
    std::vector<std::tuple<CallableObject*, EVENT_TYPE>> EventListeners;
    std::vector<std::tuple<CallableObject*, std::string>> GenericEventListeners;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::EventHandler;
#endif
