// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Logger.h"

#include "angelscript.h"

#include <map>
#include <string>

#define ANGELSCRIPT_REGISTERFAIL                                    \
    Leviathan::Logger::Get()->Error("AngelScript: Binding failed: " \
                                    "file " __FILE__ " on line " +  \
                                    std::to_string(__LINE__));      \
    return false;

// Maybe this could be avoided by typedefing the right thing to size_t in angelscript
// initialization based on size of size_t
//! Put this anywhere it is assumed that angelscript type uint64 is the same as size_t
#define ANGELSCRIPT_ASSUMED_SIZE_T                    \
    static_assert(sizeof(size_t) == sizeof(uint64_t), \
        "Script register for size_t assumes it is "   \
        "equivalent to uint64_t in angelscript");


namespace Leviathan {

// ------------------ Dynamic cast proxies ------------------ //
template<class From, class To>
To* DoReferenceCastDynamic(From* ptr)
{
    // If already invalid just return it //
    if(!ptr)
        return nullptr;

    To* newptr = dynamic_cast<To*>(ptr);
    if(newptr) {
        // Add reference so script doesn't screw up with the handles //
        newptr->AddRef();
    }

    // Return the ptr (which might be invalid) //
    return newptr;
}

template<class From, class To>
To* DoReferenceCastStatic(From* ptr)
{
    // If already invalid just return it //
    if(!ptr)
        return nullptr;

    To* newptr = static_cast<To*>(ptr);
    if(newptr) {
        // Add reference so script doesn't screw up with the handles //
        newptr->AddRef();
    }

    // Return the ptr (which might be invalid) //
    return newptr;
}

template<class From, class To>
To* DoReferenceCastDynamicNoRef(From* ptr)
{
    // If already invalid just return it //
    if(!ptr)
        return nullptr;

    To* newptr = dynamic_cast<To*>(ptr);

    // Return the ptr (which might be invalid) //
    return newptr;
}

template<class From, class To>
To* DoReferenceCastStaticNoRef(From* ptr)
{
    // If already invalid just return it //
    if(!ptr)
        return nullptr;

    To* newptr = static_cast<To*>(ptr);

    // Return the ptr (which might be invalid) //
    return newptr;
}

// ------------------------------------ //
// Register helpers
#define ANGELSCRIPT_REGISTER_REF_TYPE(RegisterName, ClassName)                     \
    if(engine->RegisterObjectType(RegisterName, 0, asOBJ_REF) < 0) {               \
        ANGELSCRIPT_REGISTERFAIL;                                                  \
    }                                                                              \
                                                                                   \
    if(engine->RegisterObjectBehaviour(RegisterName, asBEHAVE_ADDREF, "void f()",  \
           asMETHOD(ClassName, AddRef), asCALL_THISCALL) < 0) {                    \
        ANGELSCRIPT_REGISTERFAIL;                                                  \
    }                                                                              \
                                                                                   \
    if(engine->RegisterObjectBehaviour(RegisterName, asBEHAVE_RELEASE, "void f()", \
           asMETHOD(ClassName, Release), asCALL_THISCALL) < 0) {                   \
        ANGELSCRIPT_REGISTERFAIL;                                                  \
    }

#define ANGELSCRIPT_REGISTER_ENUM_VALUE(enum, x)                                   \
    {                                                                              \
        if(engine->RegisterEnumValue(#enum, #x, static_cast<int>(enum ::x)) < 0) { \
            ANGELSCRIPT_REGISTERFAIL;                                              \
        }                                                                          \
    }

#define ANGELSCRIPT_REGISTER_ENUM_VALUE_WITH_NAME(enum, valuename, x)             \
    {                                                                             \
        if(engine->RegisterEnumValue(enum, valuename, static_cast<int>(x)) < 0) { \
            ANGELSCRIPT_REGISTERFAIL;                                             \
        }                                                                         \
    }
} // namespace Leviathan

#define ANGLESCRIPT_BASE_CLASS_CASTS(base, base_as, derived, derived_as)                 \
    if(engine->RegisterObjectMethod(base_as, derived_as "@ opCast()",                    \
           asFUNCTION((Leviathan::DoReferenceCastDynamic<base, derived>)),               \
           asCALL_CDECL_OBJFIRST) < 1) {                                                 \
        ANGELSCRIPT_REGISTERFAIL;                                                        \
    }                                                                                    \
    if(engine->RegisterObjectMethod(derived_as, base_as "@ opImplCast()",                \
           asFUNCTION((Leviathan::DoReferenceCastStatic<derived, base>)),                \
           asCALL_CDECL_OBJFIRST) < 1) {                                                 \
        ANGELSCRIPT_REGISTERFAIL;                                                        \
    }                                                                                    \
    if(engine->RegisterObjectMethod(base_as, "const " derived_as "@ opCast() const",     \
           asFUNCTION((Leviathan::DoReferenceCastDynamic<base, derived>)),               \
           asCALL_CDECL_OBJFIRST) < 1) {                                                 \
        ANGELSCRIPT_REGISTERFAIL;                                                        \
    }                                                                                    \
    if(engine->RegisterObjectMethod(derived_as, "const " base_as "@ opImplCast() const", \
           asFUNCTION((Leviathan::DoReferenceCastStatic<derived, base>)),                \
           asCALL_CDECL_OBJFIRST) < 1) {                                                 \
        ANGELSCRIPT_REGISTERFAIL;                                                        \
    }


#define ANGLESCRIPT_BASE_CLASS_CASTS_NO_REF(base, base_as, derived, derived_as)          \
    if(engine->RegisterObjectMethod(base_as, derived_as "@ opCast()",                    \
           asFUNCTION((Leviathan::DoReferenceCastDynamicNoRef<base, derived>)),          \
           asCALL_CDECL_OBJFIRST) < 1) {                                                 \
        ANGELSCRIPT_REGISTERFAIL;                                                        \
    }                                                                                    \
    if(engine->RegisterObjectMethod(derived_as, base_as "@ opImplCast()",                \
           asFUNCTION((Leviathan::DoReferenceCastStaticNoRef<derived, base>)),           \
           asCALL_CDECL_OBJFIRST) < 1) {                                                 \
        ANGELSCRIPT_REGISTERFAIL;                                                        \
    }                                                                                    \
    if(engine->RegisterObjectMethod(base_as, "const " derived_as "@ opCast() const",     \
           asFUNCTION((Leviathan::DoReferenceCastDynamicNoRef<base, derived>)),          \
           asCALL_CDECL_OBJFIRST) < 1) {                                                 \
        ANGELSCRIPT_REGISTERFAIL;                                                        \
    }                                                                                    \
    if(engine->RegisterObjectMethod(derived_as, "const " base_as "@ opImplCast() const", \
           asFUNCTION((Leviathan::DoReferenceCastStaticNoRef<derived, base>)),           \
           asCALL_CDECL_OBJFIRST) < 1) {                                                 \
        ANGELSCRIPT_REGISTERFAIL;                                                        \
    }


#define ANGLESCRIPT_BASE_CLASS_CASTS_NO_REF_STRING(base, base_as, derived, derived_as)        \
    if(engine->RegisterObjectMethod(base_as.c_str(), (derived_as + "@ opCast()").c_str(),     \
           asFUNCTION((Leviathan::DoReferenceCastDynamicNoRef<base, derived>)),               \
           asCALL_CDECL_OBJFIRST) < 1) {                                                      \
        ANGELSCRIPT_REGISTERFAIL;                                                             \
    }                                                                                         \
    if(engine->RegisterObjectMethod(derived_as.c_str(), (base_as + "@ opImplCast()").c_str(), \
           asFUNCTION((Leviathan::DoReferenceCastStaticNoRef<derived, base>)),                \
           asCALL_CDECL_OBJFIRST) < 1) {                                                      \
        ANGELSCRIPT_REGISTERFAIL;                                                             \
    }                                                                                         \
    if(engine->RegisterObjectMethod(base_as.c_str(),                                          \
           ("const " + derived_as + "@ opCast() const").c_str(),                              \
           asFUNCTION((Leviathan::DoReferenceCastDynamicNoRef<base, derived>)),               \
           asCALL_CDECL_OBJFIRST) < 1) {                                                      \
        ANGELSCRIPT_REGISTERFAIL;                                                             \
    }                                                                                         \
    if(engine->RegisterObjectMethod(derived_as.c_str(),                                       \
           ("const " + base_as + "@ opImplCast() const").c_str(),                             \
           asFUNCTION((Leviathan::DoReferenceCastStaticNoRef<derived, base>)),                \
           asCALL_CDECL_OBJFIRST) < 1) {                                                      \
        ANGELSCRIPT_REGISTERFAIL;                                                             \
    }
