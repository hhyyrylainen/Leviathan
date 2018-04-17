// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Handlers/IDFactory.h"

namespace Leviathan { namespace GUI {

//! \brief Base class for all objects that support having a JavaScript proxy of them created in
//! the render process
class JSProxyable : public ReferenceCounted {
public:
    using pointer = boost::intrusive_ptr<JSProxyable>;

    inline JSProxyable(int id = IDFactory::GetID()) : ID(id) {}
    virtual ~JSProxyable() {}

    inline int GetID() const
    {
        return ID;
    }

protected:
    const int ID;
};


}} // namespace Leviathan::GUI
