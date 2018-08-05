// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Events/CallableObject.h"
#include "Events/EventHandler.h"
#include "Script/NonOwningScriptCallback.h"

#include "Common/ThreadSafe.h"
#include "Exceptions.h"
#include "Script/ScriptExecutor.h"

#include "Engine.h"

namespace Leviathan { namespace Script {

//! \brief An EventListener that scripts can use to listen for events
class EventListener : public ReferenceCounted, public CallableObject, public ThreadSafe {
public:
    EventListener(asIScriptFunction* onevent, asIScriptFunction* ongeneric) :
        OnEventScript(onevent), OnGenericScript(ongeneric)
    {
        // Fail if neither is set //
        if(!OnGenericScript.HasCallback() && !OnEventScript.HasCallback())
            throw InvalidArgument("At least on event or on generic listeners need to be "
                                  "provided, both are null");
    }

    ~EventListener()
    {
        UnRegisterAllEvents();

        OnEventScript.Reset();
        OnGenericScript.Reset();
    }

    int OnEvent(Event* event) override
    {
        GUARD_LOCK();

        if(OnEventScript.HasCallback()) {

            ScriptRunningSetup setup;

            // Run the script //
            return _HandleReturnValue(OnEventScript.Run<int>(setup, event), "OnEventScript");
        }

        return -1;
    }

    int OnGenericEvent(GenericEvent* event) override
    {

        GUARD_LOCK();

        if(OnGenericScript.HasCallback()) {

            ScriptRunningSetup setup;

            // Run the script //
            return _HandleReturnValue(
                OnGenericScript.Run<int>(setup, event), "OnGenericScript");
        }

        return -1;
    }

    inline int _HandleReturnValue(const ScriptRunResult<int>& result, std::string_view which)
    {
        if(result.Result != SCRIPT_RUN_RESULT::Success) {

            LOG_ERROR(
                "ScriptEventListener: failed to call " + std::string(which) + " callback");
            return 0;
        }

        // Return the value returned by the script //
        return result.Value;
    }

    //! \brief Registers for a predefined event type if OnEvent is not NULL
    bool RegisterForEventType(EVENT_TYPE type)
    {
        {
            GUARD_LOCK();

            if(!OnEventScript.HasCallback())
                return false;
        }

        Engine::Get()->GetEventHandler()->RegisterForEvent(this, type);
        return true;
    }

    //! \brief Registers for a generic event if OnGeneric is not NULL
    bool RegisterForEventGeneric(const std::string& name)
    {
        {
            GUARD_LOCK();

            if(!OnGenericScript.HasCallback())
                return false;
        }

        Engine::Get()->GetEventHandler()->RegisterForEvent(this, name);
        return true;
    }

protected:
    // The AngelScript functions to be called //
    NonOwningScriptCallback OnEventScript;
    NonOwningScriptCallback OnGenericScript;
};

EventListener* EventListenerFactory(asIScriptFunction* onevent, asIScriptFunction* ongeneric)
{
    EventListener* listener = nullptr;

    try {

        listener = new EventListener(onevent, ongeneric);

    } catch(const InvalidArgument& e) {

        Logger::Get()->Error("Failed to construct Script::EventListener, exception: ");
        e.PrintToLog();
    }

    if(!listener) {

        // The object won't release the parameter references //
        if(onevent)
            onevent->Release();

        if(ongeneric)
            ongeneric->Release();
    }

    return listener;
}


}} // namespace Leviathan::Script
