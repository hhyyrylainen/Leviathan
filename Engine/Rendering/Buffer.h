// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

// part of the hack
#undef LOG_ERROR

#include "DiligentCore/Common/interface/RefCntAutoPtr.hpp"
#include "DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h"

// hack workaround
#undef LOG_ERROR
#define LOG_ERROR(x) Logger::Get()->Error(x);
#undef CHECK
#define CHECK(x)


#include <cstring>

namespace Leviathan {

class Graphics;

namespace Rendering {

class Buffer;

//! \brief Helper for mapping and unmapping a buffer for reading/writing
class MappedBuffer {
public:
    //! \brief Creates a mapping for the specified buffer
    DLLEXPORT MappedBuffer(Graphics& graphics, Buffer& buffer, Diligent::MAP_TYPE mappingtype,
        Diligent::MAP_FLAGS mapflags);
    DLLEXPORT ~MappedBuffer();
    MappedBuffer(const MappedBuffer& other) = delete;
    MappedBuffer& operator=(const MappedBuffer& other) = delete;

    //! \brief Writes to the mapped buffer. Doesn't check if it was mapped for write
    inline void Write(const void* source, size_t bytes, size_t offset = 0)
    {
        std::memcpy(MappedData + offset, source, bytes);
    }

    inline bool Valid() const
    {
        return MappedData != nullptr;
    }

private:
    Graphics& _Graphics;
    Buffer& _Buffer;
    uint8_t* MappedData;
    const Diligent::MAP_TYPE Type;
    const Diligent::MAP_FLAGS Flags;
};

//! \brief Can hold any type of buffer for use in graphics rendering
class Buffer {
public:
    Buffer(const Diligent::RefCntAutoPtr<Diligent::IBuffer>& buffer);

    auto& GetInternal()
    {
        return _Buffer;
    }

private:
    Diligent::RefCntAutoPtr<Diligent::IBuffer> _Buffer;
};

} // namespace Rendering
} // namespace Leviathan
