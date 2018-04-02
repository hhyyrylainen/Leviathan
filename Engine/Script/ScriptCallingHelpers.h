// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "angelscript.h"

#include <type_traits>

namespace Leviathan {

//! \brief Returns true of object type is reference counted
template<class T>
static inline constexpr bool IsTypeReferenceCounted()
{
    using CheckType = std::remove_pointer_t<std::remove_cv_t<T>>;
    return std::is_base_of_v<ReferenceCounted, CheckType> ||
           std::is_same_v<asIScriptFunction, CheckType> ||
           std::is_same_v<asIScriptObject, CheckType>;
}

//! \brief Increments refcount of obj if it is derived from ReferenceCounted
//! or an angelscript type
template<class T>
static inline void IncrementRefCountIfRefCountedType(T* current)
{
    if constexpr(IsTypeReferenceCounted<T>()) {
        if(current)
            current->AddRef();
    }
}

//! \brief Increments refcount of obj if it is derived from ReferenceCounted
//! or an angelscript type
template<class T>
static inline void DecrementRefCountIfRefCountedType(T* current)
{
    if constexpr(IsTypeReferenceCounted<T>()) {
        if(current)
            current->Release();
    }
}

//! \brief Returns true if type is an angelscript type that can
//! represent different types of objects
template<class T>
static inline constexpr bool CanTypeRepresentAngelScriptTypes()
{
    using CheckType = std::remove_pointer_t<std::remove_cv_t<T>>;
    return std::is_same_v<asIScriptFunction, CheckType> ||
           std::is_same_v<asIScriptObject, CheckType>;
}


} // namespace Leviathan
