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

    REFERENCE_COUNTED_PTR_TYPE(BaseDelegateSlot);

    //! \brief Called from Delegate::Call
    virtual void OnCalled(const NamedVars::pointer &values) = 0;
    
};

class LambdaDelegateSlot : public BaseDelegateSlot{
public:

    LambdaDelegateSlot(std::function<void (const NamedVars::pointer &values)> callback);

    void OnCalled(const NamedVars::pointer &values) override;

private:
    
    std::function<void (const NamedVars::pointer &values)> Callback;
};


//! \brief An object which can accept BaseDelegateSlot derived callbacks that
//! can be called when the event represented by this delegate is
//! called
//! \todo Allow unregistering callbacks
class Delegate : public ThreadSafe{
public:

    Delegate();
    ~Delegate();

    //! \brief Calls all the attached delegates
    //!
    //! \param values The data for the callbacks to receive. This
    //! method expects the caller to have increased the refcount
    //! \todo Find a way to more efficiently pass known types or
    //! data that may not be stored (only copied)
    void Call(NamedVars::pointer values) const;

    //! \brief Registers a new callback
    void Register(BaseDelegateSlot::pointer callback);

    //! \brief AngelScript wrapper for Register
    //! \note This increases the reference count
    void Register(BaseDelegateSlot* callback);


private:

    std::vector<BaseDelegateSlot::pointer> AttachedCallbacks;
};




}
