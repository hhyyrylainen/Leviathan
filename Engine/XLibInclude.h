#pragma once
#ifdef __linux
#ifndef LEVIATHAN_XINCLUDES
#define LEVIATHAN_XINCLUDES
// ------------------------------------ //
//! \file File that defines Xlib includes and undefines (most) conflicting things
// ---- includes ---- //

#include <X11/Xlib.h>
    
// X11 additional includes
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xmu/Atoms.h>


#undef GenericEvent
//#undef Status
//#undef None
#undef index
// This needs to be up to date with the current Xlib definition
//#define X11CurrentTime 0L
//#undef CurrentTime
#undef min
#undef max
#undef True
#undef False


#endif
#endif //__linux
