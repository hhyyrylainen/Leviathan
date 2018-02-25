// ------------------------------------ //
#include "BindStandardFunctions.h"

#include "Define.h"

#include <algorithm>
#include <inttypes.h>

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //

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