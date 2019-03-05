// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "AutoUpdateable.h"
#include "CallableObject.h"
#include "Script/ScriptModule.h"
#include "Script/ScriptScript.h"


namespace Leviathan {

class EventableScriptObject : public AutoUpdateableObject, public CallableObject {
public:
    DLLEXPORT EventableScriptObject(std::shared_ptr<ScriptScript> script);
    DLLEXPORT virtual ~EventableScriptObject();

    //! \brief Registers for generic events
    //!
    //! Call GetListOfListeners on ScriptModule to get the data
    DLLEXPORT void RegisterStandardScriptEvents(
        const std::vector<std::shared_ptr<ValidListenerData>>& data);

    DLLEXPORT virtual int OnEvent(Event* event) override;
    DLLEXPORT virtual int OnGenericEvent(GenericEvent* event) override;
    DLLEXPORT virtual bool OnUpdate(
        const std::shared_ptr<NamedVariableList>& updated) override;

protected:
    //! Used by callbacks to call the script for handling
    //! \todo Allow return values from here
    //!
    //! By default uses Scripting and retrieves functions by name
    //! \todo Store the callbacks as asIScriptFunction instead of by name for performance
    //! \returns The value from scripts. -1 for a global event signals disconnecting. 1 having
    //! handled it and 0 that it was ignored. For specific object events the return value may
    //! have other meanings
    virtual int _CallScriptListener(Event* event, GenericEvent* event2);

    //! Needed to pass the derived type to the callback
    //!
    //! \example \code{cpp}
    //!     if(event)
    //! return ScriptExecutor::Get()->RunScript<int>(
    //!    Scripting->GetModuleSafe(), sargs, this, event);
    //! else
    //!    return ScriptExecutor::Get()->RunScript<int>(
    //!        Scripting->GetModuleSafe(), sargs, this, event2);
    virtual ScriptRunResult<int> _DoCallWithParams(
        ScriptRunningSetup& sargs, Event* event, GenericEvent* event2) = 0;

    // ------------------------------------ //

    // Stores the script reference //
    std::shared_ptr<ScriptScript> Scripting;
};

} // namespace Leviathan
