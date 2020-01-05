// ------------------------------------ //
#include "Shader.h"


using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Shader::Shader(const bs::HShader& shader) : BsShader(shader)
{
    LEVIATHAN_ASSERT(shader, "Given bsf shader is null");
}
