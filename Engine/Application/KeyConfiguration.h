// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Input/Key.h"

#include <atomic>
#include <functional>

namespace Leviathan {

//! \brief Holds key configuration for an application
class KeyConfiguration : public ThreadSafe {
public:
    //! \param configfile The file which is used for saving and loading keys
    DLLEXPORT KeyConfiguration(const std::string& configfile);

    //! Creates an in-memory only configuration
    DLLEXPORT KeyConfiguration();

    DLLEXPORT ~KeyConfiguration();

    //! \brief Loads the defined keys from a file
    //!
    //! the function argument is called to verify that all required keys are defined and it can
    //! add missing keys
    DLLEXPORT bool Init(
        std::function<void(Lock& guard, KeyConfiguration* checkfrom)> functocheck);

    //! Saves all keys if modified
    DLLEXPORT void Release();

    //! \brief Adds a key if one with the name isn't defined
    //! \returns True if added
    DLLEXPORT bool AddKeyIfMissing(
        Lock& guard, const std::string& name, const std::vector<std::string>& defaultkeys);

    //! \brief Marks current keys as changed and that the configuration should be saved to a
    //! file
    //!
    //! Useful during initially checking that all keys exist
    DLLEXPORT void MarkAsChanged();

    //! Saves current keys
    DLLEXPORT void Save(Lock& guard);

    //! \brief Resolves a control key string ("WalkForward") to a key
    DLLEXPORT std::shared_ptr<std::vector<GKey>> ResolveControlNameToKey(
        const std::string& controlkey);

    //! \brief Resolve variant for getting a reference to the vector of keys
    //! \exception InvalidArgument if controlkey not found
    DLLEXPORT const std::vector<GKey>& ResolveControlNameToKeyVector(
        const std::string& controlkey);

    //! \brief Resolve variant for getting the first binding or throwing
    //! \exception InvalidArgument if controlkey not found
    DLLEXPORT GKey ResolveControlNameToFirstKey(const std::string& controlkey);

    //! Checks which configuration key string matches input key
    //! \warning this is quite costly function
    DLLEXPORT std::string ResolveKeyToControlName(const GKey& key);

private:
    //! true if keys have changed
    std::atomic<bool> Marked = false;

    std::string KeyStorageFile;

    //! If true this isn't saved / loaded from a file
    bool InMemory = false;

    //! Not a bidirectional map to allow multiple keys to resolve to the same things
    std::map<std::string, std::shared_ptr<std::vector<GKey>>> KeyConfigurations;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::KeyConfiguration;
#endif
