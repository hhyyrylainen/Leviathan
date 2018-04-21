#include "Input/Key.h"

#include "../PartialEngine.h"
#include "catch.hpp"

#include <SDL.h>

using namespace Leviathan;
using namespace Test;

TEST_CASE("Common key names parsing", "[string][input]")
{
    TestLogger log;

    SECTION("ESCAPE")
    {
        const auto key = GKey::GenerateKeyFromString("ESCAPE");
        // 27 == ESC from asciitable
        CHECK(key.GetCharacter() == 27);
    }

    SECTION("+")
    {
        CHECK_NOTHROW(GKey::GenerateKeyFromString("+"));
    }

    SECTION("-")
    {
        CHECK_NOTHROW(GKey::GenerateKeyFromString("-"));
    }

    SECTION("Keypad keys")
    {
        CHECK(GKey::GenerateKeyFromString("Keypad -").GetCharacter() == SDLK_KP_MINUS);
        CHECK(GKey::GenerateKeyFromString("Keypad +").GetCharacter() == SDLK_KP_PLUS);
    }
}

TEST_CASE("Combination key parsing", "[string][input]")
{
    TestLogger log;

    SECTION("random keys plus a modifier")
    {
        CHECK_NOTHROW(GKey::GenerateKeyFromString("A+SHIFT"));
        CHECK_NOTHROW(GKey::GenerateKeyFromString("K+ALT"));
    }

    SECTION("plus key plus a modifier")
    {
        CHECK_NOTHROW(GKey::GenerateKeyFromString("++SHIFT"));
    }
}

TEST_CASE("Invalid key name gives an error", "[string][input]")
{
    // Needs to print an error. In the future this might throw //
    TestLogRequireError log;

    const auto key = GKey::GenerateKeyFromString("not a key");

    CHECK(key.GetCharacter() == 0);
}
