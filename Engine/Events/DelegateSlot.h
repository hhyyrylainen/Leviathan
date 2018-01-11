// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"
#include "Common/ReferenceCounted.h"

#include <functional>

namespace Leviathan{

//! \brief Base class for all delegate types
class BaseDelegateSlot : public ReferenceCounted{
public:

    //! \brief Called from Delegate::Call
    virtual void OnCalled(const NamedVars::pointer &values) = 0;

    REFERENCE_COUNTED_PTR_TYPE(BaseDelegateSlot);
};

class LambdaDelegateSlot : public BaseDelegateSlot{
public:

    void OnCalled(const NamedVars::pointer &values) override{
        Callback(values);
    }

protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    
    LambdaDelegateSlot(std::function<void(const NamedVars::pointer &values)> callback) :
        Callback(callback){

    }
    
private:
    
    std::function<void (const NamedVars::pointer &values)> Callback;
};


//! \brief An object which can accept BaseDelegateSlot derived callbacks that
//! can be called when the event represented by this delegate is
//! called
//! \todo Allow unregistering callbacks
//! \note Many objects will have Delegates as their members without pointers so scripts really
//! shouldn't try to store these. unless the specific object supports storing them
class Delegate : public ThreadSafe, public ReferenceCounted{
public:
    
    DLLEXPORT Delegate();
    DLLEXPORT ~Delegate();

    //! \brief Calls all the attached delegates
    //!
    //! \param values The data for the callbacks to receive. 
    //! \todo Find a way to more efficiently pass known types or
    //! data that may not be stored (only copied)
    DLLEXPORT void Call(const NamedVars::pointer &values) const;

    //! \brief Registers a new callback
    DLLEXPORT void Register(const BaseDelegateSlot::pointer &callback);

    //! \brief AngelScript wrapper for Call
    //! \note Decreases reference count
    DLLEXPORT void Call(NamedVars* variables) const;

    REFERENCE_COUNTED_PTR_TYPE(Delegate);    

private:

    std::vector<BaseDelegateSlot::pointer> AttachedCallbacks;
};


}
