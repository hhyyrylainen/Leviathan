// ------------------------------------ //
#include "ScriptTypeResolver.h"

#include "ScriptExecutor.h"
// ------------------------------------ //
namespace Leviathan {
DLLEXPORT int ResolveProxy(const char* type, ScriptExecutor* resolver)
{
    return resolver->ResolveStringToASID(type);
}

DLLEXPORT ScriptExecutor* GetCurrentGlobalScriptExecutor()
{
    return ScriptExecutor::Get();
}
} // namespace Leviathan
