// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
#include "Include.h"

#include "ForwardDeclarations.h"
#include "Entities/EntityCommon.h"

//! Defines the networking mode
//! In this mode the server sends snapshots of moving entities to all clients which then interpolate
//! between states. Input will not be replicated on all clients. Clients locally simulate their own
//! inputs. Server uses resimulation to simulate clients taking actions in the past.
#define NETWORK_USE_SNAPSHOTS

#include <string>

namespace Leviathan{

    //! Number of milliseconds between engine and world ticks
    static const int TICKSPEED = 50;

    //! \todo Allow this to not be a multiple of TICKSPEED or smaller than it
    static const int INTERPOLATION_TIME  = 100;
    
    //! For checking vector normalization
    static const float NORMALIZATION_TOLERANCE = 1e-6f;

#ifdef LEVIATHAN_VERSION
    static const double VERSION = LEVIATHAN_VERSION;
    static const std::string VERSIONS = LEVIATHAN_VERSION_ANSIS;

    static const int VERSION_STABLE = LEVIATHAN_VERSION_STABLE;
    static const int VERSION_MAJOR = LEVIATHAN_VERSION_MAJOR;
    static const int VERSION_MINOR = LEVIATHAN_VERSION_MINOR;
    static const int VERSION_PATCH = LEVIATHAN_VERSION_PATCH;
#endif //LEVIATHAN_VERSION

#ifndef PI
    static const float PI = 3.14159265f;
#endif //PI
    static const float DEGREES_TO_RADIANS = PI/180.f;
    static const float EPSILON = 0.00000001f;
}

// Logging macros //
LOG_INFO(x) Logger::Get()->Info(x);
LOG_WARNING(x) Logger::Get()->Warning(x);
LOG_ERROR(x) Logger::Get()->Error(x);
LOG_WRITE(x) Logger::Get()->Write(x);


#ifdef _MSC_VER

#ifndef DEBUG_BREAK
#define DEBUG_BREAK __debugbreak();
#endif //DEBUG_BREAK

#elif defined __linux

// For making SIGINT work as debug break on linux //
#include <signal.h>
#ifndef DEBUG_BREAK
#define DEBUG_BREAK { Leviathan::Logger::Get()->Write("DEBUG_BREAK HIT!"); raise(SIGINT); }
#endif //DEBUG_BREAK

#else

#error "Debug break won't work"

#endif

#define SAFE_RELEASE( x ) {if(x){(x)->Release();(x)=NULL;}}
#define SAFE_RELEASEDEL( x ) {if(x){(x)->Release();delete (x);(x)=NULL;}}
#define SAFE_DELETE( x ) {if(x){delete (x);(x)=NULL;}}
#define SAFE_DELETE_ARRAY( x ) {if(x){delete[] (x);(x)=NULL;}}

#define SAFE_RELEASE_VECTOR(x) {for(auto iter = x.begin(); iter != x.end(); ++iter) if(*iter){ (*iter)->Release(); } \
        x.clear();}

#define SAFE_DELETE_VECTOR(x) for(size_t vdind = 0; vdind < x.size(); ++vdind){if(x[vdind]){delete x[vdind];}}; \
    x.clear();


#ifdef LEVIATHAN_FULL
#include "Logger.h"
#endif // LEVIATHAN_FULL

