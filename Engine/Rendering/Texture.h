// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

// part of the hack
#undef LOG_ERROR

#include "DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "DiligentCore/Graphics/GraphicsEngine/interface/Texture.h"

// hack workaround
#undef LOG_ERROR
#define LOG_ERROR(x) Logger::Get()->Error(x);
#undef CHECK
#define CHECK(x)


namespace Leviathan {

class Texture : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT Texture(
        const Diligent::RefCntAutoPtr<Diligent::ITexture>& texture, int width, int height);

public:
    auto GetWidth() const
    {
        return Width;
    }

    auto GetHeight() const
    {
        return Height;
    }

    auto* GetDefaultSRV()
    {
        return _Texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
    }

    auto& GetInternal()
    {
        return _Texture;
    }

    REFERENCE_COUNTED_PTR_TYPE(Texture);

private:
    Diligent::RefCntAutoPtr<Diligent::ITexture> _Texture;
    int Width;
    int Height;
};

} // namespace Leviathan
