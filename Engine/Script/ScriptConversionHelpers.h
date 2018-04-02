// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
//! \file This file contains conversion helpers for example to return arrays to angelscript
#include "angelscript.h"

#include "add_on/scriptarray/scriptarray.h"

#include "ScriptExecutor.h"

#include <type_traits>
#include <vector>

namespace Leviathan {

//! \brief Converts a std::vector to an AngelScript array
//! \param arraytype If given must be of the form "array<ContainedType>". This is more
//! efficient than automatically letting this construct the typename but care must be taken to
//! make sure it is correct
template<class T>
CScriptArray* ConvertVectorToASArray(
    const std::vector<T>& data, asIScriptEngine* engine, const char* arraytype = nullptr)
{
    using UsedT = std::remove_const_t<std::remove_reference_t<T>>;

    asITypeInfo* typeInfo = nullptr;

    if(arraytype) {
        typeInfo = engine->GetTypeInfoByDecl(arraytype);
    } else {
        typeInfo = engine->GetTypeInfoByDecl(
            ("array<" + std::string(TypeToAngelScriptTypeString<UsedT>::Type()) + ">")
                .c_str());
    }
    if(!typeInfo)
        throw InvalidArgument("Trying to convert vector of non-angelscript supported types to "
                              "angelscript array");

    CScriptArray* array = CScriptArray::Create(typeInfo, static_cast<asUINT>(data.size()));

    if(!array)
        return nullptr;

    // Copy over values. This does copy construction so should work
    // with value types and pointers
    for(asUINT i = 0; i < static_cast<asUINT>(data.size()); ++i) {

        if constexpr(std::is_pointer_v<UsedT>) {

            // Hopefully this const cast doesn't let as run a constructor that changes the
            // original values
            array->SetValue(i, const_cast<UsedT>(data[i]));

        } else {
            // This should do copy construction based on the pointer
            array->SetValue(i, const_cast<std::add_pointer_t<UsedT>>(&data[i]));
        }
    }

    return array;
}

//! \brief Variant that converts from iterator range to an AngelScript array
//! \see ConvertVectorToASArray
//!
//! If calling with a map you need to unwrap the pairs with boost like this:
//! \begincode
//! ConvertIteratorToASArray(std::begin(testData | boost::adaptors::map_keys),
//!     std::end(testData | boost::adaptors::map_keys), engine)
//! \endcode
template<class T>
CScriptArray* ConvertIteratorToASArray(
    T begin, T end, asIScriptEngine* engine, const char* arraytype = nullptr)
{
    // This makes maps work and also as we want the things to not be in const angelscript so...
    using UsedT = std::remove_const_t<std::remove_reference_t<decltype(*begin)>>;

    asITypeInfo* typeInfo = nullptr;

    if(arraytype) {
        typeInfo = engine->GetTypeInfoByDecl(arraytype);
    } else {
        typeInfo = engine->GetTypeInfoByDecl(
            ("array<" + std::string(TypeToAngelScriptTypeString<UsedT>::Type()) + ">")
                .c_str());
    }
    if(!typeInfo)
        throw InvalidArgument("Trying to convert iterators of non-angelscript supported type "
                              "to angelscript array");

    CScriptArray* array = CScriptArray::Create(typeInfo);

    if(!array)
        return nullptr;

    // Copy over values. This does copy construction so should work
    // with value types and pointers
    while(begin != end) {

        if constexpr(std::is_pointer_v<UsedT>) {

            // Hopefully this const cast doesn't let as run a constructor that changes the
            // original values
            array->InsertLast(const_cast<UsedT>(*begin));

        } else {
            // This should do copy construction based on the pointer
            array->InsertLast(const_cast<std::add_pointer_t<UsedT>>(&*begin));
        }

        ++begin;
    }

    return array;
}


} // namespace Leviathan
