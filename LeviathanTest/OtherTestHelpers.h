// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once

#include "Common/ReferenceCounted.h"
#include "Script/Bindings/BindHelpers.h"
#include "Script/ScriptExecutor.h"

#include "catch.hpp"

//! \file Contains other test helpers that aren't loggers or partial engine

#include <iostream>

namespace Leviathan { namespace Test {

//! \brief Tests that a sequence of values is give to this class
class RequirePassSequence : public ReferenceCounted {
public:
    RequirePassSequence(std::vector<std::string> expectedvalues) : Expected(expectedvalues) {}

    void Provide(const std::string& str)
    {
        REQUIRE(Index < Expected.size());
        CHECK(Expected[Index] == str);
        ++Index;
    }

    bool AllProvided()
    {

        return Index == Expected.size();
    }

    static bool Register(asIScriptEngine* engine)
    {

        ANGELSCRIPT_REGISTER_REF_TYPE("RequirePassSequence", RequirePassSequence);

        REQUIRE(engine->RegisterObjectMethod("RequirePassSequence",
            "void Provide(const string &in str)", asMETHOD(RequirePassSequence, Provide),
            asCALL_THISCALL));

        return true;
    }

    REFERENCE_COUNTED_PTR_TYPE(RequirePassSequence);

    size_t Index = 0;
    std::vector<std::string> Expected;
};



}} // namespace Leviathan::Test
