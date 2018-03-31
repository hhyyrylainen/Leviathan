// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Event.h"

namespace Leviathan {

//! \todo Rewrite this to be cleaner with the event return codes
class CallableObject {
public:
    DLLEXPORT CallableObject();
    DLLEXPORT virtual ~CallableObject();

    // If you want to keep the pointer to the event call AddRef on it
    DLLEXPORT virtual int OnEvent(Event* event) = 0;
    DLLEXPORT virtual int OnGenericEvent(GenericEvent* event) = 0;

    //! \returns event type from wstring (invalid event type is returned if matches none)
    //! \note if invalid type is returned type should be registered as generic event
    DLLEXPORT static EVENT_TYPE ResolveStringToType(const std::string& type);

    //! \returns event type from string if it is one of the common events that different
    //! objects handle themselves
    //!
    //! not a global event like in ResolveStringToType
    DLLEXPORT static EVENT_TYPE GetCommonEventType(const std::string& type);


    //! reverse of above and returns for example from EVENT_TYPE_SHOW "OnShow"
    //! \todo Have a bi-directional map to speed up lookups
    DLLEXPORT static std::string GetListenerNameFromType(EVENT_TYPE type);

protected:
    //! \warning Unregister all has to be called the first thing
    //! in any derived class constructor otherwise this class'
    //! destructor will assert
    //! \note This object shouldn't be locked while calling this to avoid deadlocking when
    //! unregistering while an event is being processed
    void UnRegisterAllEvents();

    void UnRegister(EVENT_TYPE from, bool all = false);
    void UnRegister(const std::string& genericname, bool all = false);

    void RegisterForEvent(EVENT_TYPE toregister);
    void RegisterForEvent(const std::string& genericname);

private:
    //! Keeps track of whether we have registered for some event. If
    //! we have UnRegisterAllEvents must be called
    bool HasRegisteredForSomeEvent = false;
};

} // namespace Leviathan
