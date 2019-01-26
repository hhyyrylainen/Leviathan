// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"
#include "Common/ThreadSafe.h"
#include <boost/function.hpp>


namespace Leviathan {

#define GAMECONFIGURATION_GET_VARIABLEACCESS(x)                  \
    NamedVars* x = NULL;                                         \
    Lock varlockaccess;                                          \
    {                                                            \
        GameConfiguration* configvar = GameConfiguration::Get(); \
        if(configvar != NULL) {                                  \
            varlockaccess = Lock(configvar->ObjectsLock);        \
            x = configvar->AccessVariables(varlockaccess);       \
        }                                                        \
    }

class GameConfiguration : public ThreadSafe {
public:
    DLLEXPORT GameConfiguration(const std::string& configfile);

    //! Creates an in-memory only configuration
    DLLEXPORT GameConfiguration();
    DLLEXPORT ~GameConfiguration();


    //! This function loads the files from the defined file and calls the argument
    //! function afterwards to verify that all requires values are set
    DLLEXPORT bool Init(
        std::function<void(Lock& guard, GameConfiguration* configobj)> sanitycheckcallback);

    //! Tries to save the changes
    DLLEXPORT void Release();

    //! Saves current values (if marked as unsaved)
    DLLEXPORT void SaveCheck();

    //! Gets the values
    //! \note you need to have locked this object while and after calling this
    //! \pre Call GAMECONFIGURATION_GET_VARIABLEACCESS(variables); to use this
    DLLEXPORT NamedVars* AccessVariables(Lock& guard);

    //! Call this when you have changed variables
    DLLEXPORT void MarkModified(Lock& guard);

    //! Verifies that the global default values are added properly
    //! \note This doesn't need to be called manually as it is called by Init
    DLLEXPORT void VerifyGlobalVariables(Lock& guard);

    DLLEXPORT static GameConfiguration* Get();

private:
    std::string GameConfigFile;

    // This stores all the values and is passed around from this object to reduce bloat //
    NamedVars* GameVars = nullptr;
    // Controls when to save changes //
    bool Modified = false;

    //! If true this isn't saved / loaded from a file
    bool InMemory = false;

    static GameConfiguration* staticaccess;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::GameConfiguration;
#endif
