// ------------------------------------ //
#include "DelegateSlot.h"


using namespace Leviathan;
// ------------------------------------ //


Delegate::Delegate(){

}

Delegate::~Delegate(){

}
// ------------------------------------ //
void Delegate::Call(NamedVars::pointer values) const{

    GUARD_LOCK();

    for(const auto& callback : AttachedCallbacks)
        callback->OnCalled(values);
}
// ------------------------------------ //
void Delegate::Register(BaseDelegateSlot::pointer callback){

    GUARD_LOCK();

    AttachedCallbacks.push_back(std::move(callback));
}

void Delegate::Register(BaseDelegateSlot* callback){

    if(!callback)
        return;

    Register(BaseDelegateSlot::pointer(callback));
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
