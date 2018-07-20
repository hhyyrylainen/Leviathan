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

class GameModuleLoader;

//! \brief Represents a scriptable part of a program
//! \note Create instances through GameModuleLoader::Load
class GameModule : public ReferenceCounted, public EventableScriptObject {
    friend GameModuleLoader;
    // This is to make MakeShared work, but it should only be called from GameModuleLoader
    friend ReferenceCounted;

protected:
    DLLEXPORT GameModule(const std::string& filepath, const std::string& ownername,
        GameModuleLoader* loadedthrough);

    //! \brief Makes the scripts usable
    DLLEXPORT bool Init();

public:
    DLLEXPORT ~GameModule();

    //! \brief Releases the script
    //!
    //! Use to release script before releasing any other objects
    //! \note GameModuleLoader will call this automatically but this is provided to allow
    //! controlling the release order
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


    //! \returns A string describing this module
    DLLEXPORT std::string GetDescription(bool full);

    //! \returns The name of this module
    DLLEXPORT const auto& GetName() const
    {
        return Name;
    }

    //! \returns The list of export files from this module
    DLLEXPORT const auto& GetExportFiles() const
    {
        return ExportFiles;
    }

    //! \brief Used to detect circular dependencies by GameModuleLoader
    DLLEXPORT bool IsInitializing() const
    {
        return IsCurrentlyInitializing;
    }

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
    std::vector<std::string> ImportModules;
    std::vector<std::string> ExportFiles;


    //! This is used to work with GameModuleLoader without it holding a reference, in order to
    //! work like a weak reference (this is uses boost::intrusive_ptr so weak_ptr isn't
    //! supported)
    GameModuleLoader* Loader;

    //! This is used to detect circular loading of modules
    bool IsCurrentlyInitializing = false;

    //! Loaded modules that this depends on. The list is defined in ImportModules
    std::vector<GameModule::pointer> LoadedImportedModules;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::GameModule;
#endif
