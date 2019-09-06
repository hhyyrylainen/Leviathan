// ------------------------------------ //
#include "BindStandardFunctions.h"

#include "Define.h"

#include "utf8.h"

#include <algorithm>
#include <cmath>
#include <inttypes.h>

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
int RoundProxy(double value)
{
    return static_cast<int>(std::round(value));
}

int RoundProxy2(float value)
{
    return static_cast<int>(std::round(value));
}

float SignProxy(float value)
{
    return value < 0.f ? -1.f : 1.f;
}

std::string CharacterToStringProxy(int32_t character)
{
    try {

        std::string str;

        utf8::append(character, std::back_inserter(str));

        return str;

    } catch(const std::exception& e) {

        auto ctx = asGetActiveContext();
        if(ctx) {

            ctx->SetException(("character code ' " + std::to_string(character) +
                               "' failed to be represented as utf8 string, error: " + e.what())
                                  .c_str());
            // The return value cannot be read by the script so it
            // isn't any use to return an error here
            return "";
        } else {
            // Not called from script
            throw;
        }
    }
}

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {

bool BindMathOperations(asIScriptEngine* engine)
{

    // ------------------------------------ //
    // min
    if(engine->RegisterGlobalFunction(
           "const float & min(const float &in a, const float &in b)",
           asFUNCTIONPR(std::min<float>, (const float&, const float&), const float&),
           asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction(
           "const int64 & min(const int64 &in a, const int64 &in b)",
           asFUNCTIONPR(std::min<int64_t>, (const int64_t&, const int64_t&), const int64_t&),
           asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // max
    if(engine->RegisterGlobalFunction(
           "const float & max(const float &in a, const float &in b)",
           asFUNCTIONPR(std::max<float>, (const float&, const float&), const float&),
           asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction(
           "const int64 & max(const int64 &in a, const int64 &in b)",
           asFUNCTIONPR(std::max<int64_t>, (const int64_t&, const int64_t&), const int64_t&),
           asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // round
    if(engine->RegisterGlobalFunction(
           "int round(double value)", asFUNCTION(RoundProxy), asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterGlobalFunction(
           "int round(float value)", asFUNCTION(RoundProxy2), asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // sign
    if(engine->RegisterGlobalFunction(
           "float sign(float value)", asFUNCTION(SignProxy), asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // clamp
    if(engine->RegisterGlobalFunction("const float & clamp(const float &in value, const float "
                                      "&in lower, const float &in upper)",
           asFUNCTIONPR(
               std::clamp<float>, (const float&, const float&, const float&), const float&),
           asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Extra string helpers
    if(engine->RegisterGlobalFunction("string CharacterToString(int32 character)",
           asFUNCTION(CharacterToStringProxy), asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}
// ------------------------------------ //

} // namespace Leviathan

// ------------------------------------ //
// Main bind function
bool Leviathan::BindStandardFunctions(asIScriptEngine* engine)
{
    if(!BindMathOperations(engine))
        return false;

    return true;
}
