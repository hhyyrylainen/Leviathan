// ------------------------------------ //
#include "Buffer.h"

#include "Graphics.h"

using namespace Leviathan;
using namespace Leviathan::Rendering;
// ------------------------------------ //
DLLEXPORT MappedBuffer::MappedBuffer(Graphics& graphics, Buffer& buffer,
    Diligent::MAP_TYPE mappingtype, Diligent::MAP_FLAGS mapflags) :
    _Graphics(graphics),
    _Buffer(buffer), Type(mappingtype), Flags(mapflags)
{
    MappedData = reinterpret_cast<uint8_t*>(_Graphics.MapBuffer(_Buffer, Type, Flags));
}

DLLEXPORT MappedBuffer::~MappedBuffer()
{
    if(Valid()) {
        _Graphics.UnMapBuffer(_Buffer, Type);
    }
}
// ------------------------------------ //
Buffer::Buffer(const Diligent::RefCntAutoPtr<Diligent::IBuffer>& buffer) : _Buffer(buffer) {}
