// ------------------------------------ //
#include "DelegateSlot.h"


using namespace Leviathan;
// ------------------------------------ //


Delegate::Delegate(){

}

Delegate::~Delegate(){

}
// ------------------------------------ //
void Delegate::Call(const NamedVars::pointer &values) const{

    GUARD_LOCK();

    for(const auto& callback : AttachedCallbacks)
        callback->OnCalled(values);
}

void Delegate::Call(NamedVars* values) const{

    GUARD_LOCK();

    for(const auto& callback : AttachedCallbacks)
        callback->OnCalled(values);

    values->Release();
}
// ------------------------------------ //
void Delegate::Register(const BaseDelegateSlot::pointer &callback){

    GUARD_LOCK();

    AttachedCallbacks.push_back(callback);
}
// ------------------------------------ //
// LambdaDelegateSlot
LambdaDelegateSlot::LambdaDelegateSlot(
    std::function<void (const NamedVars::pointer &values)> callback) :
    Callback(callback)
{
}

void LambdaDelegateSlot::OnCalled(const NamedVars::pointer &values){

    Callback(values);
}
