// ------------------------------------ //
#include "DelegateSlot.h"


using namespace Leviathan;
// ------------------------------------ //


DLLEXPORT Delegate::Delegate(){

}

DLLEXPORT Delegate::~Delegate(){

    LEVIATHAN_ASSERT(GetRefCount() == 1, "Delegate still has active references, scripts "
        "shouldn't store these");
}
// ------------------------------------ //
DLLEXPORT void Delegate::Call(const NamedVars::pointer &values) const{

    GUARD_LOCK();

    for(const auto& callback : AttachedCallbacks)
        callback->OnCalled(values);
}

DLLEXPORT void Delegate::Call(NamedVars* values) const{

    GUARD_LOCK();

    for(const auto& callback : AttachedCallbacks)
        callback->OnCalled(values);

    values->Release();
}
// ------------------------------------ //
DLLEXPORT void Delegate::Register(const BaseDelegateSlot::pointer &callback){

    GUARD_LOCK();

    AttachedCallbacks.push_back(callback);
}
// ------------------------------------ //
// LambdaDelegateSlot
