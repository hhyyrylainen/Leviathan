// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

// Forward declare things for type resolver specification
namespace bs {
class Matrix4;
class Color;
} // namespace bs

class NewtonCollision;

class asIScriptFunction;
class asIScriptObject;

namespace Leviathan {

class ScriptExecutor;

//! This has to be constant (and luckily so far it has been)
constexpr auto ANGELSCRIPT_VOID_TYPEID = 0;

//! Proxy for stuff that wants to only include this file
DLLEXPORT ScriptExecutor* GetCurrentGlobalScriptExecutor();

//! Needed as ScriptExecutor depends on AngelScriptTypeIDResolver
//! and that depends on it
DLLEXPORT int ResolveProxy(const char* type, ScriptExecutor* resolver, bool constversion);

//! Converts type to AngelScript type string
template<class T>
struct TypeToAngelScriptTypeString {
    static constexpr const char* Type()
    {
        if constexpr(std::is_pointer_v<T>) {
            return std::remove_pointer_t<T>::ANGELSCRIPT_TYPE;
        } else {

            return T::ANGELSCRIPT_TYPE;
        }
    }
};

//! Helper for querying each type for their corresponding angelscript type once
//! \todo This will entirely break if there can be multiple script executors in use
template<class T>
struct AngelScriptTypeIDResolver {
public:
    static int Get(ScriptExecutor* resolver)
    {
        static int cached = ResolveProxy(TypeToAngelScriptTypeString<T>::Type(), resolver,
            std::is_const_v<std::remove_pointer_t<T>>);
        return cached;
    }
};

#define TYPE_RESOLVER_AS_PREDEFINED(x, astype) \
    template<>                                 \
    struct TypeToAngelScriptTypeString<x> {    \
        static constexpr const char* Type()    \
        {                                      \
            return astype;                     \
        }                                      \
    };

// Special cases of AngelScriptTypeIDResolver for fundamental types //
TYPE_RESOLVER_AS_PREDEFINED(int, "int");
TYPE_RESOLVER_AS_PREDEFINED(unsigned int, "uint");
TYPE_RESOLVER_AS_PREDEFINED(int64_t, "int64");
TYPE_RESOLVER_AS_PREDEFINED(uint64_t, "uint64");
TYPE_RESOLVER_AS_PREDEFINED(int16_t, "int16");
TYPE_RESOLVER_AS_PREDEFINED(uint16_t, "uint16");
TYPE_RESOLVER_AS_PREDEFINED(int8_t, "int8");
TYPE_RESOLVER_AS_PREDEFINED(uint8_t, "uint8");
TYPE_RESOLVER_AS_PREDEFINED(double, "double");
TYPE_RESOLVER_AS_PREDEFINED(float, "float");
TYPE_RESOLVER_AS_PREDEFINED(bool, "bool");
// Char isn't actually signed or unsigned in c++ but this allows basically using char as alias
// for int8_t without explicit casts to angelscript
TYPE_RESOLVER_AS_PREDEFINED(char, "int8");

// And other inbuilt types that can't have ANGELSCRIPT_TYPE in their class
TYPE_RESOLVER_AS_PREDEFINED(std::string, "string");

// And other types
TYPE_RESOLVER_AS_PREDEFINED(bs::Color, "bs::Color@");
TYPE_RESOLVER_AS_PREDEFINED(bs::Matrix4, "bs::Matrix4@");

TYPE_RESOLVER_AS_PREDEFINED(NewtonCollision, "NewtonCollision@");

// Special case void //
template<>
struct AngelScriptTypeIDResolver<void> {
    static int Get(ScriptExecutor* resolver)
    {
        return ANGELSCRIPT_VOID_TYPEID;
    }
};

// Also these angelscript types that can have a bunch of different
// types is a special case
template<>
struct AngelScriptTypeIDResolver<asIScriptFunction*> {
    static int Get(ScriptExecutor* resolver)
    {
        return -1;
    }
};

template<>
struct AngelScriptTypeIDResolver<asIScriptObject*> {
    static int Get(ScriptExecutor* resolver)
    {
        return -1;
    }
};
} // namespace Leviathan
