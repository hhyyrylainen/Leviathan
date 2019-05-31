// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Include.h"

#include "Entities/EntityCommon.h"
#include "ForwardDeclarations.h"

//! Defines the networking mode In this mode the server sends
//! snapshots of moving entities to all clients which then interpolate
//! between states. Input will not be replicated on all
//! clients. Clients locally simulate their own inputs. Server uses
//! resimulation to simulate clients taking actions in the past.
#define NETWORK_USE_SNAPSHOTS

#include <string>

namespace Leviathan {

//! Number of milliseconds between engine and world ticks
constexpr auto TICKSPEED = 50;

//! \todo Allow this to not be a multiple of TICKSPEED or smaller than it
constexpr auto INTERPOLATION_TIME = 100;

//! Defines the interval between heartbeats
//! Should be the same as CLIENT_HEARTBEATS_MILLISECOND
constexpr auto HEARTBEATS_MILLISECOND = 180;

constexpr auto DEFAULT_MAXCONNECT_TRIES = 5;

constexpr auto MAX_SERVERCOMMAND_LENGTH = 550;

constexpr auto BASESENDABLE_STORED_RECEIVED_STATES = 6;

constexpr auto PACKET_LOST_AFTER_MILLISECONDS = 1000;

constexpr auto PACKET_LOST_AFTER_RECEIVED_NEWER = 4;

constexpr auto CRITICAL_PACKET_MAX_TRIES = 8;

//! For checking vector normalization
constexpr float NORMALIZATION_TOLERANCE = 1e-6f;

#ifdef LEVIATHAN_VERSION
constexpr double VERSION = LEVIATHAN_VERSION;
static const std::string VERSIONS = LEVIATHAN_VERSION_ANSIS;

constexpr int VERSION_STABLE = LEVIATHAN_VERSION_STABLE;
constexpr int VERSION_MAJOR = LEVIATHAN_VERSION_MAJOR;
constexpr int VERSION_MINOR = LEVIATHAN_VERSION_MINOR;
constexpr int VERSION_PATCH = LEVIATHAN_VERSION_PATCH;
#endif // LEVIATHAN_VERSION

constexpr auto MICROSECONDS_IN_SECOND = 1000000;

#ifndef PI
constexpr float PI = 3.14159265f;
#endif // PI
constexpr float DEGREES_TO_RADIANS = PI / 180.f;
constexpr float RADIANS_TO_DEGREES = 180.f / PI;
constexpr float EPSILON = 0.00000001f;
} // namespace Leviathan

// This is here until bsf implements separate scenes
namespace bs {
using Scene = int32_t;
}


// AngelScript type registration (that isn't ReferenceCounted)
#ifdef LEVIATHAN_USING_ANGELSCRIPT
#define REFERENCE_HANDLE_UNCOUNTED_TYPE(x) static constexpr auto ANGELSCRIPT_TYPE = #x "@";
// Appended @ because these are handle types
#define REFERENCE_HANDLE_UNCOUNTED_TYPE_NAMED(x, y) \
    static constexpr auto ANGELSCRIPT_TYPE = #y "@";
#define VALUE_TYPE(x) static constexpr auto ANGELSCRIPT_TYPE = #x;
#define VALUE_TYPE_NAMED(x, y) static constexpr auto ANGELSCRIPT_TYPE = #y;
#else
#define REFERENCE_HANDLE_UNCOUNTED_TYPE(x)
#define REFERENCE_HANDLE_UNCOUNTED_TYPE_NAMED(x, y)
#define VALUE_TYPE(x)
#define VALUE_TYPE_NAMED(x)
#endif // LEVIATHAN_USING_ANGELSCRIPT

// Logging macros //
#define LOG_INFO(x) Logger::Get()->Info(x);
#define LOG_WARNING(x) Logger::Get()->Warning(x);
#define LOG_ERROR(x) Logger::Get()->Error(x);
#define LOG_WRITE(x) Logger::Get()->Write(x);
#define LOG_FATAL(x) \
    Logger::Get()->Fatal(x + (", at: " __FILE__ "(" + std::to_string(__LINE__) + ")"));

// Assertions for controlled crashing
#ifndef LEVIATHAN_ASSERT
#include <stdlib.h>
#define LEVIATHAN_ASSERT(x, msg) \
    {                            \
        if(!(x)) {               \
            LOG_FATAL(msg);      \
            abort();             \
        }                        \
    };
#endif // LEVIATHAN_ASSERT

#ifdef _MSC_VER

#ifndef DEBUG_BREAK
#define DEBUG_BREAK __debugbreak();
#endif // DEBUG_BREAK

#elif defined __linux

// For making SIGINT work as debug break on linux //
#include <signal.h>
#ifndef DEBUG_BREAK
#define DEBUG_BREAK                                               \
    {                                                             \
        LOG_WRITE("DEBUG_BREAK HIT! at:");                        \
        LOG_WRITE(__FILE__ "(" + std::to_string(__LINE__) + ")"); \
        ::raise(SIGINT);                                          \
    }
#endif // DEBUG_BREAK

#else

#error "Debug break won't work"

#endif

#define SAFE_RELEASE(x)     \
    {                       \
        if(x) {             \
            (x)->Release(); \
            (x) = NULL;     \
        }                   \
    }
#define SAFE_RELEASEDEL(x)  \
    {                       \
        if(x) {             \
            (x)->Release(); \
            delete(x);      \
            (x) = NULL;     \
        }                   \
    }
#define SAFE_DELETE(x)  \
    {                   \
        if(x) {         \
            delete(x);  \
            (x) = NULL; \
        }               \
    }
#define SAFE_DELETE_ARRAY(x) \
    {                        \
        if(x) {              \
            delete[](x);     \
            (x) = NULL;      \
        }                    \
    }

#define SAFE_RELEASE_VECTOR(x)                              \
    {                                                       \
        for(auto iter = x.begin(); iter != x.end(); ++iter) \
            if(*iter) {                                     \
                (*iter)->Release();                         \
            }                                               \
        x.clear();                                          \
    }

#define SAFE_DELETE_VECTOR(x)                          \
    for(size_t vdind = 0; vdind < x.size(); ++vdind) { \
        if(x[vdind]) {                                 \
            delete x[vdind];                           \
        }                                              \
    };                                                 \
    x.clear();

#define UNUSED(x) ((void)x);

#include "Logger.h"
