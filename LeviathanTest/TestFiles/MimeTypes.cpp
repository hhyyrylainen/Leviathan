
//! \file Testing for methods in StringOperations
#include "Common/MimeTypes.h"

#include "catch.hpp"

using namespace Leviathan;

TEST_CASE("Leviathan::GetMimeTypeFromPath non-existing basic filepaths", "[string]")
{
    SECTION("test.js")
    {
        CHECK(GetMimeTypeFromPath("test.js") == "application/javascript");
    }

    SECTION("Data/Scripts/gui/main_menu.mjs")
    {
        CHECK(
            GetMimeTypeFromPath("Data/Scripts/gui/main_menu.mjs") == "application/javascript");
    }
}
