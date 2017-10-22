#pragma once

//! \file Forward declarations for entity things
#include "Include.h"
#include <inttypes.h>

// ------------------------------------ //


namespace Leviathan{

    using ObjectID = int32_t;

}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::ObjectID;
#endif
