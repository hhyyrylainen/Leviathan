// ------------------------------------ //
#include "ScriptTypeResolver.h"

#include "ScriptExecutor.h"
// ------------------------------------ //
namespace Leviathan {
DLLEXPORT int ResolveProxy(const char* type, ScriptExecutor* resolver, bool constversion)
{
    return resolver->ResolveStringToASID(type, constversion);
}

DLLEXPORT ScriptExecutor* GetCurrentGlobalScriptExecutor()
{
    return ScriptExecutor::Get();
}
} // namespace Leviathan
