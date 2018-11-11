#pragma once

#include "Define.h"

#ifdef LEVIATHAN_FULL
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include "OgreColourValue.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#endif // LEVIATHAN_USING_OGRE

// Uncomment for debugging and major slow downs
// #define CHECK_FOR_NANS

#ifdef CHECK_FOR_NANS
#define DO_NAN_CHECK    \
    {                   \
        CheckForNans(); \
    }
#else
#define DO_NAN_CHECK
#endif // CHECK_FOR_NANS
