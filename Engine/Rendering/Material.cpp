// ------------------------------------ //
#include "Material.h"

#include "bsfCore/Material/BsMaterial.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Material::Material(const Shader::pointer& shader) :
    BsMaterial(bs::Material::create(shader->GetInternal())), _Shader(shader)
{}

DLLEXPORT Material::Material() {}
// ------------------------------------ //
DLLEXPORT void Material::SetTexture(
    const std::string& parameter, const Texture::pointer& texture)
{
    if(!BsMaterial || !texture)
        return;

    BsMaterial->setTexture(parameter.c_str(), texture->GetInternal());
}

DLLEXPORT void Material::SetFloat4(const std::string& parameter, const Float4& data)
{
    if(!BsMaterial)
        return;

    BsMaterial->setVec4(parameter.c_str(), data);
}
