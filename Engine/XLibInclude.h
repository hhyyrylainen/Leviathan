#pragma once
#ifdef __linux__
// ------------------------------------ //
//! \file
//! File that defines Xlib includes and undefines (most) conflicting things

// ---- includes ---- //
#include <X11/Xlib.h>

// X11 additional includes
#include <X11/Xatom.h>
#include <X11/Xlibint.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <X11/extensions/Xfixes.h>

#undef max
#undef min
#undef index

#endif //__linux__
