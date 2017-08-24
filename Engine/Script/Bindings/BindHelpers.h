// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "angelscript.h"

#include <map>
#include <string>

#define ANGELSCRIPT_REGISTERFAIL Logger::Get()->Error("AngelScript: Binding failed: " \
"file " __FILE__ " on line " + std::to_string(__LINE__));                             \
 return false;

namespace Leviathan{

// ------------------ Dynamic cast proxies ------------------ //
template<class From, class To>
    To* DoReferenceCastDynamic(From* ptr){
    // If already invalid just return it //
    if(!ptr)
        return nullptr;

    To* newptr = dynamic_cast<To*>(ptr);
    if(newptr){
        // Add reference so script doesn't screw up with the handles //
        newptr->AddRef();
    }

    // Return the ptr (which might be invalid) //
    return newptr;
}

template<class From, class To>
    To* DoReferenceCastStatic(From* ptr){
    // If already invalid just return it //
    if(!ptr)
        return nullptr;

    To* newptr = static_cast<To*>(ptr);
    if(newptr){
        // Add reference so script doesn't screw up with the handles //
        newptr->AddRef();
    }

    // Return the ptr (which might be invalid) //
    return newptr;
}

template<class From, class To>
    To* DoReferenceCastDynamicNoRef(From* ptr){
	// If already invalid just return it //
	if(!ptr)
		return nullptr;

	To* newptr = dynamic_cast<To*>(ptr);

	// Return the ptr (which might be invalid) //
	return newptr;
}

template<class From, class To>
    To* DoReferenceCastStaticNoRef(From* ptr){
	// If already invalid just return it //
	if(!ptr)
		return nullptr;

	To* newptr = static_cast<To*>(ptr);

	// Return the ptr (which might be invalid) //
	return newptr;
}

// ------------------------------------ //
// Register helpers
#define ANGELSCRIPT_REGISTER_REF_TYPE(RegisterName, ClassName)          \
if(engine->RegisterObjectType(RegisterName, 0, asOBJ_REF) < 0){         \
    ANGELSCRIPT_REGISTERFAIL;                                           \
 }                                                                      \
                                                                        \
 if(engine->RegisterObjectBehaviour(RegisterName, asBEHAVE_ADDREF,      \
         "void f()", asMETHOD(ClassName, AddRefProxy),                  \
         asCALL_THISCALL) < 0)                                          \
 {                                                                      \
     ANGELSCRIPT_REGISTERFAIL;                                          \
 }                                                                      \
                                                                        \
 if(engine->RegisterObjectBehaviour(RegisterName, asBEHAVE_RELEASE,     \
         "void f()", asMETHOD(ClassName, ReleaseProxy),                 \
         asCALL_THISCALL) < 0)                                          \
 {                                                                      \
     ANGELSCRIPT_REGISTERFAIL;                                          \
 }                                                                      \

#define ANGELSCRIPT_REGISTER_ENUM_VALUE(enum, x) {if(engine->RegisterEnumValue( \
        #enum, #x, static_cast<int>(enum :: x)) < 0){ANGELSCRIPT_REGISTERFAIL;}}
}


