// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
//! \file Wraps an angelscript object for use as a script defined system by GameWorld
// ------------------------------------ //

class asIScriptObject;
class asIScriptFunction;
class CScriptArray;

namespace Leviathan {

class GameWorld;

//! \brief Holds a single component type from c++ or from script, which a ScriptSystem uses
struct ScriptSystemUses {

    ScriptSystemUses(const std::string& name) : Name(name), UsesName(true) {}
    ScriptSystemUses(uint16_t type) : Type(type), UsesName(false) {}
    ScriptSystemUses() : Name("invalid"), UsesName(true) {}
    ~ScriptSystemUses() {}

    ScriptSystemUses& operator=(const ScriptSystemUses& other)
    {
        Type = other.Type;
        Name = other.Name;
        UsesName = other.UsesName;
        return *this;
    }

    static constexpr auto ANGELSCRIPT_TYPE = "ScriptSystemUses";

    uint16_t Type = -1;
    std::string Name;
    bool UsesName = false;
};

//! \brief Helper for script systems to call to properly handle added and removed nodes
DLLEXPORT void ScriptSystemNodeHelper(GameWorld* world, void* cachedcomponents,
    int cachedtypeid, CScriptArray& systemcomponents);

//! \brief Wraps an AngelScript object that is an implementation of ScriptSystem
class ScriptSystemWrapper {
public:
    //! \note Doesn't increase reference on impl so caller needs to
    //! make sure that it is increased
    DLLEXPORT ScriptSystemWrapper(const std::string& name, asIScriptObject* impl);
    DLLEXPORT ~ScriptSystemWrapper();

    // Wrap this in a std::unique_ptr
    ScriptSystemWrapper(const ScriptSystemWrapper& other) = delete;
    ScriptSystemWrapper& operator=(const ScriptSystemWrapper& other) = delete;

    //! \brief Returns the ImplementationObject increasing refcount
    DLLEXPORT asIScriptObject* GetASImplementationObject();

    // These functions are proxied to the script object
    // Use these similarly to normal systems
    DLLEXPORT void Init(GameWorld* world);
    //! \note This also clears ImplementationObject ptr.
    DLLEXPORT void Release();

    DLLEXPORT void Run();

    DLLEXPORT void CreateAndDestroyNodes();

    DLLEXPORT void Clear();

    DLLEXPORT void Suspend();

    DLLEXPORT void Resume();


    const std::string Name;

protected:
    //! Helper for reducing copy pasting between the functions that don't need extra parameters
    DLLEXPORT bool _CallMethodOnUs(const std::string& methodname);

    //! Must be called to not leak the cached function pointers (RunMethod,
    //! CreateAndDestroyNodesMethod)
    DLLEXPORT void _ReleaseCachedFunctions();

private:
    //! This is the actual implementation of this system in angelscript
    //! This is reference counted so make sure to release the reference
    asIScriptObject* ImplementationObject;

    // Cached methods for performance reasons
    asIScriptFunction* RunMethod = nullptr;
    asIScriptFunction* CreateAndDestroyNodesMethod = nullptr;
};

} // namespace Leviathan
