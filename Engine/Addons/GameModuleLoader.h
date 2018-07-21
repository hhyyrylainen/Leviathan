// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GameModule.h"

#include "Common/ThreadSafe.h"

namespace Leviathan {

//! \brief Manages loading GameModule objects to make sure that each is loaded only once
//! \note This is ThreadSafeRecursive to allow the loaded modules to load their required
//! modules without deadlocking
//! \todo Module unloading needs some trick
class GameModuleLoader : public ThreadSafeRecursive {
    friend GameModule;

public:
	DLLEXPORT GameModuleLoader();
	DLLEXPORT ~GameModuleLoader();

    //! \brief Finds the names of all modules and which files they are in. This is safe to call
    //! again if changes to module files are detected
	DLLEXPORT void Init();

    //! \brief Returns a module by name
    //!
    //! This returns a loaded module or loads a module with the given name (or file name)
    //! \param requiredby Is used for debugging the loading hierarchies of modules. You should
    //! use an unique string here. This is taken as a const char to avoid constructing unused
    //! string objects \exception NotFound if not found
    //! \todo Allow loading by relative path (right now must just have the exact module name)
    DLLEXPORT GameModule::pointer Load(const std::string& modulename, const char* requiredby);

protected:
    //! \brief Used by GameModule to report that it is going to be deleted
    void GameModuleReportDestruction(GameModule& module);

private:
    bool _LoadInfoFromModuleFile(
        const std::string& file, std::string& modulename, std::string& errorstring);

    //! \exception NotFound on error
    GameModule::pointer _LoadModuleFromFile(
        const std::string& filename, const char* requiredby);

private:
    std::map<std::string, std::string> ModuleNameToPath;

    // This is a plain pointer to allow GameModules to be deleted when only us have a reference
    // to it
    std::map<std::string, GameModule*> LoadedModules;
};

} // namespace Leviathan
