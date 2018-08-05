// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "ScriptExecutor.h"

namespace Leviathan {

//! \brief A helper for keeping a weak reference to a script delegate or function.
//!
//! Useful for breaking circular references without implementing container semantics
class NonOwningScriptCallback {
public:
    NonOwningScriptCallback(asIScriptFunction* callback = nullptr);
    ~NonOwningScriptCallback();

    //! \brief Sets the bound function / delegate
    void SetCallback(asIScriptFunction* callback);

    //! \note setup is mainly ignored, only error reporting properties are used
    //! \todo It might be possible to just keep this locked while calling the method
    //! instead of increasing and decrementing the reference count
    template<typename ReturnT, class... Args>
    ScriptRunResult<ReturnT> Run(ScriptRunningSetup& setup, Args&&... args)
    {
        if(!Callback)
            return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);

        // Normal function
        if(!CallbackObject) {
            return ScriptExecutor::Get()->RunScript<ReturnT>(
                Callback, nullptr, setup, std::forward<Args>(args)...);
        } else {
            // Delegate
            if(DelegateObjectWeak) {

                // Must ensure that the object will be alive.

                // To atomically get a reference to the object we must lock the weak flag
                DelegateObjectWeak->Lock();
                if(!DelegateObjectWeak->Get()) {
                    asIScriptEngine* engine = CallbackObjectType->GetEngine();

                    // It might be possible to just keep this locked while calling the method
                    // instead of increasing and decrementing the reference count
                    engine->AddRefScriptObject(CallbackObject, CallbackObjectType);

                    DelegateObjectWeak->Unlock();


                    const auto result = ScriptExecutor::Get()->RunScriptMethod<ReturnT>(
                        setup, Callback, CallbackObject, std::forward<Args>(args)...);

                    // Release the reference we added
                    engine->ReleaseScriptObject(CallbackObject, CallbackObjectType);

                    return result;
                }

                DelegateObjectWeak->Unlock();

                LOG_WARNING("NonOwningScriptCallback: delegate object is no longer alive. "
                            "Resetting state");
                _ReleaseCallback();

                return ScriptRunResult<ReturnT>(SCRIPT_RUN_RESULT::Error);

            } else {

                // Strong refernce is used
                return ScriptExecutor::Get()->RunScriptMethod<ReturnT>(
                    setup, Callback, CallbackObject, std::forward<Args>(args)...);
            }
        }
    }

    inline bool HasCallback() const
    {
        return Callback != nullptr;
    }

    inline void Reset()
    {
        _ReleaseCallback();
    }

private:
    void _ReleaseCallback();

private:
    asIScriptFunction* Callback = nullptr;
    // Delegate support
    void* CallbackObject = nullptr;

    //! This is only required for releasing references to the CallbackObject
    asITypeInfo* CallbackObjectType = nullptr;

    //! Weak reference support for delegate objects
    asILockableSharedBool* DelegateObjectWeak = nullptr;
};

} // namespace Leviathan
