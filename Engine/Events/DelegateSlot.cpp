// ------------------------------------ //
#include "DelegateSlot.h"


using namespace Leviathan;
// ------------------------------------ //


DLLEXPORT Delegate::Delegate() {}

DLLEXPORT Delegate::~Delegate()
{

    // Allow having pointers to delegates from application, but give errors if used as a value
    // and a script keeps a handle around
    LEVIATHAN_ASSERT(GetRefCount() == 1 || GetRefCount() == 0,
        "Delegate still has active references, scripts "
        "shouldn't store these");
}
// ------------------------------------ //
DLLEXPORT void Delegate::Call(const NamedVars::pointer& values) const
{

    GUARD_LOCK();

    for(const auto& callback : AttachedCallbacks)
        callback->OnCalled(values);
}

DLLEXPORT void Delegate::Call(NamedVars* values) const
{

    GUARD_LOCK();

    for(const auto& callback : AttachedCallbacks)
        callback->OnCalled(values);

    values->Release();
}
// ------------------------------------ //
DLLEXPORT void Delegate::Register(const BaseDelegateSlot::pointer& callback)
{

    GUARD_LOCK();

    AttachedCallbacks.push_back(callback);
}
// ------------------------------------ //
// LambdaDelegateSlot
