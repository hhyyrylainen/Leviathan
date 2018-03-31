// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Events/EventableScriptObject.h"
#include "Script/ScriptModule.h"
// Required for template based script running
#include "Script/ScriptExecutor.h"

namespace Leviathan {

//! \brief Represents a scriptable part of a program
class GameModule : public ReferenceCounted, public EventableScriptObject {
public:
    //! \warning Quite expensive constructor since it loads the definition file
    //! \todo Make load all source files, instead of loading just the first
    DLLEXPORT GameModule(const std::string& modulename, const std::string& ownername,
        const std::string& extension = "txt|levgm");

    DLLEXPORT ~GameModule();

    //! \brief Makes the scripts usable
    DLLEXPORT bool Init();

    //! \brief Releases the script
    //!
    //! Use to release script before releasing any other objects
    DLLEXPORT void ReleaseScript();

    //! \returns The script module
    DLLEXPORT inline auto GetScriptModule()
    {
        return ScriptMain;
    }

    // Used to actually use the module //

    //! Executes something on the module and returns the result. Adds the module as first
    //! parameter and existed is set to true if something was executed
    //! \parameter pasself If true this object is added as the first parameter
    template<typename ReturnT, class... Args>
    ScriptRunResult<ReturnT> ExecuteOnModule(
        ScriptRunningSetup& setup, bool passself, Args&&... args)
    {
        if(!Scripting)
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);

        if(passself) {
            return ScriptExecutor::Get()->RunScript<ReturnT>(
                Scripting->GetModuleSafe(), setup, this, std::forward<Args>(args)...);
        } else {
            return ScriptExecutor::Get()->RunScript<ReturnT>(
                Scripting->GetModuleSafe(), setup, std::forward<Args>(args)...);
        }
    }


    //! \brief Returns a string describing this module
    DLLEXPORT std::string GetDescription(bool full);

    REFERENCE_COUNTED_PTR_TYPE(GameModule);

protected:
    virtual ScriptRunResult<int> _DoCallWithParams(
        ScriptRunningSetup& sargs, Event* event, GenericEvent* event2) override;

private:
    AccessFlags ExtraAccess = 0;

    std::string OwnerName;
    std::string LoadedFromFile;

    std::shared_ptr<ScriptModule> ScriptMain;

    // Properties loaded from the file //
    std::string Name;
    std::string Version;
    std::vector<std::string> SourceFiles;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::GameModule;
#endif
