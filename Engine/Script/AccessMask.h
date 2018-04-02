// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
//! \brief The access mask controls which registered functions and classes a script sees
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <string_view>

namespace Leviathan {

//! \note There is a maximum of 32 of different masks that can be defined
enum class ScriptAccess : uint32_t{

    //! Can access none of the bound functions or "inbuilt" math and string types
    Nothing = 0x0,

    //! Access to the builtin types like string and math functions
    Builtin = 0x1,

    //! Default mask for engine classes (some things are accessible even with this off)
    DefaultEngine = 0x2,
    FullFileSystem = 0x4,
	// 0x8
	// 0x10
    // 0x20
	//0x40
	//0x80 // first byte full
	//0x100
	//0x200
	//0x400
	//0x800
	//0x1000
	//0x2000
	//0x4000
	//0x8000 // second byte full (int range might stop here(
	//0x10000
	//0x20000
	//0x40000
	//0x80000
	//0x100000
	//0x200000
	//0x400000
	//0x800000 // third byte full
	//0x1000000
	//0x2000000
	//0x4000000
	//0x8000000
	//0x10000000
	//0x20000000
	//0x40000000
	//0x80000000 // fourth byte full. Last valid flag for angelscript
};

using AccessFlags = uint32_t;
constexpr AccessFlags DefaultAccessFlags = static_cast<uint32_t>(ScriptAccess::DefaultEngine) |
    static_cast<uint32_t>(ScriptAccess::Builtin);

// Convenience operators
inline bool operator==(const ScriptAccess &lhs, const AccessFlags &rhs){
    return static_cast<AccessFlags>(lhs) == rhs;
}

inline bool operator==(const AccessFlags &lhs, const ScriptAccess &rhs){
    return rhs == static_cast<AccessFlags>(lhs);
}

//! \brief Parses a string for access flags
//!
//! In the form of 'flag+flag+flag...' For example 'DefaultEngine+FullFileSystem'
//! \exception InvalidArgument if the string contained something that wasn't valid
DLLEXPORT AccessFlags ParseScriptAccess(std::string_view flagstring);

//! \brief Returns ScriptAccess matching string
//! \exception InvalidArgument if didn't match
DLLEXPORT AccessFlags StringToScriptAccess(std::string_view str);

} // namespace Leviathan
