#pragma once
#include "Include.h"

#include "ForwardDeclarations.h"

//! Defines the networking mode
//! In this mode the server sends snapshots of moving entities to all clients which then interpolate
//! between states. Input will not be replicated on all clients. Clients locally simulate their own
//! inputs. Server uses resimulation to simulate clients taking actions in the past.
#define NETWORK_USE_SNAPSHOTS


namespace Leviathan{

    //! Number of milliseconds between engine and world ticks
    static const int TICKSPEED = 50;

    //! \todo Allow this to not be a multiple of TICKSPEED or smaller than it
    static const int INTERPOLATION_TIME  = 100;

    static const double VERSION = LEVIATHAN_VERSION;
    static const std::string VERSIONS = LEVIATHAN_VERSION_ANSIS;

    static const int VERSION_STABLE = LEVIATHAN_VERSION_STABLE;
    static const int VERSION_MAJOR = LEVIATHAN_VERSION_MAJOR;
    static const int VERSION_MINOR = LEVIATHAN_VERSION_MINOR;
    static const int VERSION_PATCH = LEVIATHAN_VERSION_PATCH;

    static const float PI = 3.14159265f;
    static const float DEGREES_TO_RADIANS = PI/180.f;
    static const float EPSILON = 0.00000001f;


	template<class T>
	void SafeReleaser(T* obj){
		SAFE_RELEASE(obj);
	}
	template<class T>
	void SafeReleaseDeleter(T* obj){
		SAFE_RELEASEDEL(obj);
	}
}


#ifdef _MSC_VER

#define DEBUG_BREAK __debugbreak();

#elif defined __linux

// For making SIGINT work as debug break on linux //
#include <signal.h>

#define DEBUG_BREAK { Leviathan::Logger::Get()->Write("DEBUG_BREAK HIT!"); raise(SIGINT); }

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

// This will break everything if it is defined //
#undef index


#include "Logger.h"

