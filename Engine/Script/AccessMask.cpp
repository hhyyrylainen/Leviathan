// ------------------------------------ //
#include "AccessMask.h"

#include "Exceptions.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT AccessFlags Leviathan::ParseScriptAccess(std::string_view flagstring)
{
    AccessFlags access = 0;

    // Handle in chunks delimited by + //
    size_t start = 0;
    size_t end = 0;

    for(size_t i = 0; i < flagstring.size(); ++i) {

        if(flagstring[i] == '+') {

            // Parse subpart //
            access |= StringToScriptAccess(flagstring.substr(start, end - start + 1));
            start = i + 1;
            end = i + 1;

        } else {

            end = i;
        }
    }

    // Add last part if there is still something //
    if(end > start) {

        access |= StringToScriptAccess(flagstring.substr(start, end - start + 1));
    }

    // Give error about ending with a plus sign //
    if(end == start && end > 0)
        throw InvalidArgument("ScriptAccess value ended in '" +
                              std::string(flagstring.substr(start - 1, 1)) + "'");

    return access;
}

#define STRING_TO_VALUE_HELPER(x) \
    if(str == #x)                 \
        return static_cast<AccessFlags>(ScriptAccess::x);

DLLEXPORT AccessFlags Leviathan::StringToScriptAccess(std::string_view str)
{

    STRING_TO_VALUE_HELPER(Nothing);

    STRING_TO_VALUE_HELPER(Builtin);
    STRING_TO_VALUE_HELPER(DefaultEngine);
    STRING_TO_VALUE_HELPER(FullFileSystem);

    throw InvalidArgument("No ScriptAccess value named '" + std::string(str) + "'");
    return static_cast<AccessFlags>(ScriptAccess::Nothing);
}
