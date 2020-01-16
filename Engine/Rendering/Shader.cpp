// ------------------------------------ //
#include "Shader.h"


using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Shader::Shader(const Diligent::RefCntAutoPtr<Diligent::IShader>& shader) :
    _Shader(shader)
{
    LEVIATHAN_ASSERT(_Shader, "Diligent shader is null");
}
// ------------------------------------ //
DLLEXPORT Diligent::RefCntAutoPtr<Diligent::IShader> Shader::GetFirstVariant() const
{
    return _Shader;
}

DLLEXPORT Diligent::RefCntAutoPtr<Diligent::IShader> Shader::GetVariant(
    const ShaderVariant& variant) const
{
    return _Shader;
}
