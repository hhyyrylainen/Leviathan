// ------------------------------------ //
#include "NonOwningScriptCallback.h"

using namespace Leviathan;
// ------------------------------------ //

NonOwningScriptCallback::NonOwningScriptCallback(asIScriptFunction* callback /*= nullptr*/)
{
    if(callback)
        SetCallback(callback);
}

NonOwningScriptCallback::~NonOwningScriptCallback()
{
    _ReleaseCallback();
}

void NonOwningScriptCallback::_ReleaseCallback()
{
    if(Callback) {
        Callback->Release();
        Callback = nullptr;
    }

    if(DelegateObjectWeak) {

        DelegateObjectWeak->Release();
        DelegateObjectWeak = nullptr;

        CallbackObject = nullptr;

    } else {
        // Non-weak reference
        if(CallbackObject) {
            CallbackObjectType->GetEngine()->ReleaseScriptObject(
                CallbackObject, CallbackObjectType);
            CallbackObject = nullptr;
        }
    }

    if(CallbackObjectType) {

        CallbackObjectType->Release();
        CallbackObjectType = nullptr;
    }
}
// ------------------------------------ //
void NonOwningScriptCallback::SetCallback(asIScriptFunction* callback)
{
    if(Callback)
        _ReleaseCallback();

    if(!callback)
        return;

    if(callback->GetFuncType() == asFUNC_DELEGATE) {

        asIScriptEngine* engine = callback->GetEngine();

        // Keep a hold of the type
        CallbackObjectType = callback->GetDelegateObjectType();
        CallbackObjectType->AddRef();

        // Hold a weak reference to the object if possible
        CallbackObject = callback->GetDelegateObject();

        DelegateObjectWeak =
            engine->GetWeakRefFlagOfScriptObject(CallbackObject, CallbackObjectType);

        // If not possible to have a weak reference hold a strong reference
        if(!DelegateObjectWeak) {
            LOG_WARNING("NonOwningScriptCallback: SetCallback: passed a delegate of an object "
                        "type that "
                        "doesn't support weak references, this reference won't be non-owning");
            engine->AddRefScriptObject(CallbackObject, CallbackObjectType);
        } else {
            DelegateObjectWeak->AddRef();
        }

        // Hold on to the method strongly
        Callback = callback->GetDelegateFunction();
        Callback->AddRef();

        // And release our reference to the delegate
        callback->Release();

    } else {

        Callback = callback;
    }
}
