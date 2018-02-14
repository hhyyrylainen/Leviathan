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

    if(engine->RegisterGlobalFunction("float min(float a, float b)",
           asFUNCTIONPR(std::min<float>, (const float&, const float&), const float&),
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
