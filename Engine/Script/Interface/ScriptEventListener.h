#pragma once
#include "Common/ReferenceCounted.h"
#include "Events/CallableObject.h"
#include "Events/EventHandler.h"

#include "Common/ThreadSafe.h"
#include "Exceptions.h"
#include "Script/ScriptExecutor.h"

#include "Engine.h"

namespace Leviathan { namespace Script {

//! \brief Script class
class EventListener : public ReferenceCounted, public CallableObject, public ThreadSafe {
public:
    EventListener(asIScriptFunction* onevent, asIScriptFunction* ongeneric)
    {

        if(onevent) {

            OnEventScript = onevent;

        } else {

            OnEventScript = NULL;
        }

        if(ongeneric) {

            OnGenericScript = ongeneric;

        } else {

            OnGenericScript = NULL;
        }

        // Fail if neither is set //
        if(!OnGenericScript && !OnEventScript)
            throw InvalidArgument("At least on event or on generic listeners need to be "
                                  "provided, both are NULL");
    }

    ~EventListener()
    {

        UnRegisterAllEvents();

        GUARD_LOCK();

        SAFE_RELEASE(OnEventScript);
        SAFE_RELEASE(OnGenericScript);
    }

    int OnEvent(Event* event) override
    {

        GUARD_LOCK();

        if(OnEventScript) {

            ScriptRunningSetup sargs;

            // Run the script //
            auto result =
                ScriptExecutor::Get()->RunScript<int>(OnGenericScript, nullptr, sargs, event);

            if(result.Result != SCRIPT_RUN_RESULT::Success) {

                LOG_ERROR("ScriptEventListener: failed to call OnGenericScript callback");
                return 0;
            }

            // Return the value returned by the script //
            return result.Value;
        }

        return -1;
    }

    int OnGenericEvent(GenericEvent* event) override
    {

        GUARD_LOCK();

        if(OnGenericScript) {

            // TODO: merge with OnEvent to reduce duplication

            ScriptRunningSetup sargs;

            // Run the script //
            auto result =
                ScriptExecutor::Get()->RunScript<int>(OnGenericScript, nullptr, sargs, event);

            if(result.Result != SCRIPT_RUN_RESULT::Success) {

                LOG_ERROR("ScriptEventListener: failed to call OnGenericScript callback");
                return 0;
            }

            // Return the value returned by the script //
            return result.Value;
        }

        return -1;
    }

    //! \brief Registers for a predefined event type if OnEvent is not NULL
    bool RegisterForEventType(EVENT_TYPE type)
    {

        {
            GUARD_LOCK();

            if(!OnEventScript)
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

            if(!OnGenericScript)
                return false;
        }

        Engine::Get()->GetEventHandler()->RegisterForEvent(this, name);
        return true;
    }

protected:
    // The AngelScript functions to be called //
    asIScriptFunction* OnEventScript;
    asIScriptFunction* OnGenericScript;
};

EventListener* EventListenerFactory(asIScriptFunction* onevent, asIScriptFunction* ongeneric)
{

    EventListener* listener = NULL;

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
