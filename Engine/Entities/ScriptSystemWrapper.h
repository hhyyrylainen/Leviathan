// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
//! \file Wraps an angelscript object for use as a script defined system by GameWorld
// ------------------------------------ //

class asIScriptObject;
class asIScriptFunction;

namespace Leviathan {

class ScriptSystemWrapper {
public:
    //! \note Doesn't increase reference on impl so caller needs to
    //! make sure that it is increased
    DLLEXPORT ScriptSystemWrapper(const std::string& name, asIScriptObject* impl);
    DLLEXPORT ~ScriptSystemWrapper();

    // Wrap this in a std::unique_ptr
    ScriptSystemWrapper(const ScriptSystemWrapper& other) = delete;
    ScriptSystemWrapper& operator=(const ScriptSystemWrapper& other) = delete;

    // These functions are proxied to the script object
    // Use these similarly to normal systems
    DLLEXPORT void Init(GameWorld* world);
    //! \note This also clears ImplementationObject ptr.
    DLLEXPORT void Release();
    
    DLLEXPORT void Run();

    DLLEXPORT void CreateAndDestroyNodes();

    DLLEXPORT void Clear();
    
protected:
    //! Helper for reducing copy pasting between the functions that don't need extra parameters
    DLLEXPORT bool _CallMethodOnUs(const std::string &methodname);

    
private:
    //! This is the actual implementation of this system in angelscript
    //! This is reference counted so make sure to release the reference
    asIScriptObject* ImplementationObject;

    // Cached methods for performance reasons
    asIScriptFunction* RunMethod = nullptr;
    asIScriptFunction* CreateAndDestroyNodesMethod = nullptr;
};

} // namespace Leviathan
