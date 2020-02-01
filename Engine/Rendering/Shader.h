// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

// part of the hack
#undef LOG_ERROR

#include "DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "DiligentCore/Graphics/GraphicsEngine/interface/Shader.h"

// hack workaround
#undef LOG_ERROR
#define LOG_ERROR(x) Logger::Get()->Error(x);

namespace Leviathan {

struct ShaderVariationInfo {
public:
};

struct ShaderVariant {
public:
};

//! \brief Represents a collection of shaders with different constant values
class Shader : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT Shader(const Diligent::RefCntAutoPtr<Diligent::IShader>& shader);

public:
    //! \brief Returns the first shader variant (usually with all defines false)
    DLLEXPORT Diligent::RefCntAutoPtr<Diligent::IShader> GetFirstVariant() const;
    DLLEXPORT Diligent::RefCntAutoPtr<Diligent::IShader> GetVariant(
        const ShaderVariant& variant) const;

    const auto& GetInternal() const
    {
        return _Shader;
    }

    REFERENCE_COUNTED_PTR_TYPE(Shader);

private:
    Diligent::RefCntAutoPtr<Diligent::IShader> _Shader;
};

} // namespace Leviathan
