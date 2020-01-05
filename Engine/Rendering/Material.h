// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Shader.h"
#include "Texture.h"

#include "Common/ReferenceCounted.h"
#include "Common/Types.h"

#include "bsfCore/BsCorePrerequisites.h"

namespace Leviathan {

class Material : public ReferenceCounted {
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT Material(const Shader::pointer& shader);
    DLLEXPORT Material();

public:
    // Setting functions for shader parameters
    DLLEXPORT void SetTexture(const std::string& parameter, const Texture::pointer& texture);
    DLLEXPORT void SetFloat4(const std::string& parameter, const Float4& data);
    DLLEXPORT void SetFloat(const std::string& parameter, float data);


    DLLEXPORT void SetVariation(const std::string& variation, bool set);


    bs::HMaterial GetInternal()
    {
        return BsMaterial;
    }

    void SetTextureWrapper(const std::string& parameter, Texture* texture)
    {
        SetTexture(parameter, Texture::WrapPtr(texture));
    }

    REFERENCE_COUNTED_PTR_TYPE(Material);

private:
    bs::HMaterial BsMaterial;
    Shader::pointer _Shader;
};

} // namespace Leviathan
