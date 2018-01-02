// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Event.h"
#include "CallableObject.h"

namespace Leviathan{

//! \brief Allows object to register for events that can be fired from anywhere
class EventHandler : public ThreadSafe{
public:
    DLLEXPORT EventHandler();
    DLLEXPORT ~EventHandler();

    DLLEXPORT bool Init();
    DLLEXPORT void Release();

    //! \param event The event to send. Reference count will be decremented by this
    DLLEXPORT void CallEvent(Event* event);

    //! \param event The event to send. Reference count will be decremented by this
    DLLEXPORT void CallEvent(GenericEvent* event);

    DLLEXPORT bool RegisterForEvent(CallableObject* toregister, EVENT_TYPE totype);
    DLLEXPORT bool RegisterForEvent(CallableObject* toregister,
        const std::string &genericname);
    
    DLLEXPORT void Unregister(CallableObject* caller, EVENT_TYPE type, bool all = false);
    DLLEXPORT void Unregister(CallableObject* caller, const std::string &genericname,
        bool all = false);

private:
    std::vector<std::tuple<CallableObject*, EVENT_TYPE>> EventListeners;
    std::vector<std::tuple<CallableObject*, std::string>> GenericEventListeners;
};

}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::EventHandler;
#endif

