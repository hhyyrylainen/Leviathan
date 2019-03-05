// ------------------------------------ //
#include "CallableObject.h"

#include "Engine.h"
#include "EventHandler.h"

using namespace Leviathan;
// ------------------------------------ //
CallableObject::CallableObject() {}
CallableObject::~CallableObject()
{
    if(HasRegisteredForSomeEvent) {

        // For easier debugging this
        // DEBUG_BREAK;
        LOG_FATAL("Callable object derived class hasn't called UnRegisterAllEvents");
    }
}
// ------------------------------------ //
void CallableObject::UnRegister(EVENT_TYPE from, bool all)
{
    Engine::Get()->GetEventHandler()->Unregister(this, from, all);
}

void CallableObject::UnRegister(const std::string& genericname, bool all /*= false*/)
{
    Engine::Get()->GetEventHandler()->Unregister(this, genericname, all);
}

void CallableObject::RegisterForEvent(EVENT_TYPE toregister)
{
    Engine::Get()->GetEventHandler()->RegisterForEvent(this, toregister);
    HasRegisteredForSomeEvent = true;
}

void CallableObject::RegisterForEvent(const std::string& genericname)
{
    Engine::Get()->GetEventHandler()->RegisterForEvent(this, genericname);
    HasRegisteredForSomeEvent = true;
}

void CallableObject::UnRegisterAllEvents()
{
    Engine::Get()->GetEventHandler()->Unregister(this, EVENT_TYPE_ALL, true);
    Engine::Get()->GetEventHandler()->Unregister(this, "", true);

    HasRegisteredForSomeEvent = false;
}
// ------------------------------------ //
DLLEXPORT EVENT_TYPE CallableObject::ResolveStringToType(const std::string& type)
{
    // lookup map and return //
    auto iter = EventListenerNameToEventMap.find(type);

    if(iter != EventListenerNameToEventMap.end()) {

        return iter->second;
    }

    return EVENT_TYPE_ERROR;
}

DLLEXPORT EVENT_TYPE CallableObject::GetCommonEventType(const std::string& type)
{
    // lookup map and return //
    auto iter = EventListenerCommonNameToEventMap.find(type);

    if(iter != EventListenerCommonNameToEventMap.end()) {

        return iter->second;
    }

    return EVENT_TYPE_ERROR;
}

DLLEXPORT std::string CallableObject::GetListenerNameFromType(EVENT_TYPE type)
{
    for(auto iter = EventListenerNameToEventMap.begin();
        iter != EventListenerNameToEventMap.end(); ++iter) {
        // when the type matches return the string associated with that value//
        if(iter->second == type)
            return iter->first;
    }

    // Also common //
    for(auto iter = EventListenerCommonNameToEventMap.begin();
        iter != EventListenerCommonNameToEventMap.end(); ++iter) {
        // when the type matches return the string associated with that value//
        if(iter->second == type)
            return iter->first;
    }

    return "";
}
