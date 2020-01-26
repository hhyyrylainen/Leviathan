// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Layout.h"

#include "Common/ReferenceCounted.h"

#include "DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"

namespace Leviathan {

namespace Rendering {
class Buffer;
}

class Mesh : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT Mesh(const std::shared_ptr<Rendering::Buffer>& vertexbuffer,
        Rendering::LayoutElements vertexlayout,
        const std::shared_ptr<Rendering::Buffer>& indexbuffer, Diligent::VALUE_TYPE indextype,
        int indexcount);

public:
    inline auto& GetVertexBuffer()
    {
        return VertexBuffer;
    }

    inline auto& GetVertexLayout()
    {
        return VertexLayout;
    }

    inline auto& GetIndexBuffer()
    {
        return IndexBuffer;
    }

    inline auto GetIndexType() const
    {
        return IndexType;
    }

    inline auto GetIndexCount() const
    {
        return IndexCount;
    }

    REFERENCE_COUNTED_PTR_TYPE(Mesh);

private:
    std::shared_ptr<Rendering::Buffer> VertexBuffer;
    Rendering::LayoutElements VertexLayout;

    std::shared_ptr<Rendering::Buffer> IndexBuffer;
    Diligent::VALUE_TYPE IndexType;
    int IndexCount;
};

} // namespace Leviathan
