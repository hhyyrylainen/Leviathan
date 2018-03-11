// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Script/AccessMask.h"

namespace Leviathan { namespace GUI {

struct ExtraParameters{

    //! Sets the extra access flags for any script modules created for the GUI object
    AccessFlags ExtraAccess = 0;
};

}}
