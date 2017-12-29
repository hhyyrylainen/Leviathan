#pragma once

//! \file Forward declarations for entity things
#include "Include.h"
#include <inttypes.h>

// ------------------------------------ //


namespace Leviathan{

    using ObjectID = int32_t;

// TODO: start using this everywhere
constexpr ObjectID NULL_OBJECT = 0;
}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::ObjectID;
using Leviathan::NULL_OBJECT;
#endif
