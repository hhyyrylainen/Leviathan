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
    asITypeInfo* typeInfo = nullptr;

    if(arraytype) {
        typeInfo = engine->GetTypeInfoByDecl(arraytype);
    } else {
        typeInfo = engine->GetTypeInfoByDecl(
            ("array<" + std::string(TypeToAngelScriptTypeString<T>::Type()) + ">").c_str());
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

        if constexpr(std::is_pointer_v<T>) {

            // Hopefully this const cast doesn't let as run a constructor that changes the
            // original values
            array->SetValue(i, const_cast<T>(data[i]));

        } else {
            // This should do copy construction based on the pointer
            array->SetValue(i, const_cast<T*>(&data[i]));
        }
    }

    return array;
}

} // namespace Leviathan
